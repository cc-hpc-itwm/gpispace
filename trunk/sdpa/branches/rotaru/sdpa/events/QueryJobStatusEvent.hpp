#ifndef QUERYJOBSTATUSEVENT_HPP
#define QUERYJOBSTATUSEVENT_HPP

#include <iostream>
#include <boost/statechart/event.hpp>
#include <JobEvent.hpp>
namespace sc = boost::statechart;

struct QueryJobStatusEvent : JobEvent, sc::event<QueryJobStatusEvent>
{
	QueryJobStatusEvent(int JobID) : JobEvent(JobID) { std::cout << "Create event 'QueryJobStatusEvent'"<< std::endl; }
	virtual ~QueryJobStatusEvent() { std::cout << "Delete event 'QueryJobStatusEvent'"<< std::endl; }
};

#endif
