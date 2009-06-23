#ifndef SDPA_REQUESTJOBEVENT_HPP
#define SDPA_REQUESTJOBEVENT_HPP

#include <iostream>
#include <boost/statechart/event.hpp>
#include <MgmtEvent.hpp>
namespace sc = boost::statechart;

namespace sdpa {
namespace events {
    class RequestJobEvent : public sdpa::events::MgmtEvent, public sc::event<sdpa::events::RequestJobEvent> {
    public:
        typedef sdpa::shared_ptr<RequestJobEvent> Ptr;

        RequestJobEvent(const address_t& from, const address_t& to) : MgmtEvent(from, to) {
			std::cout << "Create event 'RequestJobEvent'"<< std::endl; }

    	virtual ~RequestJobEvent() { std::cout << "Delete event 'RequestJobEvent'"<< std::endl; }

    	std::string str() const { std::cout<<from()<<" - RequestJobEvent -> "<<to()<<std::endl; }
    };
}}

#endif
