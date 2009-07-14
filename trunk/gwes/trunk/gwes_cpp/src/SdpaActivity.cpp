/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
//gwes
#include <gwes/SdpaActivity.h>
#include <gwes/WorkflowHandler.h>
#include <gwes/GWES.h>
//gwdl
#include <gwdl/Token.h>
//std
#include <iostream>
#include <sstream>

using namespace std;

namespace gwes
{

SdpaActivity::SdpaActivity(WorkflowHandler* handler, gwdl::OperationCandidate* operationP) : Activity(handler, "SdpaActivity", operationP)
{
}

SdpaActivity::~SdpaActivity()
{
}

/**
 * Initiate this activity. Status should switch to INITIATED. Method should only work if the status was
 * UNDEFINED before. 
 */
void SdpaActivity::initiateActivity() throw (ActivityException,StateTransitionException) {
	cout << "gwes::SdpaActivity::initiateActivity(" << _id << ") ... " << endl;
	//check status
	if (_status != STATUS_UNDEFINED) {
		ostringstream oss;
		oss << "Invalid status " << getStatusAsString()
				<< " for initiating activity \"" << _id << "\"" << endl;
		throw StateTransitionException(oss.str());
	}
	
	// get and check SDPA handler
	_sdpaP = _wfhP->getGWES()->getSdpaHandler();
	if (_sdpaP == NULL) {
		throw ActivityException("Missing SDPA Handler! Use Spda2Gwes::registerHandler() to register SDPA.");
	}
	
	//// ToDo: implement!
	
	// set status
	setStatus(STATUS_INITIATED);
}

/**
 * Start this activity. Status should switch to RUNNING. 
 */
void SdpaActivity::startActivity() throw (ActivityException,StateTransitionException,gwdl::WorkflowFormatException) {
	cout << "gwes::SdpaActivity::startActivity(" << _id << ") ... " << endl;
	//check status
	if (_status != STATUS_INITIATED) {
		ostringstream oss;
		oss << "Invalid status " << getStatusAsString()
				<< " for starting activity \"" << _id << "\"" << endl;
		throw StateTransitionException(oss.str());
	}
	setStatus(STATUS_RUNNING);
	
	// submit activity to SDPA
	// SDPA will notify us about state transitions using the Spda2Gwes callback interface.
	// ToDo: use specific SDPA activity type instead?
	_sdpaP->submitActivity(*this);
}

/**
 * Suspend this activity. Status should switch to SUSPENDED. 
 */
void SdpaActivity::suspendActivity() throw (ActivityException,StateTransitionException) {
	cout << "gwes::SdpaActivity::suspendActivity(" << _id << ") ... " << endl;
	///ToDo: Implement!
	cerr << "gwes::SdpaActivity::suspendActivity() not yet implemented!" << endl;
}

/**
 * Resume this activity. Status should switch to RUNNING. 
 */
void SdpaActivity::resumeActivity() throw (ActivityException,StateTransitionException) {
	cout << "gwes::SdpaActivity::resumeActivity(" << _id << ") ... " << endl;
	//check status
	if (_status != STATUS_SUSPENDED) {
		ostringstream oss;
		oss << "Invalid status " << getStatusAsString()
				<< " for resuming activity \"" << _id << "\"" << endl;
		throw StateTransitionException(oss.str());
	}
	///ToDo: Implement!
	cerr << "gwes::SdpaActivity::resumeActivity() not yet implemented!" << endl;
}

/**
 * Abort this activity. Status should switch to TERMINATED.
 */
void SdpaActivity::abortActivity() throw (ActivityException,StateTransitionException) {
	cout << "gwes::SdpaActivity::abortActivity(" << _id << ") ... " << endl;
	//check status
	if (_status == STATUS_COMPLETED) {
		ostringstream oss;
		oss << "Invalid status " << getStatusAsString()
				<< " for aborting activity \"" << _id << "\"" << endl;
		throw StateTransitionException(oss.str());
	}

	_abort = true;
	_sdpaP->cancelActivity(_id);
	waitForStatusChangeToCompletedOrTerminated();
	setStatus(STATUS_TERMINATED);
}

/**
 * Restart this activity. Status should switch to INITIATED. 
 */
void SdpaActivity::restartActivity() throw (ActivityException,StateTransitionException) {
	cout << "gwes::SdpaActivity::restartActivity(" << _id << ") ... " << endl;
	///ToDo: Implement!
	cerr << "gwes::SdpaActivity::restartActivity() not yet implemented!" << endl;
}

} // end namespace gwes
