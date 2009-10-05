#ifndef SDPA_DELETE_JOB_EVENT_HPP
#define SDPA_DELETE_JOB_EVENT_HPP 1

#include <boost/statechart/event.hpp>
#include <sdpa/events/JobEvent.hpp>
namespace sc = boost::statechart;

namespace sdpa {
namespace events {
	class DeleteJobEvent : public sdpa::events::JobEvent, public sc::event<sdpa::events::DeleteJobEvent> {
	public:
		typedef sdpa::shared_ptr<DeleteJobEvent> Ptr;

		DeleteJobEvent(const address_t& a_from, const address_t& a_to, const sdpa::job_id_t& a_job_id = sdpa::job_id_t())
          :  sdpa::events::JobEvent( a_from, a_to, a_job_id ) {
		}

		virtual ~DeleteJobEvent() {
		}

		std::string str() const { return "DeleteJobEvent"; }
	};
}}

#endif
