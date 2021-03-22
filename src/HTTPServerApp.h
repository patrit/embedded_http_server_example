#ifndef SRC_HTTPSERVERAPP_H_
#define SRC_HTTPSERVERAPP_H_

#include <Poco/Net/HTTPRequestHandlerFactory.h>
#include <Poco/Util/ServerApplication.h>
#include <string>
#include <utility>
#include <vector>

class HTTPServerApp : public Poco::Util::ServerApplication {
public:
  HTTPServerApp() = default;
  virtual ~HTTPServerApp() = default;

protected:
  void defineOptions(Poco::Util::OptionSet &options);
  void handleOption(const std::string &name, const std::string &value);
  void displayHelp();
  int main(const std::vector<std::string> & /*args*/);

private:
  bool helpRequested = false;
  int listenPort = 80;
  std::string configProperties;
};
#endif
