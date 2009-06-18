#ifndef FSMEVENTS_H
#define FSMEVENTS_H

// FSM events
#include <iostream>

#include <boost/statechart/event.hpp>
namespace sc = boost::statechart;

struct MgmtEvent {
	MgmtEvent( ) { }
	virtual ~MgmtEvent() {}

private:
};

struct StartUpEvent : MgmtEvent, sc::event<StartUpEvent>
{
	StartUpEvent() : MgmtEvent() { std::cout << "Create event 'StartUpEvent'"<< std::endl; }
	virtual ~StartUpEvent() { std::cout << "Delete event 'StartUpEvent'"<< std::endl; }
};

struct InterruptEvent : MgmtEvent, sc::event<InterruptEvent>
{
	InterruptEvent() : MgmtEvent() { std::cout << "Create event 'InterruptEvent'"<< std::endl; }
	virtual ~InterruptEvent(){ std::cout << "Delete event 'InterruptEvent'"<< std::endl; }
};

struct LifeSignEvent : MgmtEvent, sc::event<LifeSignEvent>
{
	LifeSignEvent() : MgmtEvent() { std::cout << "Create event 'LifeSignEvent'"<< std::endl; }
	virtual ~LifeSignEvent() { std::cout << "Delete event 'LifeSignEvent'"<< std::endl; }
};

struct DeleteJobEvent : MgmtEvent, sc::event<DeleteJobEvent>
{
	DeleteJobEvent() : MgmtEvent() { std::cout << "Create event 'DeleteJobEvent'"<< std::endl; }
	virtual ~DeleteJobEvent() { std::cout << "Delete event 'DeleteJobEvent'"<< std::endl; }
};

struct RequestJobEvent : MgmtEvent, sc::event<RequestJobEvent>
{
	RequestJobEvent() : MgmtEvent() { std::cout << "Create event 'RequestJobEvent'"<< std::endl; }
	virtual ~RequestJobEvent() { std::cout << "Delete event 'RequestJobEvent'"<< std::endl; }
};

struct SubmitAckEvent : MgmtEvent, sc::event<SubmitAckEvent>
{
	SubmitAckEvent():  MgmtEvent() { std::cout << "Create event 'SubmitAckEvent'"<< std::endl; }
	virtual ~SubmitAckEvent() { std::cout << "Delete event 'SubmitAckEvent'"<< std::endl; }
};

struct ConfigRequestEvent : MgmtEvent, sc::event<ConfigRequestEvent>
{
	ConfigRequestEvent() : MgmtEvent() { std::cout << "Create event 'ConfigRequestEvent'"<< std::endl; }
	virtual ~ConfigRequestEvent() { std::cout << "Delete event 'ConfigRequestEvent'"<< std::endl; }
};


#endif
