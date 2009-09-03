#ifndef SDPA_SubmitJobEvent_HPP
#define SDPA_SubmitJobEvent_HPP

#include <iostream>
#include <boost/statechart/event.hpp>
#include <sdpa/events/JobEvent.hpp>
#include <sdpa/types.hpp>

namespace sc = boost::statechart;

namespace sdpa {
namespace events {
    class SubmitJobEvent : public sdpa::events::JobEvent, public sc::event<sdpa::events::SubmitJobEvent> {
    public:
        typedef sdpa::shared_ptr<SubmitJobEvent> Ptr;

        SubmitJobEvent( const address_t& from, const address_t& to,
        		        const sdpa::job_id_t& job_id = sdpa::job_id_t(""), const job_desc_t& description = sdpa::job_desc_t("")) :
        	sdpa::events::JobEvent( from, to, job_id ), desc_(description)  {
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
