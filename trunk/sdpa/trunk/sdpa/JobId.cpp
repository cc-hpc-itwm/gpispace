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

const JobId & JobId::operator=(const std::string &rhs)
{
  id_ = rhs;
  return *this;
}

const JobId & JobId::operator=(const char *rhs)
{
  id_ = rhs;
  return *this;
}

bool JobId::operator!=(const JobId &rhs) const
{
  return id_ != rhs.id_;
}

bool JobId::operator<(const JobId &rhs) const
{
  return id_ < rhs.id_;
}

bool JobId::operator==(const JobId &rhs) const
{
  return id_ == rhs.id_;
}

JobId::~JobId()
{
}

std::ostream & operator<<(std::ostream &os, const sdpa::JobId &jid) {
  os << jid.str();
  return os;
}

