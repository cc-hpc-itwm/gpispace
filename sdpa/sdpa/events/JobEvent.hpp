#pragma once

#include <string>

#include <sdpa/types.hpp>
#include <sdpa/events/SDPAEvent.hpp>

namespace sdpa
{
  namespace events
  {
    class JobEvent : public sdpa::events::SDPAEvent
    {
    public:
      JobEvent (const sdpa::job_id_t& a_job_id)
        : SDPAEvent()
        , job_id_ (a_job_id)
      {}

      const sdpa::job_id_t& job_id() const
      {
        return job_id_;
      }

    private:
      sdpa::job_id_t job_id_;
    };
  }
}
