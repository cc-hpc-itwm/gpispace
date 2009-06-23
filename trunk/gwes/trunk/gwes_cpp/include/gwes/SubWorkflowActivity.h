/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#ifndef SUBWORKFLOWACTIVITY_H_
#define SUBWORKFLOWACTIVITY_H_
// std
#include <iostream>
#include <sstream>
#include <string>
// gwdl
#include <gwdl/Workflow.h>
// gwes
#include <gwes/Activity.h>
#include <gwes/GWES.h>

namespace gwes
{

/**
 * The SubWorkflowActivity is a specific implementation of Activity which invokes sub workflows.
 * @version $Id$
 * @author Andreas Hoheisel &copy; 2008 <a href="http://www.first.fraunhofer.de/">Fraunhofer FIRST</a>  
 */ 
class SubWorkflowActivity : public Activity
{
	
private:
	std::string _subworkflowdl;
	std::string _subworkflowid;
	gwdl::Workflow* _swfP;
	GWES* _gwesP;

public:
	SubWorkflowActivity(WorkflowHandler* handler, gwdl::OperationCandidate* operation);
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
	
};

}

#endif /*SUBWORKFLOWACTIVITY_H_*/
