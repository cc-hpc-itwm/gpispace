/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#ifndef SDPAACTIVITY_H_
#define SDPAACTIVITY_H_
// gwes
#include <gwes/Activity.h>
#include <gwes/Gwes2Sdpa.h>
// gwdl
#include <gwdl/OperationCandidate.h>

namespace gwes
{

/**
 * The SdpaActivity is a specific implementation of Activity which invokes remote or local algorithms 
 * using SDPA. 
 * @version $Id$
 * @author Andreas Hoheisel &copy; 2008 <a href="http://www.first.fraunhofer.de/">Fraunhofer FIRST</a>  
 */ 
class SdpaActivity : public Activity
{
	
private:
	Gwes2Sdpa* _sdpaP;

public:
	explicit SdpaActivity(WorkflowHandler* handler, TransitionOccurrence* toP, gwdl::OperationCandidate::ptr_t operationP);
	
	virtual ~SdpaActivity();

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

#endif /*SDPAACTIVITY_H_*/
