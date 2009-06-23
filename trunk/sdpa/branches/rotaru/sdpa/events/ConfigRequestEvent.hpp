#ifndef SDPA_CONFIGREQUESTEVENT_HPP
#define SDPA_CONFIGREQUESTEVENT_HPP

#include <iostream>
#include <boost/statechart/event.hpp>
#include <MgmtEvent.hpp>

namespace sc = boost::statechart;

namespace sdpa {
namespace events {
    class ConfigRequestEvent : public sdpa::events::MgmtEvent, public sc::event<sdpa::events::ConfigRequestEvent> {
    public:
        typedef sdpa::shared_ptr<ConfigRequestEvent> Ptr;

        ConfigRequestEvent(const address_t& from, const address_t& to) : MgmtEvent(from, to) {
        	std::cout << "Create event 'ConfigRequestEvent'"<< std::endl; }

    	virtual ~ConfigRequestEvent() { std::cout << "Delete event 'ConfigRequestEvent'"<< std::endl; }

    	std::string str() const { std::cout<<from()<<" - ConfigRequestEvent -> "<<to()<<std::endl; }
    };
}}

#endif
