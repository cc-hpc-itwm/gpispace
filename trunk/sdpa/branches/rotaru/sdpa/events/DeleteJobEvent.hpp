#ifndef SDPA_DELETE_JOB_EVENT_HPP
#define SDPA_DELETE_JOB_EVENT_HPP 1

#include <iostream>
#include <boost/statechart/event.hpp>
#include <sdpa/events/JobEvent.hpp>
namespace sc = boost::statechart;

namespace sdpa {
namespace events {
	class DeleteJobEvent : public sdpa::events::JobEvent, public sc::event<sdpa::events::DeleteJobEvent> {
	public:
		typedef sdpa::shared_ptr<DeleteJobEvent> Ptr;

		DeleteJobEvent(const address_t& from, const address_t& to, const sdpa::job_id_t& job_id = sdpa::job_id_t())
          :  sdpa::events::JobEvent( from, to, job_id ) {
			std::cout << "Create event 'DeleteJobEvent'"<< std::endl; }

		virtual ~DeleteJobEvent() {
			std::cout << "Delete event 'DeleteJobEvent'"<< std::endl; }

		std::string str() const { std::cout<<from()<<" - DeleteJobEvent -> "<<to()<<std::endl; }
	};
}}

#endif
