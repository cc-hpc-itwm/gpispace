#ifndef SDPA_INTERRUPTEVENT_HPP
#define SDPA_INTERRUPTEVENT_HPP 1

#include <iostream>
#include <boost/statechart/event.hpp>
#include <sdpa/events/MgmtEvent.hpp>

namespace sc = boost::statechart;

namespace sdpa {
namespace events {
    class InterruptEvent : public sdpa::events::MgmtEvent, public sc::event<sdpa::events::InterruptEvent> {
    public:
        typedef sdpa::shared_ptr<InterruptEvent> Ptr;

        InterruptEvent(const address_t& from, const address_t& to) : MgmtEvent(from, to) {
			//std::cout << "Create event 'InterruptEvent'"<< std::endl;
        }

    	virtual ~InterruptEvent() {
    		//std::cout << "Delete event 'InterruptEvent'"<< std::endl;
    	}

    	std::string str() const { return "InterruptEvent"; }
    };
}}

#endif
