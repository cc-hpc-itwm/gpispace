#ifndef SDPA_WORKER_REGISTRATION_EVENT_HPP
#define SDPA_WORKER_REGISTRATION_EVENT_HPP 1

#include <iostream>
#include <boost/statechart/event.hpp>
#include <sdpa/events/MgmtEvent.hpp>
namespace sc = boost::statechart;

namespace sdpa {
namespace events {
    class WorkerRegistrationEvent : public sdpa::events::MgmtEvent, public sc::event<sdpa::events::WorkerRegistrationEvent> {
    public:
        typedef sdpa::shared_ptr<WorkerRegistrationEvent> Ptr;

        WorkerRegistrationEvent(const address_t& from, const address_t& to) : MgmtEvent(from, to) {
			//std::cout << "Create event 'WorkerRegistrationEvent'"<< std::endl;
        }

    	virtual ~WorkerRegistrationEvent() {
    		//std::cout << "Delete event 'WorkerRegistrationEvent'"<< std::endl;
    	}

    	std::string str() const { return "WorkerRegistrationEvent"; }
    private:
    };
}}

#endif
