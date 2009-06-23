/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
//std
#include <iostream>
#include <sstream>
//gwes
#include <gwes/PreStackProActivity.h>

using namespace std;

namespace gwes
{

PreStackProActivity::PreStackProActivity(WorkflowHandler* handler, gwdl::OperationCandidate* operation) : Activity(handler, "PreStackProActivity", operation)
{
	///calls Activity constructor
}

PreStackProActivity::~PreStackProActivity()
{
}

/**
 * Initiate this activity. Status should switch to INITIATED. Method should only work if the status was
 * UNDEFINED before. 
 */
void PreStackProActivity::initiateActivity() throw (ActivityException,StateTransitionException) {
	cout << "gwes::PreStackProActivity::initiateActivity(" << _id << ") ... " << endl;
	//check status
	if (_status != STATUS_UNDEFINED) {
		ostringstream oss;
		oss << "Invalid status " << getStatusAsString() << " for initiating activity \"" << _id << "\"" << endl;
		throw StateTransitionException(oss.str());
	}
	// set status
	setStatus(STATUS_INITIATED);
}

/**
 * Start this activity. Status should switch to RUNNING. 
 */
void PreStackProActivity::startActivity() throw (ActivityException,StateTransitionException,gwdl::WorkflowFormatException) {
	cout << "gwes::PreStackProActivity::startActivity(" << _id << ") ... " << endl;
	//check status
	if (_status != STATUS_INITIATED) {
		ostringstream oss;
		oss << "Invalid status " << getStatusAsString()
				<< " for starting activity \"" << _id << "\"" << endl;
		throw StateTransitionException(oss.str());
	}
	setStatus(STATUS_RUNNING);

	// generate event which triggers the execution of the algorithm.
	ostringstream message;
	message << _operation->getOperationName() << "@" << _operation->getResourceName();
	setStatus(STATUS_ACTIVE);
	notifyObservers(Event::EVENT_ACTIVITY_START,message.str(),&_inputs);
    // the notification about completion or termination of activity is done asynchrounous by communication channel!
	// setStatus(STATUS_RUNNING);
    // (!_abort) ? setStatus(STATUS_COMPLETED) : setStatus(STATUS_TERMINATED);
}

/**
 * Suspend this activity. Status should switch to SUSPENDED. 
 */
void PreStackProActivity::suspendActivity() throw (ActivityException,StateTransitionException) {
	///ToDo: Implement!
	cerr << "gwes::PreStackProActivity::suspendActivity() not yet implemented!" << endl;
}

/**
 * Resume this activity. Status should switch to RUNNING. 
 */
void PreStackProActivity::resumeActivity() throw (ActivityException,StateTransitionException) {
	//check status
	if (_status != STATUS_SUSPENDED) {
		ostringstream oss;
		oss << "Invalid status " << getStatusAsString()
				<< " for resuming activity \"" << _id << "\"" << endl;
		throw StateTransitionException(oss.str());
	}
	///ToDo: Implement!
	cerr << "gwes::PreStackProActivity::resumeActivity() not yet implemented!" << endl;
}

/**
 * Abort this activity. Status should switch to TERMINATED.
 */
void PreStackProActivity::abortActivity() throw (ActivityException,StateTransitionException) {
	//check status
	if (_status == STATUS_COMPLETED) {
		ostringstream oss;
		oss << "Invalid status " << getStatusAsString()
				<< " for aborting activity \"" << _id << "\"" << endl;
		throw StateTransitionException(oss.str());
	}

	_abort = true;
	waitForStatusChangeToCompletedOrTerminated();
	setStatus(STATUS_TERMINATED);
}

/**
 * Restart this activity. Status should switch to INITIATED. 
 */
void PreStackProActivity::restartActivity() throw (ActivityException,StateTransitionException) {
	///ToDo: Implement!
	cerr << "gwes::PreStackProActivity::restartActivity() not yet implemented!" << endl;
}


}
