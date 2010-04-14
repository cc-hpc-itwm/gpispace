#ifndef SDPA_SubmitJobAckEvent_HPP
#define SDPA_SubmitJobAckEvent_HPP

#include <sdpa/sdpa-config.hpp>

#ifdef USE_BOOST_SC
#   include <boost/statechart/event.hpp>
namespace sc = boost::statechart;
#endif
#include <sdpa/events/JobEvent.hpp>
#include <sdpa/events/EventHandler.hpp>

namespace sdpa { namespace events {
#ifdef USE_BOOST_SC
  class SubmitJobAckEvent : public JobEvent, public sc::event<SubmitJobAckEvent>
#else
  class SubmitJobAckEvent : public JobEvent
#endif
  {
    public:
      typedef sdpa::shared_ptr<SubmitJobAckEvent> Ptr;

      SubmitJobAckEvent()
        : JobEvent("", "", "", message_id_type())
      { }

      SubmitJobAckEvent(const address_t& a_from, const address_t& a_to, const sdpa::job_id_t & a_job_id, const message_id_type &mid)
        : JobEvent(a_from, a_to, a_job_id, mid)
      {
      }

      virtual ~SubmitJobAckEvent() {
      }

      std::string str() const { return "SubmitJobAckEvent"; }

      virtual void handleBy(DaemonEventHandler *handler)
      {
        handler->handleSubmitJobAckEvent(this);
      }
  };
}}

#endif
