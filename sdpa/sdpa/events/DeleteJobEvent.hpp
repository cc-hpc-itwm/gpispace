#ifndef SDPA_DELETE_JOB_EVENT_HPP
#define SDPA_DELETE_JOB_EVENT_HPP 1

#include <sdpa/events/JobEvent.hpp>

namespace sdpa
{
  namespace events
  {
    class DeleteJobEvent : public JobEvent
    {
    public:
      typedef boost::shared_ptr<DeleteJobEvent> Ptr;

      DeleteJobEvent ( const address_t& from
                     , const address_t& to
                     , const sdpa::job_id_t& job_id
                     )
        : sdpa::events::JobEvent (from, to, job_id)
      {}

      std::string str() const
      {
        return "DeleteJobEvent(" + job_id ().str () + ")";
      }

      virtual void handleBy (EventHandler* handler)
      {
        handler->handleDeleteJobEvent (this);
      }
    };

    CONSTRUCT_DATA_DEFS_FOR_EMPTY_JOBEVENT_OVERLOAD (DeleteJobEvent)
  }
}

#endif
