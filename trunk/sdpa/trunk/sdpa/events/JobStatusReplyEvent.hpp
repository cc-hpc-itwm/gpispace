#ifndef SDPA_ReplyJobStatusEvent_HPP
#define SDPA_ReplyJobStatusEvent_HPP

#include <boost/statechart/event.hpp>
#include <sdpa/events/JobEvent.hpp>
#include <sdpa/types.hpp>
namespace sc = boost::statechart;

namespace sdpa { namespace events {
  class JobStatusReplyEvent : public sdpa::events::JobEvent, public sc::event<sdpa::events::JobStatusReplyEvent>
  {
    public:
      typedef sdpa::shared_ptr<JobStatusReplyEvent> Ptr;
      typedef sdpa::status_t status_t;

      JobStatusReplyEvent(const address_t& a_from, const address_t& a_to, const sdpa::job_id_t& a_job_id, const status_t &a_status = status_t())
        : sdpa::events::JobEvent(a_from, a_to, a_job_id)
        , status_(a_status)
      { }

      virtual ~JobStatusReplyEvent() { }

      std::string str() const { return "JobStatusReplyEvent"; }

      const status_t &status() const { return status_; }
    private:
      status_t status_;
  };
}}

#endif
