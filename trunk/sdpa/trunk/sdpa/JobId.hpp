#ifndef SDPA_JOB_ID_HPP
#define SDPA_JOB_ID_HPP 1

#include <string>
#include <ostream>

namespace sdpa {
  /**
    This class defines a 'transparent-to-string' JobId class type.
   */
  class JobId {
  public:
    /**
      Construct a default job id with usage of uuids.
    */
    JobId();

    /**
      Construct a jobid out of a string
    */
    JobId(const std::string &s);

    /**
      Construct a jobid out of a constant char*
    */
    JobId(const char *s);

    /**
      Copy-construct a jobid
    */
    JobId(const JobId &other);

    /**
      Assign a jobid to this one.
    */
    JobId &operator=(const JobId &rhs);

    /**
      Assign a string to this one.
    */
    JobId &operator=(const std::string &rhs);

    /**
      Assign a const char* to this one.
    */
    JobId &operator=(const char *rhs);

    /**
      Auto convert the jobid back to a std::string
    */
    operator std::string() const { return id_; }

    /**
      Unequal operator.
    */
    bool operator!=(const JobId& rhs) const;

    /**
      Equal operator.
    */
    bool operator==(const JobId& rhs) const;

    /**
      Strict less than operator.
    */
    bool operator<(const JobId& rhs) const;

    ~JobId();

    const std::string &str() const;
  private:
    std::string id_;
  };
}

extern std::ostream & operator<<(std::ostream &, const sdpa::JobId &);

#endif
