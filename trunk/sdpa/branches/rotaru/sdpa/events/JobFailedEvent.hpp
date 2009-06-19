#ifndef JOBFAILEDEVENT_HPP
#define JOBFAILEDEVENT_HPP

#include <iostream>
#include <boost/statechart/event.hpp>
#include <JobEvent.hpp>

namespace sc = boost::statechart;

struct JobFailedEvent : JobEvent, sc::event<JobFailedEvent>
{
	JobFailedEvent(int JobID) : JobEvent(JobID) { std::cout << "Create event 'JobFailedEvent'"<< std::endl; }
	virtual ~JobFailedEvent(){ std::cout << "Delete event 'JobFailedEvent'"<< std::endl; }
};

#endif
