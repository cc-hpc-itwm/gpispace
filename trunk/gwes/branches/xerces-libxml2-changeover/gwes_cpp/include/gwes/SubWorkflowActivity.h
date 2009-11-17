/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#ifndef SUBWORKFLOWACTIVITY_H_
#define SUBWORKFLOWACTIVITY_H_
// gwes
#include <gwes/Activity.h>
#include <gwes/GWES.h>
// gwdl
#include <gwdl/Workflow.h>
// std
#include <iostream>
#include <sstream>
#include <string>

namespace gwes
{

/**
 * The SubWorkflowActivity is a specific implementation of Activity which invokes sub workflows.
 * @version $Id$
 * @author Andreas Hoheisel &copy; 2008 <a href="http://www.first.fraunhofer.de/">Fraunhofer FIRST</a>  
 */ 
class SubWorkflowActivity : public Activity, public Observer
{
	
private:
	std::string _subworkflowFilename;
	std::string _subworkflowId;
	gwdl::Workflow* _subworkflowP;
	GWES* _gwesP;

public:
	explicit SubWorkflowActivity(WorkflowHandler* handler, TransitionOccurrence* toP, gwdl::OperationCandidate::ptr_t operationP);
	
	virtual ~SubWorkflowActivity();

	/**
	 * Initiate this activity. Status should switch to INITIATED. Method should only work if the status was
	 * UNDEFINED before. 
	 */
	virtual void initiateActivity() throw (ActivityException,StateTransitionException);

	/**
	 * Start this activity. Status should switch to RUNNING. 
	 */
	virtual void startActivity() throw (ActivityException,StateTransitionException,gwdl::WorkflowFormatException);

	/**
	 * Suspend this activity. Status should switch to SUSPENDED. 
	 */
	virtual void suspendActivity() throw (ActivityException,StateTransitionException);

	/**
	 * Resume this activity. Status should switch to RUNNING. 
	 */
	virtual void resumeActivity() throw (ActivityException,StateTransitionException);

	/**
	 * Abort this activity. Status should switch to TERMINATED.
	 */
	virtual void abortActivity() throw (ActivityException,StateTransitionException);

	/**
	 * Restart this activity. Status should switch to INITIATED. 
	 */
	virtual void restartActivity() throw (ActivityException,StateTransitionException);
	
	/**
	 * Overides gwes::Observer::update().
	 * This method is called by the source of the channels connected to this activity.
	 */
	virtual void update(const Event& event);
	
};

}

#endif /*SUBWORKFLOWACTIVITY_H_*/
