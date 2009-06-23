#ifndef SDPA_JOBFINISHEDEVENT_HPP
#define SDPA_JOBFINISHEDEVENT_HPP

#include <iostream>
#include <boost/statechart/event.hpp>
#include <JobEvent.hpp>

namespace sc = boost::statechart;

namespace sdpa {
namespace events {
	class JobFinishedEvent : public sdpa::events::JobEvent, public sc::event<sdpa::events::JobFinishedEvent> {
	public:
		typedef sdpa::shared_ptr<JobFinishedEvent> Ptr;

		JobFinishedEvent(const address_t& from, const address_t& to, const sdpa::Job::job_id_t& job_id = sdpa::Job::job_id_t())
          :  sdpa::events::JobEvent( from, to, job_id ) {
			std::cout << "Create event 'JobFinishedEvent'"<< std::endl; }

		virtual ~JobFinishedEvent() {
			std::cout << "Delete event 'JobFinishedEvent'"<< std::endl; }

		std::string str() const { std::cout<<from()<<" - JobFinishedEvent -> "<<to()<<std::endl; }
	};
}}

#endif
