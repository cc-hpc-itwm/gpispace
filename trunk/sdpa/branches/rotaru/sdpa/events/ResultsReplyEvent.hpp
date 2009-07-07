#ifndef SDPA_RESULTS_REPLY_EVENT_HPP
#define SDPA_RESULTS_REPLY_EVENT_HPP 1

#include <iostream>
#include <boost/statechart/event.hpp>
#include <sdpa/events/JobEvent.hpp>
namespace sc = boost::statechart;

namespace sdpa {
namespace events {
	class ResultsReplyEvent : public sdpa::events::JobEvent, public sc::event<sdpa::events::ResultsReplyEvent> {
	public:
		typedef sdpa::shared_ptr<ResultsReplyEvent> Ptr;

		ResultsReplyEvent(const address_t& from, const address_t& to, const sdpa::job_id_t& job_id = sdpa::job_id_t())
          :  sdpa::events::JobEvent(from, to, job_id) {
			//std::cout << "Create event 'ResultsReplyEvent'"<< std::endl;
		}

		virtual ~ResultsReplyEvent() {
			//std::cout << "Delete event 'ResultsReplyEvent'"<< std::endl;
		}

		std::string str() const { return "ResultsReplyEvent"; }
	};
}}


#endif
