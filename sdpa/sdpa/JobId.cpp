#include "JobId.hpp"

using namespace sdpa;

JobId::JobId() : id_(invalid_job_id())
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
