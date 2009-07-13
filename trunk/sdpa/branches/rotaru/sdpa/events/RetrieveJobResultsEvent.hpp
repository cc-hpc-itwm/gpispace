#ifndef SDPA_RETRIEVERESULTSEVENT_HPP
#define SDPA_RETRIEVERESULTSEVENT_HPP 1

#include <iostream>
#include <boost/statechart/event.hpp>
#include <sdpa/events/JobEvent.hpp>
namespace sc = boost::statechart;

namespace sdpa {
namespace events {
	class RetrieveJobResultsEvent : public sdpa::events::JobEvent, public sc::event<sdpa::events::RetrieveJobResultsEvent> {
	public:
		typedef sdpa::shared_ptr<RetrieveJobResultsEvent> Ptr;

		RetrieveJobResultsEvent(const address_t& from, const address_t& to, const sdpa::job_id_t& job_id = sdpa::job_id_t())
          :  sdpa::events::JobEvent(from, to, job_id) {
			//std::cout << "Create event 'RetrieveJobResultsEvent'"<< std::endl;
		}

		virtual ~RetrieveResultsEvent() {
			//std::cout << "Delete event 'RetrieveJobResultsEvent'"<< std::endl;
		}

		std::string str() const { return "RetrieveJobResultsEvent"; }
	};
}}


#endif
