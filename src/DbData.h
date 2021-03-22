#ifndef DBDATA_H_
#define DBDATA_H_

#include <string>
#include <vector>

struct DataStruct {
  std::string firstname;
  std::string lastname;
  int32_t edlevel;
};

class DbData {

public:
  DbData() = delete;
  ~DbData() = delete;

  static int getData(std::string const &job, std::vector<DataStruct> &values);
};

#endif /* DBDATA_H_ */
