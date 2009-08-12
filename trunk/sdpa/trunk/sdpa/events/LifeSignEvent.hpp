#ifndef SDPA_LIFESIGNEVENT_HPP
#define SDPA_LIFESIGNEVENT_HPP 1

#include <iostream>
#include <boost/statechart/event.hpp>
#include <sdpa/events/MgmtEvent.hpp>

namespace sc = boost::statechart;

namespace sdpa {
namespace events {
	class LifeSignEvent : public sdpa::events::MgmtEvent, public sc::event<sdpa::events::LifeSignEvent> {
	public:
		typedef sdpa::shared_ptr<LifeSignEvent> Ptr;

		LifeSignEvent(const address_t& from, const address_t& to ) :  sdpa::events::MgmtEvent(from, to) {
			//std::cout << "Create event 'LifeSignEvent'"<< std::endl;
		}

		virtual ~LifeSignEvent() {
			//std::cout << "Delete event 'LifeSignEvent'"<< std::endl;
		}

		const sdpa::job_id_t & last_job_id() const { return last_job_id_; }

		std::string str() const { return "LifeSignEvent"; }
	private:
		sdpa::job_id_t last_job_id_;
	};
}}

#endif
