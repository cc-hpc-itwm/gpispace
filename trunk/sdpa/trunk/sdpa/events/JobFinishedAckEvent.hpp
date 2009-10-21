#ifndef SDPA_JOB_FINISHED_ACK_EVENT_HPP
#define SDPA_JOB_FINISHED_ACK_EVENT_HPP 1

#include <boost/statechart/event.hpp>
#include <sdpa/events/JobEvent.hpp>

namespace sc = boost::statechart;

namespace sdpa {
namespace events {
	class JobFinishedAckEvent : public sdpa::events::JobEvent, public sc::event<sdpa::events::JobFinishedAckEvent> {
	public:
		typedef sdpa::shared_ptr<JobFinishedAckEvent> Ptr;

		JobFinishedAckEvent(const address_t& a_from, const address_t& a_to, const sdpa::job_id_t& a_job_id = sdpa::job_id_t())
          :  sdpa::events::JobEvent( a_from, a_to, a_job_id ) {
		}

		virtual ~JobFinishedAckEvent() {
		}

		std::string str() const { return "JobFinishedAckEvent"; }
	};
}}

#endif
