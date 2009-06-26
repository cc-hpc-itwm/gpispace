#ifndef SDPA_SubmitJobAckEvent_HPP
#define SDPA_SubmitJobAckEvent_HPP

#include <iostream>
#include <boost/statechart/event.hpp>
#include <sdpa/events/MgmtEvent.hpp>

namespace sc = boost::statechart;

namespace sdpa {
namespace events {
    class SubmitJobAckEvent : public sdpa::events::MgmtEvent, public sc::event<sdpa::events::SubmitJobAckEvent> {
    public:
        typedef sdpa::shared_ptr<SubmitJobAckEvent> Ptr;

        SubmitJobAckEvent(const address_t& from, const address_t& to) : MgmtEvent(from, to) {
			std::cout << "Create event 'SubmitJobAckEvent'"<< std::endl; }

    	virtual ~SubmitJobAckEvent() { std::cout << "Delete event 'SubmitJobAckEvent'"<< std::endl; }

    	std::string str() const { std::cout<<from()<<" - SubmitJobAckEvent -> "<<to()<<std::endl; }
    };
}}

#endif
