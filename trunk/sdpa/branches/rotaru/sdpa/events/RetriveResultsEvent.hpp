#ifndef RETRIEVERESULTSEVENT_HPP
#define RETRIEVERESULTSEVENT_HPP

#include <iostream>
#include <boost/statechart/event.hpp>
#include <JobEvent.hpp>
namespace sc = boost::statechart;

struct RetriveResultsEvent : JobEvent, sc::event<RetriveResultsEvent>
{
	RetriveResultsEvent(int JobID) : JobEvent(JobID) { std::cout << "Create event 'RetriveResultsEvent'"<< std::endl; }
	virtual ~RetriveResultsEvent() { std::cout << "Delete event 'RetriveResultsEvent'"<< std::endl; }
};

#endif
