#ifndef SDPA_CONFIGNOKEVENT_HPP
#define SDPA_CONFIGNOKEVENT_HPP 1

#include <iostream>
#include <boost/statechart/event.hpp>
#include <sdpa/events/MgmtEvent.hpp>

namespace sc = boost::statechart;

namespace sdpa {
namespace events {
    class ConfigNokEvent : public sdpa::events::MgmtEvent, public sc::event<sdpa::events::ConfigNokEvent> {
    public:
        typedef sdpa::shared_ptr<ConfigNokEvent> Ptr;

        ConfigNokEvent(const address_t& from, const address_t& to) : MgmtEvent(from, to) {
        	//std::cout << "Create event 'ConfigNokEvent'"<< std::endl;
        }

    	virtual ~ConfigNokEvent() {
    		//std::cout << "Delete event 'ConfigNokEvent'"<< std::endl;
    	}

    	std::string str() const { return "ConfigNokEvent"; }
    };
}}

#endif
