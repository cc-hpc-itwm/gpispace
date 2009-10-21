#ifndef SDPA_RUNJOBEVENT_HPP
#define SDPA_RUNJOBEVENT_HPP

#include <boost/statechart/event.hpp>
#include <sdpa/events/JobEvent.hpp>

namespace sc = boost::statechart;

namespace sdpa { namespace events {
  class RunJobEvent : public sdpa::events::JobEvent, public sc::event<RunJobEvent>
  {
    public:
      typedef sdpa::shared_ptr<RunJobEvent> Ptr;

      RunJobEvent(const address_t &a_from, const address_t& a_to, const sdpa::job_id_t& a_job_id = sdpa::job_id_t())
        : JobEvent(a_from, a_to, a_job_id) { }

      virtual ~RunJobEvent() { }

      std::string str() const { return "RunJobEvent"; }
  };
}}


#endif
