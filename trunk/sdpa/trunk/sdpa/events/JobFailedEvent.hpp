#ifndef SDPA_JOB_FAILED_EVENT_HPP
#define SDPA_JOB_FAILED_EVENT_HPP 1

#include <iostream>
#include <boost/statechart/event.hpp>
#include <sdpa/events/JobEvent.hpp>

namespace sc = boost::statechart;

namespace sdpa {
namespace events {
	class JobFailedEvent : public sdpa::events::JobEvent, public sc::event<sdpa::events::JobFailedEvent> {
	public:
		typedef sdpa::shared_ptr<JobFailedEvent> Ptr;

		JobFailedEvent(const address_t& from, const address_t& to, const sdpa::job_id_t& job_id = sdpa::job_id_t())
          :  sdpa::events::JobEvent( from, to, job_id ) {
			//std::cout << "Create event 'JobFailedEvent'"<< std::endl;
		}

		virtual ~JobFailedEvent() {
			//std::cout << "Delete event 'JobFailedEvent'"<< std::endl;
		}

		std::string str() const { return "JobFailedEvent"; }
	};
}}

#endif
