#ifndef DELETEJOBEVENT_HPP
#define DELETEJOBEVENT_HPP

#include <iostream>
#include <boost/statechart/event.hpp>
#include <MgmtEvent.hpp>

namespace sc = boost::statechart;

struct DeleteJobEvent : MgmtEvent, sc::event<DeleteJobEvent>
{
	DeleteJobEvent() : MgmtEvent() { std::cout << "Create event 'DeleteJobEvent'"<< std::endl; }
	virtual ~DeleteJobEvent() { std::cout << "Delete event 'DeleteJobEvent'"<< std::endl; }
};

#endif
