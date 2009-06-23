#ifndef SDPA_JOBSTATUSANSWEREVENT_HPP
#define SDPA_JOBSTATUSANSWEREVENT_HPP

#include <iostream>
#include <boost/statechart/event.hpp>
#include <JobEvent.hpp>

namespace sc = boost::statechart;

namespace sdpa {
namespace events {
	class JobStatusAnswerEvent : public sdpa::events::JobEvent, public sc::event<sdpa::events::JobStatusAnswerEvent> {
	public:
		typedef sdpa::shared_ptr<JobStatusAnswerEvent> Ptr;

		JobStatusAnswerEvent(const address_t& from, const address_t& to, const sdpa::Job::job_id_t& job_id = sdpa::Job::job_id_t())
          :  sdpa::events::JobEvent( from, to, job_id ) {
			std::cout << "Create event 'JobStatusAnswerEvent'"<< std::endl; }

		virtual ~JobStatusAnswerEvent() {
			std::cout << "Delete event 'JobStatusAnswerEvent'"<< std::endl; }

		std::string str() const { std::cout<<from()<<" - JobStatusAnswerEvent -> "<<to()<<std::endl; }
	};
}}

#endif
