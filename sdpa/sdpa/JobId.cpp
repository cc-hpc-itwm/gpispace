#include "JobId.hpp"
#include  <sdpa/id_generator.hpp>

using namespace sdpa;

namespace
{
  struct job_id_tag
  {
    static const char *name ()
    {
      return "job";
    }
  };
}

JobId JobId::create_unique_id()
{
  return JobId (sdpa::id_generator<job_id_tag>::instance().next());
}

JobId::JobId() : id_(create_unique_id().str())
{
}

JobId::JobId(const std::string &s)
  : id_(s)
{
}

JobId::JobId(const char *s)
  : id_(s)
{
}

bool JobId::operator!=(const JobId &rhs) const
{
	return !(*this == rhs);
}

bool JobId::operator<(const JobId &rhs) const
{
	return id_ < rhs.id_;
}

bool JobId::operator==(const JobId &rhs) const
{
	return id_ == rhs.id_;
}
