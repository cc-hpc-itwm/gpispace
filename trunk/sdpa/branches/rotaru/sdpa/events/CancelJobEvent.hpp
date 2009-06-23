#ifndef SDPA_CANCELJOBEVENT_HPP
#define SDPA_CANCELJOBEVENT_HPP

#include <iostream>
#include <boost/statechart/event.hpp>
#include <JobEvent.hpp>
namespace sc = boost::statechart;

namespace sdpa {
namespace events {
	class CancelJobEvent : public sdpa::events::JobEvent, public sc::event<sdpa::events::CancelJobEvent> {
	public:
		typedef sdpa::shared_ptr<CancelJobEvent> Ptr;

		CancelJobEvent(const address_t& from, const address_t& to, const sdpa::Job::job_id_t& job_id = sdpa::Job::job_id_t())
          :  sdpa::events::JobEvent( from, to, job_id ) {
			std::cout << "Create event 'CancelJobEvent'"<< std::endl; }

		virtual ~CancelJobEvent() {
			std::cout << "Delete event 'CancelJobEvent'"<< std::endl; }

		std::string str() const { std::cout<<from()<<" - CancelJobEvent -> "<<to()<<std::endl; }
	};
}}

#endif
