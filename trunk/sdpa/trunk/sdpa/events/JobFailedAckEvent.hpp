#ifndef SDPA_JOB_FAILED_ACK_EVENT_HPP
#define SDPA_JOB_FAILED_ACK_EVENT_HPP 1

#include <iostream>
#include <boost/statechart/event.hpp>
#include <sdpa/events/JobEvent.hpp>

namespace sc = boost::statechart;

namespace sdpa {
namespace events {
	class JobFailedAckEvent : public sdpa::events::JobEvent, public sc::event<sdpa::events::JobFailedAckEvent> {
	public:
		typedef sdpa::shared_ptr<JobFailedAckEvent> Ptr;

		JobFailedAckEvent(const address_t& from, const address_t& to, const sdpa::job_id_t& job_id = sdpa::job_id_t())
          :  sdpa::events::JobEvent( from, to, job_id ) {
			//std::cout << "Create event 'JobFailedAckEvent'"<< std::endl;
		}

		virtual ~JobFailedAckEvent() {
			//std::cout << "Delete event 'JobFailedAckEvent'"<< std::endl;
		}

		std::string str() const { return "JobFailedAckEvent"; }
	};
}}

#endif
