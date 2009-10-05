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

		RetrieveJobResultsEvent(const address_t& a_from, const address_t& a_to, const sdpa::job_id_t& a_job_id = sdpa::job_id_t())
          :  sdpa::events::JobEvent(a_from, a_to, a_job_id) {
		}

		virtual ~RetrieveJobResultsEvent() {
		}

		std::string str() const { return "RetrieveJobResultsEvent"; }
	};
}}


#endif
