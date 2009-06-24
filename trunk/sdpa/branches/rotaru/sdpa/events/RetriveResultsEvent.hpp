#ifndef SDPA_RETRIEVERESULTSEVENT_HPP
#define SDPA_RETRIEVERESULTSEVENT_HPP

#include <iostream>
#include <boost/statechart/event.hpp>
#include <sdpa/events/JobEvent.hpp>
namespace sc = boost::statechart;

namespace sdpa {
namespace events {
	class RetriveResultsEvent : public sdpa::events::JobEvent, public sc::event<sdpa::events::RetriveResultsEvent> {
	public:
		typedef sdpa::shared_ptr<RetriveResultsEvent> Ptr;

		RetriveResultsEvent(const address_t& from, const address_t& to, const sdpa::daemon::Job::job_id_t& job_id = sdpa::daemon::Job::job_id_t())
          :  sdpa::events::JobEvent(from, to, job_id) {
			std::cout << "Create event 'RetriveResultsEvent'"<< std::endl; }

		virtual ~RetriveResultsEvent() {
			std::cout << "Delete event 'RetriveResultsEvent'"<< std::endl; }

		std::string str() const { std::cout<<from()<<" - RetriveResultsEvent -> "<<to()<<std::endl; }
	};
}}


#endif
