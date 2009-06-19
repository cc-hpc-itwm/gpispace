#ifndef CANCELJOBACKEVENT_HPP
#define CANCELJOBACKEVENT_HPP

#include <iostream>
#include <boost/statechart/event.hpp>
#include <JobEvent.hpp>
namespace sc = boost::statechart;

struct CancelJobAckEvent : JobEvent, sc::event<CancelJobAckEvent>
{
	CancelJobAckEvent(int JobID):  JobEvent(JobID) { std::cout << "Create event 'CancelJobAckEvent'"<< std::endl; }
	virtual ~CancelJobAckEvent() { std::cout << "Delete event 'CancelJobAckEvent'"<< std::endl; }
};

#endif
