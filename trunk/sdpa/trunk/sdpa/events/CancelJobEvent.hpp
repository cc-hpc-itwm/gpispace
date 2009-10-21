#ifndef SDPA_CANCELJOBEVENT_HPP
#define SDPA_CANCELJOBEVENT_HPP 1

#include <boost/statechart/event.hpp>
#include <sdpa/events/JobEvent.hpp>
namespace sc = boost::statechart;

namespace sdpa {
namespace events {
	class CancelJobEvent : public sdpa::events::JobEvent, public sc::event<sdpa::events::CancelJobEvent> {
	public:
		typedef sdpa::shared_ptr<CancelJobEvent> Ptr;

		CancelJobEvent(const address_t& a_from, const address_t& a_to, const sdpa::job_id_t& a_job_id = sdpa::job_id_t())
          :  sdpa::events::JobEvent( a_from, a_to, a_job_id ) {
		}

		virtual ~CancelJobEvent() { }

		std::string str() const { return "CancelJobEvent"; }
	};
}}

#endif
