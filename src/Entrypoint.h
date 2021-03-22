#ifndef ENTRYPOINT_H_
#define ENTRYPOINT_H_

#include <string>

class Entrypoint {
public:
  Entrypoint() = default;
  ~Entrypoint();

  bool init();
  std::string call(std::string const &req);
};
#endif
