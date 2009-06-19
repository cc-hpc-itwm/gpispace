#ifndef JOBSTATUSANSWEREVENT_HPP
#define JOBSTATUSANSWEREVENT_HPP

#include <iostream>
#include <boost/statechart/event.hpp>
#include <JobEvent.hpp>
namespace sc = boost::statechart;

struct JobStatusAnswerEvent : JobEvent, sc::event<JobStatusAnswerEvent>
{
	JobStatusAnswerEvent(int JobID) : JobEvent(JobID) { std::cout << "Create event 'JobStatusAnswerEvent'"<< std::endl; }
	virtual ~JobStatusAnswerEvent() { std::cout << "Delete event 'JobStatusAnswerEvent'"<< std::endl; }
};
#endif
