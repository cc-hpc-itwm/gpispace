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
			//std::cout << "Create event 'SubmitJobAckEvent'"<< std::endl;
        }

    	virtual ~SubmitJobAckEvent() {
    		//std::cout << "Delete event 'SubmitJobAckEvent'"<< std::endl;
    	}

    	const sdpa::job_id_t & job_id() const { return job_id_; }

    	std::string str() const { return "SubmitJobAckEvent"; }
    private:
    	sdpa::job_id_t job_id_;
    };
}}

#endif
