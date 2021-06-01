#include "Entrypoint.h"
#include "DB.h"
#include "MyCFunc.h"
#include "DbData.h"
#include <iostream>
#include <vector>

using namespace std;

Entrypoint::~Entrypoint() { DB::disconnect(); }

namespace {
bool logon() {
  const char *dbname = getenv("DBNAME");
  const char *dba = getenv("DBA");
  const char *dba_passwd = getenv("DBA_PASSWD");
  // TODO: do some sanity checks here
  return DB::connect(dbname, dba, dba_passwd);
};
} // namespace

bool Entrypoint::init() { return logon(); }

std::string Entrypoint::call(std::string const &req) {
  cout << "Received " << req << endl;

  my_c_func("just started");

  if (!DB::rollback()) {
    DB::disconnect();
    if (!logon()) {
      cerr << "Unable to connect to " << getenv("DBA") << "@"
           << getenv("DBNAME") << endl;
      // TODO: error handling, e.g. return code|| throw exception
      return std::string();
    }
  }

  std::vector<DataStruct> values;
  int ret = DbData::getData("DESIGNER", values);

  // commit or rollback transaction after work is done
  DB::commit();

  if (ret != 0) {
    cerr << "DB call result: " << ret << endl;
    // TODO: dump SQL error message, throw exception or whatever suits
  }

  string retStr;
  for (DataStruct ds : values) {
    retStr +=
        ds.firstname + "," + ds.lastname + "," + to_string(ds.edlevel) + "\n";
  }

  return retStr;
}
