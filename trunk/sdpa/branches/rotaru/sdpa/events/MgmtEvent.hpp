#ifndef MGMTEVENT_HPP
#define MGMTEVENT_HPP

#include <iostream>
#include <boost/statechart/event.hpp>
namespace sc = boost::statechart;

struct MgmtEvent {
	MgmtEvent( ) { }
	virtual ~MgmtEvent() {}

private:
};

#endif
