/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#ifndef PRESTACKPROACTIVITY_H_
#define PRESTACKPROACTIVITY_H_
// gwdl
#include "../../gworkflowdl_cpp/src/OperationCandidate.h"
// gwes
#include "Activity.h"

namespace gwes {

/**
 * This class is used in order to invoke workflow activities which are related to Pre-StackPro.
 * @version $Id$
 * @author Andreas Hoheisel &copy; 2008 <a href="http://www.first.fraunhofer.de/">Fraunhofer FIRST</a>
 */  
class PreStackProActivity : public Activity {
	
public:
	
	/**
	 * Constructor.
	 */
	PreStackProActivity(WorkflowHandler* handler, gwdl::OperationCandidate* operation);
	
	/**
	 * Destructor.
	 */
	virtual ~PreStackProActivity();

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

#endif /*PRESTACKPROACTIVITY_H_*/
