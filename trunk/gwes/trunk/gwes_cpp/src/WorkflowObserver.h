#ifndef WORKFLOWOBSERVER_H_
#define WORKFLOWOBSERVER_H_

// gwes
#include "Observer.h"

namespace gwes
{

/**
 * A simple Observer which prints out the events to stdout.
 * @version $Id$
 * @author Andreas Hoheisel &copy; 2008 <a href="http://www.first.fraunhofer.de/">Fraunhofer FIRST</a>  
 */ 
class WorkflowObserver : public gwes::Observer
{
public:
	WorkflowObserver();
	virtual ~WorkflowObserver();
	virtual void update(Event event);
};

}

#endif /*WORKFLOWOBSERVER_H_*/
