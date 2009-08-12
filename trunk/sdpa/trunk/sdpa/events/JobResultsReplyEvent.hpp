#ifndef SDPA_RESULTS_REPLY_EVENT_HPP
#define SDPA_RESULTS_REPLY_EVENT_HPP 1

#include <iostream>
#include <boost/statechart/event.hpp>
#include <sdpa/events/JobEvent.hpp>
namespace sc = boost::statechart;

namespace sdpa {
namespace events {
	class JobResultsReplyEvent : public sdpa::events::JobEvent, public sc::event<sdpa::events::JobResultsReplyEvent> {
	public:
		typedef sdpa::shared_ptr<JobResultsReplyEvent> Ptr;

		JobResultsReplyEvent(const address_t& from, const address_t& to, const sdpa::job_id_t& job_id = sdpa::job_id_t())
          :  sdpa::events::JobEvent(from, to, job_id) {
			//std::cout << "Create event 'JobResultsReplyEvent'"<< std::endl;
		}

		virtual ~JobResultsReplyEvent() {
			//std::cout << "Delete event 'JobResultsReplyEvent'"<< std::endl;
		}

		std::string str() const { return "JobResultsReplyEvent"; }
	};
}}


#endif
