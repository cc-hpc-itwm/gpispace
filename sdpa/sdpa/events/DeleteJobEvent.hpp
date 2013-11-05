#ifndef SDPA_DELETE_JOB_EVENT_HPP
#define SDPA_DELETE_JOB_EVENT_HPP 1

#include <sdpa/sdpa-config.hpp>

#include <sdpa/events/JobEvent.hpp>

namespace sdpa
{
  namespace events
  {
    class DeleteJobEvent : public JobEvent
    {
    public:
      typedef sdpa::shared_ptr<DeleteJobEvent> Ptr;

      DeleteJobEvent()
        : JobEvent ("","","")
      {}

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

    private:
      friend class boost::serialization::access;
      template <typename Archive>
      void serialize (Archive& ar, const unsigned int)
      {
        ar & boost::serialization::base_object<JobEvent> (*this);
      }
    };
  }
}

#endif
