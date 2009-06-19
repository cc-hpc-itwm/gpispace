#ifndef LIFESIGNEVENT_HPP
#define LIFESIGNEVENT_HPP

#include <iostream>
#include <boost/statechart/event.hpp>
#include <LifeSignEvent.hpp>

namespace sc = boost::statechart;

struct LifeSignEvent : MgmtEvent, sc::event<LifeSignEvent>
{
	LifeSignEvent() : MgmtEvent() { std::cout << "Create event 'LifeSignEvent'"<< std::endl; }
	virtual ~LifeSignEvent() { std::cout << "Delete event 'LifeSignEvent'"<< std::endl; }
};


#endif
