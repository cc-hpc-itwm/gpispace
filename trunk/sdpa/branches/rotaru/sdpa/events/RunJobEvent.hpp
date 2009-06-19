#ifndef RUNJOBEVENT_HPP
#define RUNJOBEVENT_HPP

#include <iostream>
#include <boost/statechart/event.hpp>
#include <JobEvent.hpp>

namespace sc = boost::statechart;

struct RunJobEvent : JobEvent, sc::event<RunJobEvent>
{
	RunJobEvent(int JobID) : JobEvent(JobID) { std::cout << "Create event 'RunJobEvent'"<< std::endl; }
	virtual ~RunJobEvent() { std::cout << "Delete event 'RunJobEvent'"<< std::endl; }
};

#endif
