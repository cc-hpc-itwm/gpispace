#ifndef SUBMITACKEVENT_HPP
#define SUBMITACKEVENT_HPP

#include <iostream>
#include <boost/statechart/event.hpp>
#include <MgmtEvent.hpp>

namespace sc = boost::statechart;

struct SubmitAckEvent : MgmtEvent, sc::event<SubmitAckEvent>
{
	SubmitAckEvent():  MgmtEvent() { std::cout << "Create event 'SubmitAckEvent'"<< std::endl; }
	virtual ~SubmitAckEvent() { std::cout << "Delete event 'SubmitAckEvent'"<< std::endl; }
};

#endif
