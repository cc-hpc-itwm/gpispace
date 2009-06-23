/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#ifndef OBSERVER_H_
#define OBSERVER_H_
// gwes
#include <gwes/Event.h>

namespace gwes
{

/**
 * The Observer is an abstract class that can be extended in order to implement a specific Observer.
 * @version $Id$
 * @author Andreas Hoheisel &copy; 2008 <a href="http://www.first.fraunhofer.de/">Fraunhofer FIRST</a>  
 */
class Observer
{
public:
	Observer(){}
	virtual ~Observer(){}
	
	/**
	 * This method is invoked each time the object changes, where this observer is attached to.
	 * This method is abstract so implement it in your specific observer implementation; see, e.g.,
	 * WorkflowObserver as example.
	 * @param event the event which encapsulates the notification.
	 */ 
	virtual void update(Event event)=0;
};

}

#endif /*OBSERVER_H_*/
