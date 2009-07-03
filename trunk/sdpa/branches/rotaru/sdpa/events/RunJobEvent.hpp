#ifndef SDPA_RUNJOBEVENT_HPP
#define SDPA_RUNJOBEVENT_HPP

#include <iostream>
#include <boost/statechart/event.hpp>
#include <sdpa/events/JobEvent.hpp>

namespace sc = boost::statechart;

namespace sdpa {
namespace events {
	class RunJobEvent : public JobEvent, public sc::event<RunJobEvent> {
	public:
		typedef sdpa::shared_ptr<RunJobEvent> Ptr;

		RunJobEvent(const address_t& from, const address_t& to, const sdpa::job_id_t& job_id = sdpa::job_id_t())
          : JobEvent(from, to, job_id) {
			//std::cout << "Create event 'RunJobEvent'"<< std::endl;
		}

		virtual ~RunJobEvent() {
			//std::cout << "Delete event 'RunJobEvent'"<< std::endl;
		}

		std::string str() const { return "RunJobEvent"; }
	};
}}


#endif
