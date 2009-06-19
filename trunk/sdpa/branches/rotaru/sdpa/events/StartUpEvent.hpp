#ifndef STARTUPEVENT_HPP
#define STARTUPEVENT_HPP

#include <iostream>
#include <boost/statechart/event.hpp>
#include <MgmtEvent.hpp>

namespace sc = boost::statechart;

struct StartUpEvent : MgmtEvent, sc::event<StartUpEvent>
{
	StartUpEvent() : MgmtEvent() { std::cout << "Create event 'StartUpEvent'"<< std::endl; }
	virtual ~StartUpEvent() { std::cout << "Delete event 'StartUpEvent'"<< std::endl; }
};

#endif
