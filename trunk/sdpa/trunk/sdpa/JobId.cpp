#include "JobId.hpp"
#include "uuidgen.hpp"

using namespace sdpa;

JobId::JobId()
{
  uuid uid;
  uuidgen gen;
  gen(uid);
  id_ = uid.str();
}

JobId::JobId(const std::string &s)
  : id_(s)
{

}

JobId::JobId(const char *s)
  : id_(s)
{
}

JobId::JobId(const JobId &other)
  : id_(other.str())
{
}

const std::string &JobId::str() const
{
  return id_;
}

const JobId & JobId::operator=(const JobId &rhs)
{
  if (this != &rhs) 
  {
    id_ = rhs.id_;
  }
  return *this;
}

JobId::operator std::string()
{
  return str();
}

bool JobId::operator!=(const JobId &rhs)
{
  return id_ != rhs.id_;
}

JobId::~JobId()
{
}
