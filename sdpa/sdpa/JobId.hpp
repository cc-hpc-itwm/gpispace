#ifndef SDPA_JOB_ID_HPP
#define SDPA_JOB_ID_HPP 1

#include <string>
#include <ostream>
#include <boost/serialization/utility.hpp>
#include <boost/functional/hash.hpp>

namespace sdpa {
  class JobId {
  public:
    //! \todo Remove default ctor: used for serialization only
    JobId()
      : id_()
    {}

    JobId(const std::string &s)
      : id_(s)
    {}

    JobId(const char *s)
      : id_(s)
    {}

    bool operator==(const JobId &rhs) const
    {
      return id_ == rhs.id_;
    }
    bool operator!=(const JobId &rhs) const
    {
      return !(*this == rhs);
    }

    bool operator<(const JobId &rhs) const
    {
      return id_ < rhs.id_;
    }

    operator std::string() const { return id_; }
    const std::string &str() const { return id_; }

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
