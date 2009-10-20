#ifndef SDPA_WORKER_REGISTRATION_ACK_EVENT_HPP
#define SDPA_WORKER_REGISTRATION_ACK_EVENT_HPP 1

#include <boost/statechart/event.hpp>
#include <sdpa/events/MgmtEvent.hpp>
namespace sc = boost::statechart;

namespace sdpa { namespace events {
  class WorkerRegistrationAckEvent : public sdpa::events::MgmtEvent, public sc::event<sdpa::events::WorkerRegistrationAckEvent> {
    public:
      typedef sdpa::shared_ptr<WorkerRegistrationAckEvent> Ptr;

      WorkerRegistrationAckEvent(const address_t& a_from, const address_t& a_to) : MgmtEvent(a_from, a_to) { }

      virtual ~WorkerRegistrationAckEvent() { }

      std::string str() const { return "WorkerRegistrationAckEvent"; }
  };
}}

#endif
