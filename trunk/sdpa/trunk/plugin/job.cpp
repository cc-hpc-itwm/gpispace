#include "job.hpp"

namespace drts
{
  Job::Job( Job::ID const &jobid
          , Job::Description const &description
          , Job::Owner const &owner
          )
    : m_id (jobid.value)
    , m_description (description.value)
    , m_owner (owner.value)
  {}
}
