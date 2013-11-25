#ifndef SDPA_JOB_EVENT_HPP
#define SDPA_JOB_EVENT_HPP 1

#include <string>

#include <sdpa/memory.hpp>
#include <sdpa/types.hpp>
#include <sdpa/events/SDPAEvent.hpp>

namespace sdpa
{
  namespace events
  {
    class JobEvent : public sdpa::events::SDPAEvent
    {
    public:
      JobEvent ( const address_t& a_from
               , const address_t& a_to
               , const sdpa::job_id_t& a_job_id
               )
        : SDPAEvent (a_from, a_to)
        , job_id_ (a_job_id)
      {}

      const sdpa::job_id_t& job_id() const
      {
        return job_id_;
      }

      virtual int priority() const
      {
        return 1;
      }

    private:
      sdpa::job_id_t job_id_;
    };
  }
}

#endif
