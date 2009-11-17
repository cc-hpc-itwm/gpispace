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

SdpaActivity::SdpaActivity(WorkflowHandler* handler, TransitionOccurrence* toP, gwdl::OperationCandidate::ptr_t operationP) : Activity(handler, toP, "SdpaActivity", operationP)
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
	LOG_INFO(_logger, "initiateActivity(" << _id << ") ... ");
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
	LOG_INFO(_logger, "startActivity(" << _id << ") ... ");
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
	LOG_INFO(_logger, "suspendActivity(" << _id << ") ... ");
	///ToDo: Implement!
	LOG_WARN(_logger, "suspendActivity() not yet implemented!");
}

/**
 * Resume this activity. Status should switch to RUNNING. 
 */
void SdpaActivity::resumeActivity() throw (ActivityException,StateTransitionException) {
	LOG_INFO(_logger, "resumeActivity(" << _id << ") ... ");
	//check status
	if (_status != STATUS_SUSPENDED) {
		ostringstream oss;
		oss << "Invalid status " << getStatusAsString()
				<< " for resuming activity \"" << _id << "\"" << endl;
		throw StateTransitionException(oss.str());
	}
	///ToDo: Implement!
	LOG_WARN(_logger, "resumeActivity() not yet implemented!");
}

/**
 * Abort this activity. Status should switch to TERMINATED.
 */
void SdpaActivity::abortActivity() throw (ActivityException,StateTransitionException) {
	LOG_INFO(_logger, "abortActivity(" << _id << ") ... ");
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
	LOG_INFO(_logger, "restartActivity(" << _id << ") ... ");
	///ToDo: Implement!
	LOG_WARN(_logger, "restartActivity() not yet implemented!");
}

} // end namespace gwes
