#ifndef SDPA_RESULTS_REPLY_EVENT_HPP
#define SDPA_RESULTS_REPLY_EVENT_HPP 1

#include <sdpa/sdpa-config.hpp>

#include <sdpa/events/JobEvent.hpp>
#include <sdpa/events/EventHandler.hpp>

namespace sdpa
{
  namespace events
  {
    class JobResultsReplyEvent : public JobEvent
    {
    public:
      typedef sdpa::shared_ptr<JobResultsReplyEvent> Ptr;

      JobResultsReplyEvent()
        : JobEvent ("", "", "")
      {}

      JobResultsReplyEvent ( const address_t& a_from
                           , const address_t& a_to
                           , const sdpa::job_id_t& a_job_id
                           , const job_result_t& a_result
                           )
        : sdpa::events::JobEvent (a_from, a_to, a_job_id)
        , result_ (a_result)
      {}

      std::string str() const
      {
        return "JobResultsReplyEvent(" + job_id ().str () + ")";
      }

      virtual void handleBy (EventHandler* handler)
      {
        handler->handleJobResultsReplyEvent (this);
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
