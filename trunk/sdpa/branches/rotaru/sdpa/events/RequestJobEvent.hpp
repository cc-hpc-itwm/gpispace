#ifndef REQUESTJOBEVENT_HPP
#define REQUESTJOBEVENT_HPP

#include <iostream>
#include <boost/statechart/event.hpp>
#include <MgmtEvent.hpp>
namespace sc = boost::statechart;

struct RequestJobEvent : MgmtEvent, sc::event<RequestJobEvent>
{
	RequestJobEvent() : MgmtEvent() { std::cout << "Create event 'RequestJobEvent'"<< std::endl; }
	virtual ~RequestJobEvent() { std::cout << "Delete event 'RequestJobEvent'"<< std::endl; }
};


#endif
