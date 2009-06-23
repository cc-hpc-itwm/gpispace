#ifndef SDPA_LIFESIGNEVENT_HPP
#define SDPA_LIFESIGNEVENT_HPP

#include <iostream>
#include <boost/statechart/event.hpp>
#include <MgmtEvent.hpp>

namespace sc = boost::statechart;

namespace sdpa {
namespace events {
	class LifeSignEvent : public sdpa::events::MgmtEvent, public sc::event<sdpa::events::LifeSignEvent> {
	public:
		typedef sdpa::shared_ptr<LifeSignEvent> Ptr;

		LifeSignEvent(const address_t& from, const address_t& to ) :  sdpa::events::MgmtEvent(from, to) {
			std::cout << "Create event 'LifeSignEvent'"<< std::endl; }

		virtual ~LifeSignEvent() {
			std::cout << "Delete event 'LifeSignEvent'"<< std::endl; }

		std::string str() const { std::cout<<from()<<" - LifeSignEvent -> "<<to()<<std::endl; }
	};
}}

#endif
