#ifndef SDPA_RETRIEVEJOBRESULTSEVENT_HPP
#define SDPA_RETRIEVEJOBRESULTSEVENT_HPP 1

#include <sdpa/events/JobEvent.hpp>
#include <sdpa/events/EventHandler.hpp>

namespace sdpa
{
  namespace events
  {
    class RetrieveJobResultsEvent : public JobEvent
    {
    public:
      typedef boost::shared_ptr<RetrieveJobResultsEvent> Ptr;

      RetrieveJobResultsEvent
        ( const address_t& a_from
        , const address_t& a_to
        , const sdpa::job_id_t& a_job_id = sdpa::job_id_t()
        )
          : sdpa::events::JobEvent (a_from, a_to, a_job_id)
      {}

      std::string str() const
      {
        return "RetrieveJobResultsEvent(" + job_id () + ")";
      }

      virtual void handleBy (EventHandler* handler)
      {
        handler->handleRetrieveJobResultsEvent (this);
      }
    };

    CONSTRUCT_DATA_DEFS_FOR_EMPTY_JOBEVENT_OVERLOAD (RetrieveJobResultsEvent)
  }
}

#endif
