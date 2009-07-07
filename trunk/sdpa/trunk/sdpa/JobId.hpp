#ifndef SDPA_JOB_ID_HPP
#define SDPA_JOB_ID_HPP 1

#include <string>

namespace sdpa {
  class JobId {
  public:
    JobId();
    JobId(const std::string &s);
    JobId(const char *s);
    JobId(const JobId &other);
    const JobId &operator=(const JobId &rhs);
    operator std::string();
    bool operator!=(const JobId& rhs);
    ~JobId();
    const std::string &str() const;

  private:
    std::string id_;
  };
}


#endif
