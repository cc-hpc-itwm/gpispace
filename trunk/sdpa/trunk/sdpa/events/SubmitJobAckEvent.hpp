#ifndef SDPA_SubmitJobAckEvent_HPP
#define SDPA_SubmitJobAckEvent_HPP

#include <boost/statechart/event.hpp>
#include <sdpa/events/MgmtEvent.hpp>

namespace sc = boost::statechart;

namespace sdpa {
namespace events {
    class SubmitJobAckEvent : public sdpa::events::JobEvent, public sc::event<sdpa::events::SubmitJobAckEvent> {
    public:
        typedef sdpa::shared_ptr<SubmitJobAckEvent> Ptr;

        SubmitJobAckEvent(const address_t& a_from, const address_t& a_to, const sdpa::job_id_t & a_job_id)
          : JobEvent(a_from, a_to, a_job_id)
        {
        }

    	virtual ~SubmitJobAckEvent() {
    	}

    	std::string str() const { return "SubmitJobAckEvent"; }
    };
}}

#endif
