#ifndef FSMEVENTS_H
#define FSMEVENTS_H

// FSM events
#include <iostream>

#include <boost/statechart/event.hpp>
namespace sc = boost::statechart;

struct JobEvent {
	JobEvent(int id = -1 ) { m_nJobID = id; }
	virtual ~JobEvent() {}
    void SetJobID( int id ) { m_nJobID = id; }
    int GetJobID() const { return m_nJobID; }
private:
	int m_nJobID;
};


struct RunJobEvent : JobEvent, sc::event<RunJobEvent>
{
	RunJobEvent(int JobID) : JobEvent(JobID) { std::cout << "Create event 'RunJobEvent'"<< std::endl; }
	virtual ~RunJobEvent() { std::cout << "Delete event 'RunJobEvent'"<< std::endl; }
};

struct JobFailedEvent : JobEvent, sc::event<JobFailedEvent>
{
	JobFailedEvent(int JobID) : JobEvent(JobID) { std::cout << "Create event 'JobFailedEvent'"<< std::endl; }
	virtual ~JobFailedEvent(){ std::cout << "Delete event 'JobFailedEvent'"<< std::endl; }
};

struct QueryJobStatusEvent : JobEvent, sc::event<QueryJobStatusEvent>
{
	QueryJobStatusEvent(int JobID) : JobEvent(JobID) { std::cout << "Create event 'QueryJobStatusEvent'"<< std::endl; }
	virtual ~QueryJobStatusEvent() { std::cout << "Delete event 'QueryJobStatusEvent'"<< std::endl; }
};

struct JobStatusAnswerEvent : JobEvent, sc::event<JobStatusAnswerEvent>
{
	JobStatusAnswerEvent(int JobID) : JobEvent(JobID) { std::cout << "Create event 'JobStatusAnswerEvent'"<< std::endl; }
	virtual ~JobStatusAnswerEvent() { std::cout << "Delete event 'JobStatusAnswerEvent'"<< std::endl; }
};

struct CancelJobEvent : JobEvent, sc::event<CancelJobEvent>
{
	CancelJobEvent(int JobID) : JobEvent(JobID) { std::cout << "Create event 'CancelJobEvent'"<< std::endl; }
	virtual ~CancelJobEvent() { std::cout << "Delete event 'CancelJobEvent'"<< std::endl; }
};

struct CancelJobAckEvent : JobEvent, sc::event<CancelJobAckEvent>
{
	CancelJobAckEvent(int JobID):  JobEvent(JobID) { std::cout << "Create event 'CancelJobAckEvent'"<< std::endl; }
	virtual ~CancelJobAckEvent() { std::cout << "Delete event 'CancelJobAckEvent'"<< std::endl; }
};

struct JobFinishedEvent : JobEvent, sc::event<JobFinishedEvent>
{
	JobFinishedEvent(int JobID) : JobEvent(JobID) { std::cout << "Create event 'JobFinishedEvent'"<< std::endl; }
	virtual ~JobFinishedEvent() { std::cout << "Delete event 'JobFinishedEvent'"<< std::endl; }
};

struct RetriveResultsEvent : JobEvent, sc::event<RetriveResultsEvent>
{
	RetriveResultsEvent(int JobID) : JobEvent(JobID) { std::cout << "Create event 'RetriveResultsEvent'"<< std::endl; }
	virtual ~RetriveResultsEvent() { std::cout << "Delete event 'RetriveResultsEvent'"<< std::endl; }
};

#endif
