#ifndef SDPA_JOB_FINISHED_EVENT_HPP
#define SDPA_JOB_FINISHED_EVENT_HPP 1

#include <boost/statechart/event.hpp>
#include <sdpa/events/JobEvent.hpp>

namespace sc = boost::statechart;

namespace sdpa { namespace events {
  class JobFinishedEvent : public sdpa::events::JobEvent, public sc::event<sdpa::events::JobFinishedEvent> {
    public:
      typedef sdpa::shared_ptr<JobFinishedEvent> Ptr;

      JobFinishedEvent(const address_t &a_from
                     , const address_t &a_to
                     , const sdpa::job_id_t &a_job_id = sdpa::job_id_t())
        : sdpa::events::JobEvent( a_from, a_to, a_job_id ) { }

      virtual ~JobFinishedEvent() { }

      std::string str() const { return "JobFinishedEvent"; }
  };
}}

#endif
