#ifndef SDPA_INTERRUPTEVENT_HPP
#define SDPA_INTERRUPTEVENT_HPP 1

#include <sdpa/sdpa-config.hpp>

#ifdef USE_BOOST_SC
#   include <boost/statechart/event.hpp>
namespace sc = boost::statechart;
#endif

#include <sdpa/events/MgmtEvent.hpp>

namespace sdpa { namespace events {
#ifdef USE_BOOST_SC
  class InterruptEvent : public MgmtEvent, public sc::event<InterruptEvent>
#else
  class InterruptEvent : public MgmtEvent
#endif
  {
  public:
    typedef sdpa::shared_ptr<InterruptEvent> Ptr;

    InterruptEvent(const address_t& a_from, const address_t& a_to) : MgmtEvent(a_from, a_to) { }

    virtual ~InterruptEvent() { }

    std::string str() const { return "InterruptEvent"; }

    virtual void accept(EventVisitor *visitor)
    {
      visitor->visitInterruptEvent(this);
    }
  };
}}

#endif
