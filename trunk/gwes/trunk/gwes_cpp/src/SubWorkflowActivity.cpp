/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
//std
#include <iostream>
#include <sstream>
#include <map>
#include <string>
//gwdl
#include "../../gworkflowdl_cpp/src/Data.h"
#include "../../gworkflowdl_cpp/src/Token.h"
//gwes
#include "SubWorkflowActivity.h"

using namespace std;

namespace gwes
{

SubWorkflowActivity::SubWorkflowActivity(WorkflowHandler* handler, gwdl::OperationCandidate* operation) : Activity(handler, "SubWorkflowActivity", operation)
{
	_gwesP = handler->getGWES();
}

SubWorkflowActivity::~SubWorkflowActivity()
{
}

/**
 * Initiate this activity. Status should switch to INITIATED. Method should only work if the status was
 * UNDEFINED before. 
 */
void SubWorkflowActivity::initiateActivity() throw (ActivityException,StateTransitionException) {
	cout << "gwes::SubWorkflowActivity::initiateActivity(" << _id << ") ... " << endl;
	//check status
	if (_status != STATUS_UNDEFINED) {
		ostringstream oss;
		oss << "Invalid status " << getStatusAsString()
				<< " for initiating activity \"" << _id << "\"" << endl;
		throw StateTransitionException(oss.str());
	}
	// set workflow file 
	_subworkflowdl = _operation->getOperationName();
	cout << "gwes::SubWorkflowActivity::startActivity(" << _id << ") ... trying to read file " << _subworkflowdl << endl;

	// parse workflow file
	_swfP = new gwdl::Workflow(_subworkflowdl);
	setStatus(STATUS_INITIATED);
}

/**
 * Start this activity. Status should switch to RUNNING. 
 */
void SubWorkflowActivity::startActivity() throw (ActivityException,StateTransitionException,gwdl::WorkflowFormatException) {
	cout << "gwes::SubWorkflowActivity::startActivity(" << _id << ") ... " << endl;
	//check status
	if (_status != STATUS_INITIATED) {
		ostringstream oss;
		oss << "Invalid status " << getStatusAsString()
				<< " for starting activity \"" << _id << "\"" << endl;
		throw StateTransitionException(oss.str());
	}
	setStatus(STATUS_RUNNING);
	
	// copy tokens

	//generate arguments from input tokens
//	for (map<string,gwdl::Data*>::iterator it=_inputs.begin(); it !=_inputs.end(); ++it) {
//		string edgeExpression = it->first;
//		// stdin is reserved edgeExpression which is not treated as normal command line parameter. 
//		if (edgeExpression != "stdin") {
//			string text = *(it->second)->getText();
//			command << " -" << edgeExpression << " " << convertUrlToLocalPath(text);
//		}
//	}
	
	// execute activity
	_subworkflowid = _gwesP->initiate(*_swfP,_wfhP->getUserId());
    setStatus(STATUS_ACTIVE);
    _gwesP->execute(_subworkflowid);
    setStatus(STATUS_RUNNING);
    
	// notify observers
	if (_observers.size()>0) {
		ostringstream oss;
		oss << "subworkflow completed with status " << _gwesP->getStatusAsString(*_swfP);
		notifyObservers(Event::EVENT_ACTIVITY_END,oss.str(),&_outputs);
	}

	setStatus( _gwesP->getStatus(*_swfP) );
}

/**
 * Suspend this activity. Status should switch to SUSPENDED. 
 */
void SubWorkflowActivity::suspendActivity() throw (ActivityException,StateTransitionException) {
	cout << "gwes::SubWorkflowActivity::suspendActivity(" << _id << ") ... " << endl;
	///ToDo: Implement!
	cerr << "gwes::SubWorkflowActivity::suspendActivity() not yet implemented!" << endl;
}

/**
 * Resume this activity. Status should switch to RUNNING. 
 */
void SubWorkflowActivity::resumeActivity() throw (ActivityException,StateTransitionException) {
	cout << "gwes::SubWorkflowActivity::resumeActivity(" << _id << ") ... " << endl;
	//check status
	if (_status != STATUS_SUSPENDED) {
		ostringstream oss;
		oss << "Invalid status " << getStatusAsString()
				<< " for resuming activity \"" << _id << "\"" << endl;
		throw StateTransitionException(oss.str());
	}
	///ToDo: Implement!
	cerr << "gwes::SubWorkflowActivity::resumeActivity() not yet implemented!" << endl;
}

/**
 * Abort this activity. Status should switch to TERMINATED.
 */
void SubWorkflowActivity::abortActivity() throw (ActivityException,StateTransitionException) {
	cout << "gwes::SubWorkflowActivity::abortActivity(" << _id << ") ... " << endl;
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
void SubWorkflowActivity::restartActivity() throw (ActivityException,StateTransitionException) {
	cout << "gwes::SubWorkflowActivity::restartActivity(" << _id << ") ... " << endl;
	///ToDo: Implement!
	cerr << "gwes::SubWorkflowActivity::restartActivity() not yet implemented!" << endl;
}

} // end namespace gwes
