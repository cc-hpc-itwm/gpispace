#ifndef SDPA_JOB_FAILED_ACK_EVENT_HPP
#define SDPA_JOB_FAILED_ACK_EVENT_HPP 1

#include <boost/statechart/event.hpp>
#include <sdpa/events/JobEvent.hpp>

namespace sc = boost::statechart;

namespace sdpa {
namespace events {
	class JobFailedAckEvent : public sdpa::events::JobEvent, public sc::event<sdpa::events::JobFailedAckEvent> {
	public:
		typedef sdpa::shared_ptr<JobFailedAckEvent> Ptr;

		JobFailedAckEvent(const address_t& a_from, const address_t& a_to, const sdpa::job_id_t& a_job_id = sdpa::job_id_t(""))
          :  sdpa::events::JobEvent( a_from, a_to, a_job_id ) {
		}

		virtual ~JobFailedAckEvent() {
		}

		std::string str() const { return "JobFailedAckEvent"; }
	};
}}

#endif
