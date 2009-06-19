#ifndef CANCELJOBEVENT_HPP
#define CANCELJOBEVENT_HPP

// FSM events
#include <iostream>
#include <boost/statechart/event.hpp>
#include <JobEvent.hpp>
namespace sc = boost::statechart;

struct CancelJobEvent : JobEvent, sc::event<CancelJobEvent>
{
	CancelJobEvent(int JobID) : JobEvent(JobID) { std::cout << "Create event 'CancelJobEvent'"<< std::endl; }
	virtual ~CancelJobEvent() { std::cout << "Delete event 'CancelJobEvent'"<< std::endl; }
};

#endif
