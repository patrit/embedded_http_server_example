#include "HTTPServerApp.h"
#include "Entrypoint.h"
#include <Poco/Net/HTMLForm.h>
#include <Poco/Net/HTTPRequestHandler.h>
#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Net/HTTPResponse.h>
#include <Poco/Net/HTTPServer.h>
#include <Poco/Net/HTTPServerRequest.h>
#include <Poco/Net/HTTPServerResponse.h>
#include <Poco/Net/ServerSocket.h>
#include <Poco/Util/HelpFormatter.h>
#include <Poco/Util/Option.h>
#include <Poco/Util/OptionSet.h>
#include <Poco/Util/ServerApplication.h>
#include <ctime>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

using namespace Poco::Net;
using namespace Poco::Util;
using namespace std;

static unsigned long memLimit = 0;
static int memoryLimitReached = 0;
static bool debug = false;

#ifdef __linux__
static int pageSize = getpagesize() / 1024;

static unsigned long getMemoryConsumption() {
  const char *statm_path = "/proc/self/statm";
  unsigned long size, resident, share, text, lib, data, dt;

  FILE *f = fopen(statm_path, "r");
  if (f == nullptr) {
    perror(statm_path);
    return 0;
  }
  if (7 != fscanf(f, "%lu %lu %lu %lu %lu %lu %lu", &size, &resident, &share,
                  &text, &lib, &data, &dt)) {
    perror(statm_path);
    return 0;
  }
  fclose(f);
  return resident * pageSize;
}

static bool isMemoryConsumptionTooHigh() {
  if (memLimit > 0) {
    return getMemoryConsumption() > memLimit;
  }
  return false;
}
#else
static bool isMemoryConsumptionTooHigh() { return false; }
#endif

class PingRequestHandler : public HTTPRequestHandler {
public:
  virtual ~PingRequestHandler() = default;
  virtual void handleRequest(HTTPServerRequest & /*request*/,
                             HTTPServerResponse &resp) {
    if (isMemoryConsumptionTooHigh()) {
      memoryLimitReached++;
      if (debug) {
        std::cout << "DEBUG: memory limit reached" << std::endl;
      }
      resp.setStatus(HTTPResponse::HTTP_NOT_FOUND);
    } else {
      memoryLimitReached = 0;
      resp.setStatus(HTTPResponse::HTTP_OK);
    }
    resp.send() << "";
  }
};

#ifdef __linux__
// the sleep request handler is to test load balancing timeouts
class SleepRequestHandler : public HTTPRequestHandler {
public:
  SleepRequestHandler(string const &sleepURI) : _sleepURI(sleepURI) {}
  virtual ~SleepRequestHandler() = default;
  virtual void handleRequest(HTTPServerRequest &req, HTTPServerResponse &resp) {
    uint32_t msec = stol(req.getURI().substr(_sleepURI.size()));
    usleep(msec * 1000);
    resp.setStatus(HTTPResponse::HTTP_OK);
    resp.send().flush();
  }

private:
  string const &_sleepURI;
};

// the memory leak handler allows explicit memory leak tests, to verify restart
// behaviour
class MemoryLeakRequestHandler : public HTTPRequestHandler {
public:
  MemoryLeakRequestHandler(string const &leakURI) : _leakURI(leakURI) {}
  virtual ~MemoryLeakRequestHandler() = default;
  virtual void handleRequest(HTTPServerRequest &req, HTTPServerResponse &resp) {
    struct MemoryLeak {
      char a[1024];
    };
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-variable"
    for (int i = 0; i < stoi(req.getURI().substr(_leakURI.size())); i++) {
      MemoryLeak *m = new MemoryLeak();
    }
#pragma clang diagnostic pop
    resp.setStatus(HTTPResponse::HTTP_OK);
    resp.send().flush();
  }

private:
  string const &_leakURI;
};
#endif

class APIRequesthandler : public HTTPRequestHandler {
  Entrypoint &_entrypoint;

public:
  APIRequesthandler(Entrypoint &entrypoint) : _entrypoint(entrypoint) {}
  virtual ~APIRequesthandler() {}
  virtual void handleRequest(HTTPServerRequest &req, HTTPServerResponse &resp) {
    if (req.getMethod() != HTTPServerRequest::HTTP_POST) {
      resp.setStatus(HTTPResponse::HTTP_METHOD_NOT_ALLOWED);
      resp.send().flush();
    }
    if (req.getContentLength() == 0) {
      resp.setStatus(HTTPResponse::HTTP_BAD_REQUEST);
      resp.send().flush();
    }
    if ((req.getContentType() != "text/xml") &&
        (req.getContentType() != "application/xml")) {
      resp.setStatus(HTTPResponse::HTTP_UNSUPPORTEDMEDIATYPE);
      resp.send().flush();
    }

    ostringstream xml;
    xml << req.stream().rdbuf();
    string retstr = _entrypoint.call(xml.str());
    // showcase - implement proper error hanlding in here
    if (!retstr.empty()) {
      resp.setStatus(HTTPResponse::HTTP_OK);
      resp.setContentType("text/xml");
      ostream &out = resp.send();
      out << retstr;
      out.flush();
    } else {
      resp.setStatus(HTTPResponse::HTTP_INTERNAL_SERVER_ERROR);
      resp.setContentType("text/plain");
      ostream &out = resp.send();
      out << "Insert meaningfull error handling";
      out.flush();
    }
  }
};

