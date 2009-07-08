#ifndef SDPA_JOB_ID_HPP
#define SDPA_JOB_ID_HPP 1

#include <string>
#include <ostream>

namespace sdpa {
  class JobId {
  public:
    JobId();
    JobId(const std::string &s);
    JobId(const char *s);
    JobId(const JobId &other);
    const JobId &operator=(const JobId &rhs);
    const JobId &operator=(const std::string &rhs);
    const JobId &operator=(const char *rhs);
    operator std::string() const;
    operator const char*() const;
    bool operator!=(const JobId& rhs) const;
    bool operator==(const JobId& rhs) const;
    bool operator<(const JobId& rhs) const;
    ~JobId();
    const std::string &str() const;

  private:
    std::string id_;
  };
}

std::ostream &operator<<(std::ostream &os, const sdpa::JobId &j);

#endif
