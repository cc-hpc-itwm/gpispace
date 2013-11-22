#ifndef SDPA_JOB_FINISHED_EVENT_HPP
#define SDPA_JOB_FINISHED_EVENT_HPP 1

#include <sdpa/events/JobEvent.hpp>
#include <sdpa/events/EventHandler.hpp>

namespace sdpa
{
  namespace events
  {
    class JobFinishedEvent : public JobEvent
    {
    public:
      typedef sdpa::shared_ptr<JobFinishedEvent> Ptr;

      JobFinishedEvent()
        : JobEvent ("", "", "")
      {}
      JobFinishedEvent ( const address_t& a_from
                       , const address_t& a_to
                       , const sdpa::job_id_t& a_job_id
                       , const job_result_t& job_result
                       )
        : sdpa::events::JobEvent (a_from, a_to, a_job_id)
        , result_ (job_result)
      {}

      std::string str() const
      {
        return "JobFinishedEvent(" + job_id ().str () + ")";
      }

      virtual void handleBy (EventHandler* handler)
      {
        handler->handleJobFinishedEvent (this);
      }

      const job_result_t& result() const
      {
        return result_;
      }

    private:
      job_result_t result_;

      friend class boost::serialization::access;
      template <class Archive>
      void serialize (Archive & ar, unsigned int)
      {
        ar & boost::serialization::base_object<JobEvent> (*this);
        ar & result_;
      }
    };
  }
}

#endif
