#ifndef SDPA_QUERYJOBSTATUSEVENT_HPP
#define SDPA_QUERYJOBSTATUSEVENT_HPP

#include <iostream>
#include <boost/statechart/event.hpp>
#include <JobEvent.hpp>

namespace sc = boost::statechart;

namespace sdpa {
namespace events {
	class QueryJobStatusEvent : public sdpa::events::JobEvent, public sc::event<sdpa::events::QueryJobStatusEvent> {
	public:
		typedef sdpa::shared_ptr<QueryJobStatusEvent> Ptr;

		QueryJobStatusEvent(const address_t& from, const address_t& to, const sdpa::Job::job_id_t& job_id = sdpa::Job::job_id_t())
          :  sdpa::events::JobEvent(from, to, job_id) {
			std::cout << "Create event 'QueryJobStatusEvent'"<< std::endl; }
		virtual ~QueryJobStatusEvent() {
			std::cout << "Delete event 'QueryJobStatusEvent'"<< std::endl; }

		std::string str() const { std::cout<<from()<<" - QueryJobStatusEvent -> "<<to()<<std::endl; }
	};
}}

#endif
