#ifndef SDPA_SUBMITACKEVENT_HPP
#define SDPA_SUBMITACKEVENT_HPP

#include <iostream>
#include <boost/statechart/event.hpp>
#include <sdpa/events/MgmtEvent.hpp>

namespace sc = boost::statechart;

namespace sdpa {
namespace events {
    class SubmitAckEvent : public sdpa::events::MgmtEvent, public sc::event<sdpa::events::SubmitAckEvent> {
    public:
        typedef sdpa::shared_ptr<SubmitAckEvent> Ptr;

        SubmitAckEvent(const address_t& from, const address_t& to) : MgmtEvent(from, to) {
			std::cout << "Create event 'SubmitAckEvent'"<< std::endl; }

    	virtual ~SubmitAckEvent() { std::cout << "Delete event 'SubmitAckEvent'"<< std::endl; }

    	std::string str() const { std::cout<<from()<<" - SubmitAckEvent -> "<<to()<<std::endl; }
    };
}}

#endif
