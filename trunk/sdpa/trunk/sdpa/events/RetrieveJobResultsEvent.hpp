#ifndef SDPA_RETRIEVEJOBRESULTSEVENT_HPP
#define SDPA_RETRIEVEJOBRESULTSEVENT_HPP 1

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
		}

		virtual ~RetrieveJobResultsEvent() {
		}

		std::string str() const { return "RetrieveJobResultsEvent"; }
	};
}}


#endif
