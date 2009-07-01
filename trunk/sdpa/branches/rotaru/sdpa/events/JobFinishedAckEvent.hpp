#ifndef SDPA_JobFinishedAckEvent_HPP
#define SDPA_JobFinishedAckEvent_HPP

#include <iostream>
#include <boost/statechart/event.hpp>
#include <sdpa/events/JobEvent.hpp>

namespace sc = boost::statechart;

namespace sdpa {
namespace events {
	class JobFinishedAckEvent : public sdpa::events::JobEvent, public sc::event<sdpa::events::JobFinishedAckEvent> {
	public:
		typedef sdpa::shared_ptr<JobFinishedAckEvent> Ptr;

		JobFinishedAckEvent(const address_t& from, const address_t& to, const sdpa::job_id_t& job_id = sdpa::job_id_t())
          :  sdpa::events::JobEvent( from, to, job_id ) {
			//std::cout << "Create event 'JobFinishedAckEvent'"<< std::endl;
		}

		virtual ~JobFinishedAckEvent() {
			// std::cout << "Delete event 'JobFinishedAckEvent'"<< std::endl;
		}

		std::string str() const { std::cout<<from()<<" - JobFinishedAckEvent -> "<<to()<<std::endl; }
	};
}}

#endif
