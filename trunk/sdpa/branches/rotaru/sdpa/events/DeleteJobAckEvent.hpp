#ifndef SDPA_DeleteJobAckEvent_HPP
#define SDPA_DeleteJobAckEvent_HPP 1

#include <iostream>
#include <boost/statechart/event.hpp>
#include <sdpa/events/MgmtEvent.hpp>

namespace sc = boost::statechart;

namespace sdpa {
namespace events {
    class DeleteJobAckEvent : public sdpa::events::JobEvent, public sc::event<sdpa::events::DeleteJobAckEvent> {
    public:
        typedef sdpa::shared_ptr<DeleteJobAckEvent> Ptr;

        DeleteJobAckEvent(const address_t& from, const address_t& to, const sdpa::job_id_t& job_id = sdpa::job_id_t(""))
              :  sdpa::events::JobEvent( from, to, job_id ) {
        	//std::cout << "Create event 'DeleteJobAckEvent'"<< std::endl;
        }

    	virtual ~DeleteJobAckEvent() {
				//std::cout << "Delete event 'DeleteJobAckEvent'"<< std::endl;
    	}

    	std::string str() const { return "DeleteJobAckEvent"; }
    };
}}

#endif
