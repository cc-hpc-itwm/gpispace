#ifndef SDPA_RETRIEVERESULTSEVENT_HPP
#define SDPA_RETRIEVERESULTSEVENT_HPP 1

#include <iostream>
#include <boost/statechart/event.hpp>
#include <sdpa/events/JobEvent.hpp>
namespace sc = boost::statechart;

namespace sdpa {
namespace events {
	class RetrieveResultsEvent : public sdpa::events::JobEvent, public sc::event<sdpa::events::RetrieveResultsEvent> {
	public:
		typedef sdpa::shared_ptr<RetrieveResultsEvent> Ptr;

		RetrieveResultsEvent(const address_t& from, const address_t& to, const sdpa::job_id_t& job_id = sdpa::job_id_t())
          :  sdpa::events::JobEvent(from, to, job_id) {
			//std::cout << "Create event 'RetrieveResultsEvent'"<< std::endl;
		}

		virtual ~RetrieveResultsEvent() {
			//std::cout << "Delete event 'RetrieveResultsEvent'"<< std::endl;
		}

		std::string str() const { return "RetrieveResultsEvent"; }
	};
}}


#endif
