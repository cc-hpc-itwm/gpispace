#ifndef SDPA_STARTUPEVENT_HPP
#define SDPA_STARTUPEVENT_HPP

#include <sdpa/sdpa-config.hpp>

#ifdef USE_BOOST_SC
#   include <boost/statechart/event.hpp>
namespace sc = boost::statechart;
#endif

#include <sdpa/events/MgmtEvent.hpp>
#include <sdpa/memory.hpp>

namespace sdpa { namespace events {
#ifdef USE_BOOST_SC
  class StartUpEvent : public MgmtEvent, public sc::event<sdpa::events::StartUpEvent> {
#else
  class StartUpEvent : public MgmtEvent {
#endif
    public:
      typedef sdpa::shared_ptr<StartUpEvent> Ptr;

      StartUpEvent() : MgmtEvent("","") { }

      virtual ~StartUpEvent() { }

      std::string str() const { return "StartUpEvent"; }

      virtual void accept(EventVisitor *visitor)
      {
        visitor->visitStartUpEvent(this);
      }
  };
}}

#endif
