#ifndef SDPA_WORKER_REGISTRATION_ACK_EVENT_HPP
#define SDPA_WORKER_REGISTRATION_ACK_EVENT_HPP 1

#include <iostream>
#include <boost/statechart/event.hpp>
#include <sdpa/events/MgmtEvent.hpp>
namespace sc = boost::statechart;

namespace sdpa {
namespace events {
    class WorkerRegistrationAckEvent : public sdpa::events::MgmtEvent, public sc::event<sdpa::events::WorkerRegistrationAckEvent> {
    public:
        typedef sdpa::shared_ptr<WorkerRegistrationAckEvent> Ptr;

        WorkerRegistrationAckEvent(const address_t& from, const address_t& to) : MgmtEvent(from, to) {
			//std::cout << "Create event 'WorkerRegistrationAckEvent'"<< std::endl;
        }

    	virtual ~WorkerRegistrationAckEvent() {
    		//std::cout << "Delete event 'WorkerRegistrationAckEvent'"<< std::endl;
    	}

    	std::string str() const { return "WorkerRegistrationAckEvent"; }
    private:
    };
}}

#endif
