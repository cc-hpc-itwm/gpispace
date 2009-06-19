#ifndef CONFIGREQUESTEVENT_HPP
#define CONFIGREQUESTEVENT_HPP

#include <iostream>
#include <boost/statechart/event.hpp>
namespace sc = boost::statechart;

#include <MgmtEvent.hpp>

struct ConfigRequestEvent : MgmtEvent, sc::event<ConfigRequestEvent>
{
	ConfigRequestEvent() : MgmtEvent() { std::cout << "Create event 'ConfigRequestEvent'"<< std::endl; }
	virtual ~ConfigRequestEvent() { std::cout << "Delete event 'ConfigRequestEvent'"<< std::endl; }
};

#endif
