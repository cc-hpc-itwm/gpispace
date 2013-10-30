#ifndef SDPA_QUERYJOBSTATUSEVENT_HPP
#define SDPA_QUERYJOBSTATUSEVENT_HPP 1

#include <sdpa/sdpa-config.hpp>

#include <sdpa/events/JobEvent.hpp>
#include <sdpa/events/EventHandler.hpp>

namespace sdpa
{
  namespace events
  {
    class QueryJobStatusEvent : public JobEvent
    {
    public:
      typedef sdpa::shared_ptr<QueryJobStatusEvent> Ptr;

      QueryJobStatusEvent()
        : JobEvent ("", "", "")
      {}

      QueryJobStatusEvent ( const address_t& a_from
                          , const address_t& a_to
                          , const sdpa::job_id_t& a_job_id = sdpa::job_id_t()
                          )
        :  sdpa::events::JobEvent (a_from, a_to, a_job_id)
      {}

      std::string str() const
      {
        return "QueryJobStatusEvent(" + job_id ().str () + ")";
      }

      virtual void handleBy (EventHandler* handler)
      {
        handler->handleQueryJobStatusEvent (this);
      }
    };
  }
}

#endif
