#ifndef JOBFINISHEDEVENT_HPP
#define JOBFINISHEDEVENT_HPP

#include <iostream>
#include <boost/statechart/event.hpp>
#include <JobEvent.hpp>
namespace sc = boost::statechart;

struct JobFinishedEvent : JobEvent, sc::event<JobFinishedEvent>
{
	JobFinishedEvent(int JobID) : JobEvent(JobID) { std::cout << "Create event 'JobFinishedEvent'"<< std::endl; }
	virtual ~JobFinishedEvent() { std::cout << "Delete event 'JobFinishedEvent'"<< std::endl; }
};

#endif
