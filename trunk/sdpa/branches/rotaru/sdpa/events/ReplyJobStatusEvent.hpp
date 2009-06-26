#ifndef SDPA_ReplyJobStatusEvent_HPP
#define SDPA_ReplyJobStatusEvent_HPP

#include <iostream>
#include <boost/statechart/event.hpp>
#include <sdpa/events/JobEvent.hpp>

namespace sc = boost::statechart;

namespace sdpa {
namespace events {
	class ReplyJobStatusEvent : public sdpa::events::JobEvent, public sc::event<sdpa::events::ReplyJobStatusEvent> {
	public:
		typedef sdpa::shared_ptr<ReplyJobStatusEvent> Ptr;

		ReplyJobStatusEvent(const address_t& from, const address_t& to, const sdpa::daemon::Job::job_id_t& job_id = sdpa::daemon::Job::job_id_t())
          :  sdpa::events::JobEvent(from, to, job_id) {
			// std::cout << "Create event 'ReplyJobStatusEvent'"<< std::endl;
		}

		virtual ~ReplyJobStatusEvent() {
			// std::cout << "Delete event 'ReplyJobStatusEvent'"<< std::endl;
		}

		std::string str() const { std::cout<<from()<<" - ReplyJobStatusEvent -> "<<to()<<std::endl; }
	};
}}

#endif
