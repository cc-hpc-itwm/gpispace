/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#ifndef WORKFLOWOBSERVER_H_
#define WORKFLOWOBSERVER_H_

// gwes
#include <gwes/Observer.h>
//fhglog
#include <fhglog/fhglog.hpp>

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
	virtual void update(const Event& event);
private:
	fhg::log::LoggerApi _logger;
};

}

#endif /*WORKFLOWOBSERVER_H_*/
