/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
//gwes
#include <gwes/SubWorkflowActivity.h>
//gwdl
#include <gwdl/Token.h>
//std
#include <iostream>
#include <sstream>
#include <map>
#include <string>

using namespace std;
using namespace gwdl;

namespace gwes
{

SubWorkflowActivity::SubWorkflowActivity(WorkflowHandler* handler, gwdl::OperationCandidate* operationP) : Activity(handler, "SubWorkflowActivity", operationP)
{
	_gwesP = handler->getGWES();
	_subworkflowP = NULL;
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
	_subworkflowFilename = _operation->getOperationName();
	cout << "gwes::SubWorkflowActivity::initiateActivity(" << _id << ") trying to read file " << _subworkflowFilename << endl;

	// parse workflow file
	_subworkflowP = new gwdl::Workflow(_subworkflowFilename);
	setStatus(STATUS_INITIATED);
}

/**
 * Start this activity. Status should switch to RUNNING. 
 */
void SubWorkflowActivity::startActivity() throw (ActivityException,StateTransitionException,WorkflowFormatException) {
	cout << "gwes::SubWorkflowActivity::startActivity(" << _id << ") ... " << endl;
	//check status
	if (_status != STATUS_INITIATED) {
		ostringstream oss;
		oss << "Invalid status " << getStatusAsString()
		<< " for starting activity \"" << _id << "\"" << endl;
		throw StateTransitionException(oss.str());
	}
	setStatus(STATUS_RUNNING);

	// copy input tokens to places in sub workflow regarding the edge expressions of the parent workflow
	string edgeExpression = "";
	try {
		for (map<string,gwdl::Token*>::iterator it=_inputs.begin(); it !=_inputs.end(); ++it) {
			edgeExpression = it->first;
			// find corresponding place in subworkflow and add token with data on this place
			cout << "gwes::SubWorkflowActivity::startActivity(" << _id << ") copy token to place \"" << edgeExpression << "\" ..." << endl; 
			Place* placeP = _subworkflowP->getPlace(edgeExpression);
			placeP->addToken(it->second);
		}
	} catch (NoSuchWorkflowElement e) {
		setStatus(STATUS_TERMINATED);
		ostringstream message; 
		message << "Subworkflow does not contain place that matches edgeExpression \"" << edgeExpression << "\": " << e.message;
		throw WorkflowFormatException(message.str()); 
	}

	// initiate subworklfow
	_subworkflowId = _gwesP->initiate(*_subworkflowP,_wfhP->getUserId());

	// connect channel for callback 
	Channel* channelP = new Channel(this);
	_gwesP->connect(channelP, _subworkflowId);

	// start subworkflow asynchronously. 
	_gwesP->start(_subworkflowId);
	setStatus(STATUS_ACTIVE);

	// Completion of workflow will be notified using callback channel (refer to update()).
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

/**
 * Overides gwes::Observer::update().
 * This method is called by the workflow handler that invokes the sub workflow.
 */
void SubWorkflowActivity::update(const Event& event) {
	// logging
	cout << "gwes::SubWorkflowActivity[" << _id << "]::update(" << event._sourceId << "," << event._eventType << "," << event._message ;
	if (event._tokensP!=NULL) {
		cout << ",";
		map<string,gwdl::Token*>* dP = event._tokensP;
		for (map<string,gwdl::Token*>::iterator it=dP->begin(); it!=dP->end(); ++it) {
			cout << "[" << it->first << "]";
		}
	}
	cout << ")" << endl;

	// parse event
	if (event._eventType==Event::EVENT_WORKFLOW) {
		if (event._sourceId.compare(_subworkflowId) == 0) {
			if (event._message.compare("COMPLETED") == 0) {
				setStatus(Activity::STATUS_RUNNING);
				setStatus(Activity::STATUS_COMPLETED);
				// remove workflow from GWES and memory
				_gwesP->remove(_subworkflowId);
				// ToDo: copy output tokens
			} else if (event._message.compare("TERMINATED") == 0) {
				// ToDo: generate fault tokens
				setStatus(Activity::STATUS_TERMINATED);
				// remove workflow from GWES and memory
				_gwesP->remove(_subworkflowId);
			}
		}
	}
}



} // end namespace gwes
