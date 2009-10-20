#ifndef SDPA_RESULTS_REPLY_EVENT_HPP
#define SDPA_RESULTS_REPLY_EVENT_HPP 1

#include <boost/statechart/event.hpp>
#include <sdpa/events/JobEvent.hpp>
namespace sc = boost::statechart;

namespace sdpa { namespace events {
  class JobResultsReplyEvent : public sdpa::events::JobEvent, public sc::event<sdpa::events::JobResultsReplyEvent> {
    public:
      typedef sdpa::shared_ptr<JobResultsReplyEvent> Ptr;
      typedef std::string result_t;

      JobResultsReplyEvent(const address_t &a_from
                         , const address_t &a_to
                         , const sdpa::job_id_t &a_job_id
                         , const result_t &a_result)
        : sdpa::events::JobEvent(a_from, a_to, a_job_id), result_(a_result)
      { }

      virtual ~JobResultsReplyEvent() { }

      std::string str() const { return "JobResultsReplyEvent"; }
      const result_t &result() const { return result_; }
    private:
      result_t result_;
  };
}}

#endif
