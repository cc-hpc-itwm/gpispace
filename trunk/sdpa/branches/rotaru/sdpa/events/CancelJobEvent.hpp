#ifndef SDPA_CANCELJOBEVENT_HPP
#define SDPA_CANCELJOBEVENT_HPP 1

#include <iostream>
#include <boost/statechart/event.hpp>
#include <sdpa/events/JobEvent.hpp>
namespace sc = boost::statechart;

namespace sdpa {
namespace events {
	class CancelJobEvent : public sdpa::events::JobEvent, public sc::event<sdpa::events::CancelJobEvent> {
	public:
		typedef sdpa::shared_ptr<CancelJobEvent> Ptr;

		CancelJobEvent(const address_t& from, const address_t& to, const sdpa::job_id_t& job_id = sdpa::job_id_t())
          :  sdpa::events::JobEvent( from, to, job_id ) {
			//std::cout << "Create event 'CancelJobEvent'"<< std::endl;
		}

		virtual ~CancelJobEvent() {
			//std::cout << "Delete event 'CancelJobEvent'"<< std::endl;
		}

		std::string str() const { return "CancelJobEvent"; }
	};
}}

#endif
