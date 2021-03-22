#ifndef DB_H_
#define DB_H_

class DB {

public:
  DB() = delete;
  ~DB() = delete;

  static bool connect(const char *db, const char *authid, const char *psswd);
  static bool disconnect();
  static bool commit();
  static bool rollback();
};

#endif /* DB_H_ */
