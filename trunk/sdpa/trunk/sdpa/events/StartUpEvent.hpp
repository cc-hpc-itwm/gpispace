#ifndef SDPA_STARTUPEVENT_HPP
#define SDPA_STARTUPEVENT_HPP

#include <boost/statechart/event.hpp>
#include <sdpa/events/MgmtEvent.hpp>

namespace sc = boost::statechart;

namespace sdpa { namespace events {
  class StartUpEvent : public sdpa::events::MgmtEvent, public sc::event<sdpa::events::StartUpEvent> {
    public:
      typedef sdpa::shared_ptr<StartUpEvent> Ptr;

      StartUpEvent(const address_t& from, const address_t& to) : MgmtEvent(from, to) { }

      virtual ~StartUpEvent() { }

      std::string str() const { return "StartUpEvent"; }
  };
}}

#endif
