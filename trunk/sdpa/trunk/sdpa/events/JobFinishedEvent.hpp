#ifndef SDPA_JOB_FINISHED_EVENT_HPP
#define SDPA_JOB_FINISHED_EVENT_HPP 1

#include <iostream>
#include <boost/statechart/event.hpp>
#include <sdpa/events/JobEvent.hpp>

namespace sc = boost::statechart;

namespace sdpa {
namespace events {
	class JobFinishedEvent : public sdpa::events::JobEvent, public sc::event<sdpa::events::JobFinishedEvent> {
	public:
		typedef sdpa::shared_ptr<JobFinishedEvent> Ptr;

		JobFinishedEvent(const address_t& from, const address_t& to, const sdpa::job_id_t& job_id = sdpa::job_id_t())
          :  sdpa::events::JobEvent( from, to, job_id ) {
			////std::cout << "Create event 'JobFinishedEvent'"<< std::endl;
		}

		virtual ~JobFinishedEvent() {
			// //std::cout << "Delete event 'JobFinishedEvent'"<< std::endl;
		}

		std::string str() const { return "JobFinishedEvent"; }
	};
}}

#endif
