#ifndef SDPA_CANCELJOBACKEVENT_HPP
#define SDPA_CANCELJOBACKEVENT_HPP

#include <iostream>
#include <boost/statechart/event.hpp>
#include <sdpa/events/JobEvent.hpp>
namespace sc = boost::statechart;

namespace sdpa {
namespace events {
	class CancelJobAckEvent : public sdpa::events::JobEvent, public sc::event<sdpa::events::CancelJobAckEvent> {
	public:
		typedef sdpa::shared_ptr<CancelJobAckEvent> Ptr;

		CancelJobAckEvent(const address_t &from, const address_t &to, const sdpa::job_id_t& job_id = sdpa::job_id_t())
          :  sdpa::events::JobEvent( from, to, job_id ) {
			std::cout << "Create event 'CancelJobAckEvent'"<< std::endl; }

		virtual ~CancelJobAckEvent() {
			std::cout << "Delete event 'CancelJobAckEvent'"<< std::endl; }

		std::string str() const { std::cout<<from()<<" - CancelJobAckEvent -> "<<to()<<std::endl; }
	};
}}

#endif
