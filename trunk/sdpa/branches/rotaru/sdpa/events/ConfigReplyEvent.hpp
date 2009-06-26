#ifndef SDPA_ConfigReplyEvent_HPP
#define SDPA_ConfigReplyEvent_HPP

#include <iostream>
#include <boost/statechart/event.hpp>
#include <sdpa/events/MgmtEvent.hpp>

namespace sc = boost::statechart;

namespace sdpa {
namespace events {
    class ConfigReplyEvent : public sdpa::events::MgmtEvent, public sc::event<sdpa::events::ConfigReplyEvent> {
    public:
        typedef sdpa::shared_ptr<ConfigReplyEvent> Ptr;

        ConfigReplyEvent(const address_t& from, const address_t& to) : MgmtEvent(from, to) {
        	std::cout << "Create event 'ConfigReplyEvent'"<< std::endl; }

    	virtual ~ConfigReplyEvent() { std::cout << "Delete event 'ConfigReplyEvent'"<< std::endl; }

    	std::string str() const { std::cout<<from()<<" - ConfigReplyEvent -> "<<to()<<std::endl; }
    };
}}

#endif