class VersionRequesthandler : public HTTPRequestHandler {
public:
  virtual ~VersionRequesthandler() = default;
  virtual void handleRequest(HTTPServerRequest &req, HTTPServerResponse &resp) {
    if (req.getMethod() != HTTPServerRequest::HTTP_GET) {
      resp.setStatus(HTTPResponse::HTTP_METHOD_NOT_ALLOWED);
      resp.send().flush();
    }
    // faked: do proper verisoning or remove if unused
    string retstr = "{\"component1\": \"0.0.0\", \"component2\": \"42.42.42\"}";

    resp.setStatus(HTTPResponse::HTTP_OK);
    resp.setContentType("application/json");
    ostream &out = resp.send();
    out << retstr;
    out.flush();
  }
};

class RequestHandlerFactory : public HTTPRequestHandlerFactory {
public:
  RequestHandlerFactory(Entrypoint &entrypoint) : _entrypoint(entrypoint) {}
  virtual ~RequestHandlerFactory() = default;

  virtual HTTPRequestHandler *
  createRequestHandler(const HTTPServerRequest &req) {
    std::time_t result = std::time(nullptr);
    ofstream myfile("/tmp/lastHttpRequest");
    if (myfile.is_open()) {
      myfile << result;
      myfile.close();
    }

    if ((req.getURI() == "/api") || (req.getURI().substr(0, 5) == "/api/")) {
      return new APIRequesthandler(_entrypoint);
    }
    if (req.getURI() == "/versions") {
      return new VersionRequesthandler;
    }
    if (req.getURI() == "/ping") {
      return new PingRequestHandler;
    }
#ifdef __linux__
    if (debug && (req.getURI().rfind("/sleep/", 0) == 0u)) {
      return new SleepRequestHandler("/sleep/");
    }
    if (debug && (req.getURI().rfind("/memoryleak/", 0) == 0u)) {
      return new MemoryLeakRequestHandler("/memoryleak/");
    }
#endif
    return nullptr;
  }

private:
  Entrypoint &_entrypoint;
};

void HTTPServerApp::defineOptions(OptionSet &options) {
  ServerApplication::defineOptions(options);
  options.addOption(Option("help", "h", "display help").repeatable(false));
  options.addOption(Option("port", "p", "Listening port (default 80)")
                        .repeatable(true)
                        .argument("<port>"));
  options.addOption(Option("config", "c", "Configuration properties")
                        .repeatable(true)
                        .argument("<config>"));
}

void HTTPServerApp::handleOption(const std::string &name,
                                 const std::string &value) {
  ServerApplication::handleOption(name, value);
  if (name == "help") {
    helpRequested = true;
  } else if (name == "port") {
    listenPort = std::stoi(value);
  } else if (name == "config") {
    configProperties = value;
  }
}

void HTTPServerApp::displayHelp() {
  HelpFormatter helpFormatter(options());
  helpFormatter.setCommand(commandName());
  helpFormatter.setUsage("[options]");
  helpFormatter.setHeader("Application server.");
  helpFormatter.format(std::cout);
}

int HTTPServerApp::main(const std::vector<std::string> & /*args*/) {
  if (helpRequested) {
    displayHelp();
    return Application::EXIT_OK;
  }

  // a good point to handle properties given through a file
  // handleMyProperties(configProperties) or use Poco property handling

  // debug
  debug = (getenv("DEBUG") != nullptr);
  if (debug) {
    cout << "DEBUG: activated debugging" << endl;
  }

  // init soft stop memory limit
  const char *limit = getenv("SOFT_RESTART_MEMORY_LIMIT");
  if (limit != nullptr) {
    memLimit = atoi(limit);
    cout << "Activated soft stop memory limit " << memLimit << endl;
  }

  /* We shall use only 1 connection to guarantee proper health check.
   * If a request is pending the health check ("/ping") shall be queued
   * to be able to determine long running requests.
   * Furthermore the server does not allow multiple real requests.
   * Multiple requests may be distributed over multiple server instances.
   */
  HTTPServerParams *params = new HTTPServerParams();
  params->setMaxThreads(1);

  // just as an example of persistent classes over multiple requests
  Entrypoint entrypoint;

  if (!entrypoint.init()) {
    cerr << "Unable to connect to DB" << endl;
    return Application::EXIT_CONFIG;
  }

  HTTPServer s(new RequestHandlerFactory(entrypoint), listenPort, params);

  s.start();
  std::cout << std::endl
            << "Server started on 0.0.0.0:" << listenPort << std::endl;

  waitForTerminationRequest(); // wait for CTRL-C or kill

  std::cout << std::endl << "Shutting down..." << std::endl;
  s.stop();

  return Application::EXIT_OK;
}
