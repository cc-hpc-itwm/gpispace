#ifndef SDPA_CONFIGOKEVENT_HPP
#define SDPA_CONFIGOKEVENT_HPP

#include <iostream>
#include <boost/statechart/event.hpp>
#include <sdpa/events/MgmtEvent.hpp>

namespace sc = boost::statechart;

namespace sdpa {
namespace events {
    class ConfigOkEvent : public sdpa::events::MgmtEvent, public sc::event<sdpa::events::ConfigOkEvent> {
    public:
        typedef sdpa::shared_ptr<ConfigOkEvent> Ptr;

        ConfigOkEvent(const address_t& from, const address_t& to) : MgmtEvent(from, to) {
        	std::cout << "Create event 'ConfigOkEvent'"<< std::endl; }

    	virtual ~ConfigOkEvent() { std::cout << "Delete event 'ConfigOkEvent'"<< std::endl; }

    	std::string str() const { std::cout<<from()<<" - ConfigOkEvent -> "<<to()<<std::endl; }
    };
}}

#endif
