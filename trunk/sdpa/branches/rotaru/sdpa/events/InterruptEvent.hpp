#ifndef SDPA_INTERRUPTEVENT_HPP
#define SDPA_INTERRUPTEVENT_HPP 1

#include <boost/statechart/event.hpp>
#include <sdpa/events/MgmtEvent.hpp>

namespace sc = boost::statechart;

namespace sdpa {
namespace events {
    class InterruptEvent : public sdpa::events::MgmtEvent, public sc::event<sdpa::events::InterruptEvent> {
    public:
        typedef sdpa::shared_ptr<InterruptEvent> Ptr;

        InterruptEvent(const address_t& a_from, const address_t& a_to) : MgmtEvent(a_from, a_to) { }

    	virtual ~InterruptEvent() { }

    	std::string str() const { return "InterruptEvent"; }
    };
}}

#endif
