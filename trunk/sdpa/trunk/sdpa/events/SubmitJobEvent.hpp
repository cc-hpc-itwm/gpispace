#ifndef SDPA_SubmitJobEvent_HPP
#define SDPA_SubmitJobEvent_HPP

#include <iostream>
#include <boost/statechart/event.hpp>
#include <sdpa/events/MgmtEvent.hpp>
#include <sdpa/types.hpp>

namespace sc = boost::statechart;

namespace sdpa {
namespace events {
    class SubmitJobEvent : public sdpa::events::MgmtEvent, public sc::event<sdpa::events::SubmitJobEvent> {
    public:
        typedef sdpa::shared_ptr<SubmitJobEvent> Ptr;

        SubmitJobEvent(const address_t& from, const address_t& to) : MgmtEvent(from, to) {
			//std::cout << "Create event 'SubmitJobEvent'"<< std::endl;
        }

    	virtual ~SubmitJobEvent() {
    		//std::cout << "Delete event 'SubmitJobEvent'"<< std::endl;
    	}

    	std::string str() const { return "SubmitJobEvent"; }

    	const sdpa::job_desc_t & description() const {return desc_;}

    private:
    	sdpa::job_desc_t desc_;
    };
}}

#endif
