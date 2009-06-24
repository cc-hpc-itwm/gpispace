#ifndef SDPA_DELETEJOBEVENT_HPP
#define SDPA_DELETEJOBEVENT_HPP

#include <iostream>
#include <boost/statechart/event.hpp>
#include <sdpa/events/MgmtEvent.hpp>

namespace sc = boost::statechart;

namespace sdpa {
namespace events {
    class DeleteJobEvent : public sdpa::events::MgmtEvent, public sc::event<sdpa::events::DeleteJobEvent> {
    public:
        typedef sdpa::shared_ptr<DeleteJobEvent> Ptr;

        DeleteJobEvent(const address_t &from, const address_t &to) : MgmtEvent(from, to) {
        	std::cout << "Create event 'DeleteJobEvent'"<< std::endl; }

    	virtual ~DeleteJobEvent() { std::cout << "Delete event 'DeleteJobEvent'"<< std::endl; }

    	std::string str() const { std::cout<<from()<<" - DeleteJobEvent -> "<<to()<<std::endl; }
    };
}}

#endif
