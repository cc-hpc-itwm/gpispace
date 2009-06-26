#ifndef SDPA_DeleteJobAckEvent_HPP
#define SDPA_DeleteJobAckEvent_HPP

#include <iostream>
#include <boost/statechart/event.hpp>
#include <sdpa/events/MgmtEvent.hpp>

namespace sc = boost::statechart;

namespace sdpa {
namespace events {
    class DeleteJobAckEvent : public sdpa::events::MgmtEvent, public sc::event<sdpa::events::DeleteJobAckEvent> {
    public:
        typedef sdpa::shared_ptr<DeleteJobAckEvent> Ptr;

        DeleteJobAckEvent(const address_t &from, const address_t &to) : MgmtEvent(from, to) {
        	std::cout << "Create event 'DeleteJobAckEvent'"<< std::endl; }

    	virtual ~DeleteJobAckEvent() { std::cout << "Delete event 'DeleteJobAckEvent'"<< std::endl; }

    	std::string str() const { std::cout<<from()<<" - DeleteJobAckEvent -> "<<to()<<std::endl; }
    };
}}

#endif
