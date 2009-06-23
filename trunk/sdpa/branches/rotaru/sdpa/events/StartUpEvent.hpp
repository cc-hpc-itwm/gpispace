#ifndef SDPA_STARTUPEVENT_HPP
#define SDPA_STARTUPEVENT_HPP

#include <iostream>
#include <boost/statechart/event.hpp>
#include <MgmtEvent.hpp>

namespace sc = boost::statechart;

namespace sdpa {
namespace events {
    class StartUpEvent : public sdpa::events::MgmtEvent, public sc::event<sdpa::events::StartUpEvent> {
    public:
        typedef sdpa::shared_ptr<StartUpEvent> Ptr;

        StartUpEvent(const address_t& from, const address_t& to) : MgmtEvent(from, to) {
			std::cout << "Create event 'StartUpEvent'"<< std::endl; }

    	virtual ~StartUpEvent() { std::cout << "Delete event 'StartUpEvent'"<< std::endl; }

    	std::string str() const { std::cout<<from()<<" - StartUpEvent -> "<<to()<<std::endl; }
    };
}}

#endif
