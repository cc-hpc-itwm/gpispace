#ifndef SDPA_JOB_FAILED_ACK_EVENT_HPP
#define SDPA_JOB_FAILED_ACK_EVENT_HPP 1

#include <sdpa/sdpa-config.hpp>

#include <sdpa/events/JobEvent.hpp>

namespace sdpa
{
  namespace events
  {
    class JobFailedAckEvent : public JobEvent
    {
    public:
      typedef sdpa::shared_ptr<JobFailedAckEvent> Ptr;

      JobFailedAckEvent()
        : JobEvent ("", "", "")
      {}

      JobFailedAckEvent ( const address_t& a_from
                        , const address_t& a_to
                        , const sdpa::job_id_t& a_job_id
                        )
        :  sdpa::events::JobEvent (a_from, a_to, a_job_id)
      {}

      std::string str() const
      {
        return "JobFailedAckEvent(" + job_id ().str () + ")";
      }

      virtual void handleBy (EventHandler* handler)
      {
        handler->handleJobFailedAckEvent (this);
      }

    private:
      friend class boost::serialization::access;
      template <class Archive>
      void serialize (Archive & ar, unsigned int)
      {
        ar & boost::serialization::base_object<JobEvent> (*this);
      }
    };
  }
}

#endif
