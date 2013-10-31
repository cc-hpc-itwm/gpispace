#ifndef SDPA_JOB_ID_HPP
#define SDPA_JOB_ID_HPP 1

#include <string>
#include <ostream>
#include <boost/serialization/shared_ptr.hpp>     // shared_ptr serialization
#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/assume_abstract.hpp>
#include <boost/functional/hash.hpp>

namespace sdpa {
  class JobId {
  public:
    JobId();
    JobId(const std::string &s);
    JobId(const char *s);

    operator std::string() const { return id_; }

    bool operator!=(const JobId& rhs) const;
    bool operator==(const JobId& rhs) const;

    bool operator<(const JobId& rhs) const;

    const std::string &str() const { return id_; }
    std::string &str() { return id_; }

    static JobId invalid_job_id() { return "-"; }
    static bool is_invalid_job_id(const JobId &job_id) { return job_id == "-"; }

    template <class Archive>
	void serialize(Archive& ar, unsigned int /* version */)
	{
    	ar &  boost::serialization::make_nvp("JobID", id_);
	}


  private:
    std::string id_;
  };

  inline std::size_t hash_value(const JobId & a)
  {
    boost::hash<std::string> hasher;
    return hasher(a.str());
  }
}

inline std::ostream & operator<<(std::ostream &os, const sdpa::JobId &jid)
{
  os << jid.str();
  return os;
}

#endif
