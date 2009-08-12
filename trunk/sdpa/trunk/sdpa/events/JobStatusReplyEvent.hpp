#ifndef SDPA_ReplyJobStatusEvent_HPP
#define SDPA_ReplyJobStatusEvent_HPP

#include <iostream>
#include <boost/statechart/event.hpp>
#include <sdpa/events/JobEvent.hpp>

namespace sc = boost::statechart;

namespace sdpa {
namespace events {
	class JobStatusReplyEvent : public sdpa::events::JobEvent, public sc::event<sdpa::events::JobStatusReplyEvent> {
	public:
		typedef sdpa::shared_ptr<JobStatusReplyEvent> Ptr;

		JobStatusReplyEvent(const address_t& from, const address_t& to, const sdpa::job_id_t& job_id = sdpa::job_id_t())
          :  sdpa::events::JobEvent(from, to, job_id) {
			// std::cout << "Create event 'JobStatusReplyEvent'"<< std::endl;
		}

		virtual ~JobStatusReplyEvent() {
			// std::cout << "Delete event 'JobStatusReplyEvent'"<< std::endl;
		}

		std::string str() const { std::cout<<from()<<" - JobStatusReplyEvent -> "<<to()<<std::endl; }
	};
}}

#endif
