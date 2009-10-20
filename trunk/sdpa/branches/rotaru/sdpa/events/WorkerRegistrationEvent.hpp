#ifndef SDPA_WORKER_REGISTRATION_EVENT_HPP
#define SDPA_WORKER_REGISTRATION_EVENT_HPP 1

#include <boost/statechart/event.hpp>
#include <sdpa/events/MgmtEvent.hpp>
namespace sc = boost::statechart;

namespace sdpa { namespace events {
  class WorkerRegistrationEvent : public sdpa::events::MgmtEvent, public sc::event<sdpa::events::WorkerRegistrationEvent> {
    public:
      typedef sdpa::shared_ptr<WorkerRegistrationEvent> Ptr;

      WorkerRegistrationEvent(const address_t& a_from, const address_t& a_to) : MgmtEvent(a_from, a_to) { }

      virtual ~WorkerRegistrationEvent() { }

      std::string str() const { return "WorkerRegistrationEvent"; }
  };
}}

#endif
