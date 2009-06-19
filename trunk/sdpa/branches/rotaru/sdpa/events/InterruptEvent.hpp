#ifndef INTERRUPTEVENT_HPP
#define INTERRUPTEVENT_HPP

#include <iostream>
#include <boost/statechart/event.hpp>
#include <MgmtEvent.hpp>
namespace sc = boost::statechart;

struct InterruptEvent : MgmtEvent, sc::event<InterruptEvent>
{
	InterruptEvent() : MgmtEvent() { std::cout << "Create event 'InterruptEvent'"<< std::endl; }
	virtual ~InterruptEvent(){ std::cout << "Delete event 'InterruptEvent'"<< std::endl; }
};

#endif
