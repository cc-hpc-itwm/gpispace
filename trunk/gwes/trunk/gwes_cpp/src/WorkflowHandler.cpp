/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
//std
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <map>
//gwes
#include <gwes/WorkflowHandler.h>
#include <gwes/CommandLineActivity.h>
#include <gwes/PreStackProActivity.h>
#include <gwes/SubWorkflowActivity.h>

using namespace std;
using namespace gwdl;
using namespace gwes;

namespace gwes {

WorkflowHandler::WorkflowHandler(GWES* gwesP, Workflow* workflowP, const string& userId) {
	_status=STATUS_UNDEFINED;
	// set user id
	_userId = userId;
	// set id
	_id = workflowP->getID();
	if (workflowP->getID()==WORKFLOW_DEFAULT_ID)
		_id=generateID();
	workflowP->setID(_id);
	// set pointer to parent gwes
	_gwesP = gwesP;
	// set pointer to workflow
	_wfP = workflowP;
	// set switches
	_abort = false;
	_suspend = false;
	_sleepTime = SLEEP_TIME_MIN;
	// set status
	setStatus(STATUS_INITIATED);
}

WorkflowHandler::~WorkflowHandler() {
	// delete activities in activity table
	for (ActivityTable::iterator it=_activityTable.begin(); it!=_activityTable.end(); ++it) {
		delete it->second;
	}
	_activityTable.clear();
}

void WorkflowHandler::setStatus(int status) {
	if (status == _status) return;
//	int oldStatus = _status;
	_status = status;
	if (_status != STATUS_UNDEFINED) {
		_wfP->getProperties().put("status", getStatusAsString(_status));
	}
	
//	cout << "gwes::WorkflowHandler: status of workflow \"" << getID()
//			<< "\" changed from " << getStatusAsString(oldStatus) << " to "
//			<< getStatusAsString() << endl;
	
	// notify observers
	if (_channels.size()>0) {
		Event event(_id,Event::EVENT_WORKFLOW,getStatusAsString());
		for (unsigned int i = 0; i<_channels.size(); i++ ) {
			_channels[i]->_sourceP->update(event);
		}
	}
}

string WorkflowHandler::getNewActivityID() {
	static long activityCounter = 0;
	ostringstream oss;
	///ToDo: leading 000000x for activity counter
	oss << _id << "_" << activityCounter++;
	return oss.str();
}

/**
 * Start this workflow. Status should switch to RUNNING.
 */
void WorkflowHandler::startWorkflow() throw (StateTransitionException) {
	//check status
	if (_status != STATUS_INITIATED) {
		ostringstream oss;
		oss << "Invalid status " << getStatusAsString()
				<< " for starting workflow \"" << getID() << "\"" << endl;
		throw StateTransitionException(oss.str());
	}

	//start workflow in own thread.
	pthread_create(&_thread, NULL, startWorkflowAsThread, (void*)this);
}

/**
 * Execute this workflow. Status should switch to RUNNING.
 */
void WorkflowHandler::executeWorkflow() throw (StateTransitionException, WorkflowFormatException) {
	//check status
	if (getStatus() != WorkflowHandler::STATUS_INITIATED) {
		ostringstream oss;
		oss << "Invalid status " << getStatusAsString()
				<< " for starting workflow \"" << getID() << "\"" << endl;
		throw StateTransitionException(oss.str());
	}

	setStatus(WorkflowHandler::STATUS_RUNNING);

	int step=0;
	bool modification = true;

	//get enabled transitions
	vector<Transition*>& enabledTransitions = _wfP->getEnabledTransitions();
	if (enabledTransitions.size() <= 0) {
		cerr << "workflow \"" << getID()
				<< "\" does not contain any enabled transitions!" << endl;
	}

	//loop while workflow is not to abort and there exists enabled transitions or this workflow is still active
	while ((!_abort && enabledTransitions.size()> 0) || _status
			== WorkflowHandler::STATUS_ACTIVE) {
		if (modification) {
//			cout << "--- step " << step << " (" << getStatusAsString()
//					<< ") --- " << enabledTransitions.size()
//					<< " enabled transition(s)" << endl;
			modification = false;
		}

		///ToDo: search for undecided decisions. Updates "unresolvedDecisionTransitions" and "undecidedDecisions"

		//select transition, find enabled transition with true condition. Updates list "enabledTrueTransitions"
		Transition* selectedTransitionP = selectTransition(enabledTransitions);

		///ToDo: if there are only transitions with unresolved decisions, then suspend the workflow!

		//suspend if breakpoint has been reached
		if (!_abort && !_suspend && selectedTransitionP != NULL) {
			Properties transprops = selectedTransitionP->getProperties();
			if (transprops.contains("breakpoint")) {
				string breakstring = transprops.get("breakpoint");
				// If workflow is resumed, remove "REACHED and put "RELEASED" as value to breakpoint property
				if (breakstring=="REACHED") {
					cout << "released breakpoint at transition "
							<< selectedTransitionP->getID() << endl;
					transprops.put("breakpoint", "RELEASED");
				} else {
					cout << "reached breakpoint at transition "
							<< selectedTransitionP->getID() << endl;
					transprops.put("breakpoint", "REACHED");
					_suspend = true;
				}
			}
		}

		///ToDo: do processing on all enabled true transitions

		///ToDo: prorate blue transtions related to program executions

		//process selected transition.
		if (!_abort && !_suspend && selectedTransitionP != NULL) {
			int abstractionLevel = selectedTransitionP->getAbstractionLevel();
//			cout << "--- step " << step << " --- processing transition \""
//					<< selectedTransitionP->getID() << "\" (level "
//					<< abstractionLevel << ") ..." << endl;
			switch (abstractionLevel) {
			case (Operation::BLACK): // no operation
				if (processBlackTransition(selectedTransitionP, step))
					modification = true;
				break;
			case (Operation::GREEN): // concrete selected operation
				if (processGreenTransition(selectedTransitionP, step))
					modification = true;
				break;
			case (Operation::BLUE): // set of operation candidates
				if (processBlueTransition(selectedTransitionP, step))
					modification = true;
				break;
			case (Operation::YELLOW): // operation class
				if (processYellowTransition(selectedTransitionP, step))
					modification = true;
				break;
			case (Operation::RED): // unspecified operation
				if (processRedTransition(selectedTransitionP, step))
					modification = true;
				break;
			default:
				cerr << "This should not happen." << endl;
				_abort = true;
			}
		}

		// check the status of all activities and process results if COMPLETED or TERMINATED (or FAILED).
		if (checkActivityStatus(step)) modification = true;

		//wait here if workflow has been suspended. Workflow must switch from ACTIVE to RUNNING first!
		if (_suspend && _status == WorkflowHandler::STATUS_RUNNING) {
			setStatus(WorkflowHandler::STATUS_SUSPENDED);
			modification = true;
			waitForStatusChangeFrom(WorkflowHandler::STATUS_SUSPENDED);
		}

		///ToDo: suspend if there is a deadlock because of false conditions

		//if no modification occurred this step, wait some time
		if (!modification && !_abort) {
			usleep(_sleepTime);
			// set dynamic sleep time
			_sleepTime *= 2;
			if (_sleepTime> WorkflowHandler::SLEEP_TIME_MAX)
				_sleepTime = WorkflowHandler::SLEEP_TIME_MAX;
		}
		// reset sleep time if there was a modification
		else
			_sleepTime = WorkflowHandler::SLEEP_TIME_MIN;

		//update enabled transitions
		if (!_abort)
			enabledTransitions = _wfP->getEnabledTransitions();
		if (modification)
			step++;
	}

	//set exit status
	setStatus((_abort) ? WorkflowHandler::STATUS_TERMINATED : WorkflowHandler::STATUS_COMPLETED);
}

/**
 * Suspend this workflow. Status should switch to SUSPENDED.
 */
void WorkflowHandler::suspendWorkflow() throw (StateTransitionException) {
	if (_status == STATUS_RUNNING || _status == STATUS_ACTIVE) {
		_suspend = true;
		waitForStatusChangeTo(STATUS_SUSPENDED);
	} else {
		ostringstream oss;
		oss << "Invalid status " << getStatusAsString()
				<< " for suspending workflow \"" << getID() << "\"" << endl;
		throw StateTransitionException(oss.str());
	}
}

/**
 * Resume this workflow. Status should switch to RUNNING. 
 */
void WorkflowHandler::resumeWorkflow() throw (StateTransitionException) {
	if (_status == STATUS_SUSPENDED) {
		///ToDo: && isAlive() 
		_suspend = false;
		setStatus(STATUS_RUNNING);
	} else {
		ostringstream oss;
		oss << "Invalid status " << getStatusAsString()
				<< " for resuming workflow \"" << getID() << "\"" << endl;
		throw StateTransitionException(oss.str());
	}
}

/**
 * Abort this workflow. Status should switch to TERMINATED.
 */
void WorkflowHandler::abortWorkflow() throw (StateTransitionException) {
	// you cannot abort an COMPLETED, or TERMINATED workflow
	if (_status == STATUS_COMPLETED || _status == STATUS_TERMINATED) {
		ostringstream oss;
		oss << "Invalid status " << getStatusAsString()
				<< " for aborting workflow \"" << getID() << "\"" << endl;
		throw StateTransitionException(oss.str());
	}

	_abort = true;

	// Workflow is NOT running nor active
	if (_status == STATUS_INITIATED || _status == STATUS_SUSPENDED || _status
			== STATUS_UNDEFINED) {
		setStatus(STATUS_TERMINATED);
	}

	// Workflow is running or active
	else {
		waitForStatusChangeToCompletedOrTerminated();
	}
}

int WorkflowHandler::waitForStatusChangeFrom(int oldStatus) {
	while (_status == oldStatus) {
		usleep(_sleepTime);
	}
	return _status;
}

void WorkflowHandler::waitForStatusChangeTo(int newStatus) {
	while (_status != newStatus) {
		usleep(_sleepTime);
	}
}

void WorkflowHandler::waitForStatusChangeToCompletedOrTerminated() {
	while (_status != STATUS_COMPLETED && _status != STATUS_TERMINATED) {
		usleep(_sleepTime);
	}
}

string WorkflowHandler::generateID() {
	///ToDo: Implement with UUID.
	static long counter = 0;
	ostringstream oss;
	oss << _userId << "_" << counter++;
	return oss.str();
}

void WorkflowHandler::connect(Channel* channel) {
	// register channel
	_channels.push_back(channel);
	// attach observer also to all activities of this workflow. 
	for (map<string,Activity*>::iterator it=_activityTable.begin(); it!=_activityTable.end(); ++it) {
		Activity* activityP = it->second;
		activityP->attachObserver(channel->_sourceP);
	}
	// set channel destination observer
	channel->setDestination(this);
}

/**
 * Overides gwes::Observer::update().
 * This method is called by the source of the channels connected to this workflow handler.
 */
void WorkflowHandler::update(const Event& event) {
	// logging
	cout << "gwes::WorkflowHandler::update(" << event._sourceId << "," << event._eventType << "," << event._message ;
	if (event._dataP!=NULL) {
		cout << ",";
		map<string,gwdl::Data*>* dP = event._dataP;
		for (map<string,gwdl::Data*>::iterator it=dP->begin(); it!=dP->end(); ++it) {
			cout << "[" << it->first << "]";
		}
	}
	cout << ")" << endl;
	
	// forward events regarding activities to the corresponding activity.
	if (event._eventType==Event::EVENT_ACTIVITY_END) {
		Activity* activityP = _activityTable.get(event._sourceId);
		if (activityP!=NULL) {
			activityP->setStatus(Activity::STATUS_RUNNING);
			if (event._dataP!=NULL) {
				activityP->setOutputs(*event._dataP);
			}
			activityP->setStatus(Activity::STATUS_COMPLETED);
		}
	}
}

/**
 * Get the workflow which is handled by this WorkflowHandler.
 * @return A pointer to the workflow.
 */
gwdl::Workflow* WorkflowHandler::getWorkflow() {
	return _wfP;
}

///////////////////////////////////////////////
/////////// PRIVATE METHODS
///////////////////////////////////////////////

/**
 * ToDo: include condition processing.
 */
Transition* WorkflowHandler::selectTransition(
		vector<gwdl::Transition*>& enabledTransitions) {
	if (enabledTransitions.size()<= 0)
		return NULL;
	else
		return enabledTransitions.front();
}

bool WorkflowHandler::processRedTransition(Transition* tP, int step) {
	/// ToDo: implement!
	cerr
			<< "gwes::WorkflowHandler::processRedTransition() not yet implemented!"
			<< endl;
	return false;
}

bool WorkflowHandler::processYellowTransition(Transition* tP, int step) {
	/// ToDo: implement!
	cerr
			<< "gwes::WorkflowHandler::processYellowTransition() not yet implemented!"
			<< endl;
	return false;
}

bool WorkflowHandler::processBlueTransition(Transition* tP, int step) {
	/// ToDo: implement!
	cerr
			<< "gwes::WorkflowHandler::processBlueTransition() not yet implemented!"
			<< endl;
	return false;
}

bool WorkflowHandler::processGreenTransition(Transition* tP, int step) {
	cout << "gwes::WorkflowHandler::processGreenTransition(" << tP->getID()
			<< ") ..." << endl;
	bool modification = false;

	// select selected operation
	vector<OperationCandidate*> ocs = tP->getOperation()->getOperationClass()->getOperationCandidates();
	OperationCandidate* operationP = NULL;
	for (unsigned int i=0; i<ocs.size(); i++) {
		if (ocs[i]->isSelected()) {
			operationP = ocs[i];
			break;
		}
	}
	if (operationP == NULL) {
		cerr << "ERROR: No selected operation available!" << endl;
		return modification;
	}

	// construct corresponding activity
	Activity* activityP;
	string operationType = operationP->getType();
	if (operationType == "cli") {
		activityP = new CommandLineActivity(this, operationP);
	} else if (operationType == "psp") {
		activityP = new PreStackProActivity(this, operationP);
	} else if (operationType == "workflow") {
		activityP = new SubWorkflowActivity(this, operationP);
	} else {
		ostringstream oss;
		oss << "Transition \"" << tP->getID()
				<< "\" is related to an operation of type \"" << operationType
				<< "\" which is not supported." << endl;
		throw WorkflowFormatException(oss.str());
	}
	if (activityP == NULL) {
		cerr << "ERROR: Activity Pointer is NULL!" << endl;
		return modification;
	}
	
	// attach workflow observers to activity
	for (unsigned int i=0; i<_channels.size(); i++) {
		activityP->attachObserver(_channels[i]->_sourceP);
	}

	setStatus(STATUS_ACTIVE);
	_activityTable.put(activityP);
	_activityTransitionTable.insert(pair<string,gwdl::Transition*>(activityP->getID(),tP));

	// initiate activity
	activityP->initiateActivity();

	// set inputs and outputs
	activityP->setInputs(retrieveInputDataTokens(tP, activityP));
	activityP->setOutputs(generateOutputDataTokensTemplate(tP));

	// invoke activity
	activityP->startActivity();
	modification = true;

	return modification;
}

bool WorkflowHandler::processBlackTransition(Transition* tP, int step) {
	cout << "gwes::WorkflowHandler::processBlackTransition(" << tP->getID() << ") ..." << endl;
	
	Token* tokenP = NULL;

	///ToDo evaluate edgeExpressions with XPath expressions.

	// process input edges
	vector<Edge*> inEdges = tP->getInEdges();
	for (unsigned int i=0; i<inEdges.size(); i++) {
		unsigned int j = 0;
		tokenP = inEdges[i]->getPlace()->getTokens()[j];
		while (tokenP->isLocked()) {
			tokenP = inEdges[i]->getPlace()->getTokens()[++j];
		}

		// logging
//		if (tokenP->isData())
//			cout << "--- step " << step << " --- processing data token \n" << *tokenP << endl;
//		else
//			cout << "--- step " << step << " --- processing control token "	<< *tokenP << endl;

		// put data tokens with edge expression to context
		///ToDo: addToContext(tokenP, inEdge, context);

		// remove input token
		inEdges[i]->getPlace()->removeToken(tokenP);
	}

	// put copy of inputTokens to corresponding output places
	vector<Edge*> outEdges = tP->getOutEdges();

	for (unsigned int i=0; i<outEdges.size(); i++) {
		try {
			string edgeExpression = outEdges[i]->getExpression();
			///ToDo: put real token!!!
			//            if (edgeExpression.size() <= 0) {          // control token
			//            	Token d(true);
			//            } else if (dataOutputTokens != null) {     // data token
			//                token = (Token) dataOutputTokens.get(edgeExpression);
			//            }
			tokenP = new Token(true);
			outEdges[i]->getPlace()->addToken(tokenP);
		} catch (CapacityException e) {
			cerr << "exception: " << e.message << endl;
			_abort = true;
			_wfP->getProperties().put(createNewErrorID(), e.message);
		}
	}
	return true;
}

bool WorkflowHandler::checkActivityStatus(int step) throw (ActivityException) {
	bool modification = false;
	int tempworkflowstatus = STATUS_RUNNING;
	// loop through activities
	for (map<string,Activity*>::iterator it=_activityTable.begin(); it
			!=_activityTable.end(); ++it) {
		string activityID = it->first;
		Activity* activityP = it->second;
		int activityStatus = activityP->getStatus();
//		cout << "--- step " << step << " --- activity#" << activityID << "=" << activityP->getStatusAsString() << endl;

		// activity has completed or terminated
		if (activityStatus == Activity::STATUS_COMPLETED || activityStatus == Activity::STATUS_TERMINATED) {
			Transition* transitionP = _activityTransitionTable.find(activityID)->second;

			// set workflow warning property if there is any activity fault message
			string faultMessage = activityP->getFaultMessage();
			if (faultMessage.size()>0) {
				ostringstream oss;
				oss << "Fault in activity \"" << activityID
						<< "\" related to transition \""
						<< transitionP->getID() << "\": " << faultMessage
						<< endl;
				_wfP->getProperties().put(createNewWarnID(), oss.str());
			}

			// remove the corresponding token from each input place that has been locked by this transition
			vector<gwdl::Token*> lockedtokens = _activityTokenlistTable.find(activityID)->second;
			vector<gwdl::Edge*> inEdges = transitionP->getInEdges();
			for (unsigned int i=0; i<inEdges.size(); i++) {
				vector<Token*>& placetokens = inEdges[i]->getPlace()->getTokens();
				unsigned int il = 0;
				while (il < lockedtokens.size()) {
					unsigned int ip = 0;
					while (ip < placetokens.size()) {
						if (lockedtokens[il]->getID() == placetokens[ip]->getID()) {
							inEdges[i]->getPlace()->removeToken(lockedtokens[il]);
							il = lockedtokens.size();
							ip = placetokens.size();
						}
						ip++;
					}
					il++;
				}
			}

			//  put new token on each output place
			vector<gwdl::Edge*> outEdges = transitionP->getOutEdges();
			for (unsigned int i=0; i<outEdges.size(); i++) {
				try {
					gwdl::Token* tokenP = NULL;
					string edgeExpression = outEdges[i]->getExpression();

					// control place (without edge expression)
					if (edgeExpression.size() <= 0 ) {
						tokenP = new Token(activityStatus == Activity::STATUS_COMPLETED);
					}
					// data place (with edge expression)
					else {
						///ToDo: detect whether the output edge expression is already attached to an input value
						//tokenP = (Token) activity.getInputs().get(edgeExpression);

						// data place with edge expression that is equal to an output parameter of the operation
						if (tokenP == NULL) {
							map<string,gwdl::Data*> outputs = activityP->getOutputs();
							if (outputs.find(edgeExpression)!=outputs.end()) {
								tokenP = new Token(outputs.find(edgeExpression)->second);
							} else {
								///put SOAP Fault if there is no data for edge expression
								ostringstream fault;
								fault << "<data><soapenv:Fault xmlns:soapenv=\"http://www.w3.org/2003/05/soap-envelope\">";
								fault << "<soapenv:Code><soapenv:Value>env:Receive</soapenv:Value></soapenv:Code>";
								fault << "<soapenv:Reason><soapenv:Text xml:lang=\"en\">Activity '";
								fault << activityP->getID();
								fault << "': Not Output Available</soapenv:Text></soapenv:Reason>";
							    fault << "<soapenv:Detail>The activity '" << activityP->getID();
							    fault << "' has no output parameter related to the edge expression '" << edgeExpression;
							    fault << "'</soapenv:Detail>";
							    fault << "</soapenv:Fault></data>";
							    tokenP = new Token(new Data(fault.str()));
							}
						}
					}
					outEdges[i]->getPlace()->addToken(tokenP);
				} catch (CapacityException e) {
					cerr << "CapacityException:" << e.message << endl;;
					_abort = true;
					_wfP->getProperties().put(createNewErrorID(), e.message);
				} catch (WorkflowFormatException e) {
					cerr << "WorkflowFormatException:" << e.message << endl;;
					_abort = true;
					_wfP->getProperties().put(createNewErrorID(), e.message);
				}
			}
			modification = true;

			///ToDo: set transition status
			
			// cleanup
			_activityTransitionTable.erase(activityID);
			_activityTokenlistTable.erase(activityID);
			_activityTable.remove(activityID);

			///ToDo: fault management regarding the fault management policy property

		} else if (activityStatus == Activity::STATUS_FAILED) {
			///ToDo: implement fault management.
			cerr
					<< "gwes::WorkflowHandler: Fault management not yet implemented!"
					<< endl;
		} else {
			// activity still not completed or terminated
			tempworkflowstatus = STATUS_ACTIVE;
			// abort active activities if workflow is to abort
			if (_abort) {
				activityP->abortActivity();
			}
		}

		///ToDo: set transition status
	}
	
	///ToDo: annotate transitions with transition status

	// set workflow status
	setStatus(tempworkflowstatus);
	return modification;
}

map<string,gwdl::Data*> WorkflowHandler::retrieveInputDataTokens(gwdl::Transition* transitionP, Activity* activityP)
		throw (gwdl::WorkflowFormatException) {
//	cout << "WorkflowHandler::retrieveInputDataTokens() ..." << endl;
	// get read edges
	vector<gwdl::Edge*>& readEdges = transitionP->getReadEdges();
	// get input edges
	vector<gwdl::Edge*>& inEdges = transitionP->getInEdges();

	// create empty vector/maps
	vector<gwdl::Token*> tokenlist;
	map<string,gwdl::Data*> inputTokens;

	// process read edges
	if (readEdges.size()>0) {
		// loop through read edges (j)
		for (unsigned int j=0; j<readEdges.size(); j++) {
			// search next token which is not locked (i)
			unsigned int i = 0;
			gwdl::Token* tokenP = readEdges[j]->getPlace()->getTokens()[i];
			while (tokenP->isLocked()) {
				tokenP = readEdges[j]->getPlace()->getTokens()[++i];
			}

			// logging
//			if (tokenP->isData())
//				cout
//						<< "WorkflowHandler::retrieveInputDataTokens(): processing read data token "
//						<< *tokenP << endl;
//			else
//				cout
//						<< "WorkflowHandler::retrieveInputDataTokens(): processing read control token "
//						<< *tokenP << endl;

			// note: tokens on read places are not locked and not removed after processing!

			// put data token to input Hash Map
			string edgeExpression = readEdges[j]->getExpression();
			if (edgeExpression.size()> 0 && tokenP->isData()) {
				///ToDo: Replace all child elements names by edge expression, e.g. edge expression = value1 -->
				// <data><value1>15</value1></data>
				inputTokens.insert(pair<string, gwdl::Data*>(edgeExpression,
						tokenP->getData()));
			}
		}
	}

	// process input edges
	if (inEdges.size()>0) {
		// loop through input edges (j)
		for (unsigned int j=0; j<inEdges.size(); j++) {
			// search next token which is not locked (i)
			unsigned int i = 0;
			gwdl::Token* tokenP = inEdges[j]->getPlace()->getTokens()[i];
			while (tokenP->isLocked()) {
				tokenP = inEdges[j]->getPlace()->getTokens()[++i];
			}

			// logging
//			if (tokenP->isData())
//				cout
//						<< "WorkflowHandler::retrieveInputDataTokens(): processing input data token \n"
//						<< *tokenP << endl;
//			else
//				cout
//						<< "WorkflowHandler::retrieveInputDataTokens(): processing input control token "
//						<< *tokenP << endl;

			// lock input tokens and put them to tokenlist
			tokenP->lock(transitionP);
			tokenlist.push_back(tokenP);

			// put data token to input Hash Map
			string edgeExpression = inEdges[j]->getExpression();
			if (edgeExpression.size()> 0 && tokenP->isData()) {
				///ToDo: Replace all child elements names by edge expression, e.g. edge expression = value1 -->
				// <data><value1>15</value1></data>
				inputTokens.insert(pair<string, gwdl::Data*>(edgeExpression,
						tokenP->getData()));
			}

			// remove token from input place is done after completion or termination of the activity.
		}
	}

	_activityTokenlistTable.insert(pair<string, vector<gwdl::Token*> >(
			activityP->getID(), tokenlist));
	return inputTokens;
}

map<string,gwdl::Data*> WorkflowHandler::generateOutputDataTokensTemplate(
		gwdl::Transition* transitionP) {
	vector<gwdl::Edge*>& outEdges = transitionP->getOutEdges();
	map<string,gwdl::Data*> outputTokens;
	if (outEdges.size()> 0) {
		// loop through output edges
		for (unsigned int i=0; i<outEdges.size(); i++) {
			string edgeExpression = outEdges[i]->getExpression();
			// put the edgeExpression to the output HashMap
			if (edgeExpression.size()>0) {
				// the real result tokens on the output place are constructed asynch
				outputTokens.insert(pair<string, gwdl::Data*>(edgeExpression,
						NULL));
			}
		}
	}
	return outputTokens;
}

string WorkflowHandler::createNewErrorID() {
	static long errorCounter = 0;
	ostringstream oss;
	oss << "error." << errorCounter++;
	return oss.str();
}

string WorkflowHandler::createNewWarnID() {
	static long warnCounter = 0;
	ostringstream oss;
	oss << "warn." << warnCounter++;
	return oss.str();
}

} // end namespace gwes

/**
 * This method is used by pthread_create in order to start this workflow in an own thread.
 * @param workflowHandlerP Pointer to the WorkflowHandler.
 */
void *startWorkflowAsThread(void *workflowHandlerP) {
	WorkflowHandler* wfhP = (WorkflowHandler*) workflowHandlerP;
	wfhP->executeWorkflow();
	return 0;
}
