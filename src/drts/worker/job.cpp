#include "job.hpp"

namespace drts
{
  Job::Job( Job::ID const &jobid
          , Job::Description const &description
          , Job::Owner const &owner
          )
    : m_id (jobid.value)
    , m_input_description (description.value)
    , m_owner (owner.value)
    , m_state (Job::PENDING)
    , m_result ()
    , m_message ("")
  {}

  Job::state_t Job::cmp_and_swp_state( Job::state_t expected
                                     , Job::state_t newstate
                                     )
  {
    lock_type lock (m_mutex);
    state_t old_state = m_state;
    if (old_state == expected)
    {
      m_state = newstate;
    }
    return old_state;
  }
}
