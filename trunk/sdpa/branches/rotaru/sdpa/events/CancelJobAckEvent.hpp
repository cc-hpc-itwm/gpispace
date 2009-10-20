#ifndef SDPA_CANCELJOBACKEVENT_HPP
#define SDPA_CANCELJOBACKEVENT_HPP 1

#include <boost/statechart/event.hpp>
#include <sdpa/events/JobEvent.hpp>
namespace sc = boost::statechart;

namespace sdpa {
namespace events {
	class CancelJobAckEvent : public sdpa::events::JobEvent, public sc::event<sdpa::events::CancelJobAckEvent> {
	public:
		typedef sdpa::shared_ptr<CancelJobAckEvent> Ptr;

		CancelJobAckEvent(const address_t &a_from, const address_t &a_to, const sdpa::job_id_t& a_job_id = sdpa::job_id_t())
          :  sdpa::events::JobEvent( a_from, a_to, a_job_id ) {
		}

		virtual ~CancelJobAckEvent() { }

		std::string str() const { return "CancelJobAckEvent"; }
	};
}}

#endif
