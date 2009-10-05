/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
//gwes
#include <gwes/WorkflowHandler.h>
#include <gwes/SdpaActivity.h>
#include <gwes/CommandLineActivity.h>
#include <gwes/SubWorkflowActivity.h>
#include <gwes/Utils.h>
//gwdl
#include <gwdl/AbstractionLevel.h>
//std
#include <unistd.h>
#include <map>

using namespace std;
using namespace gwdl;
using namespace gwes;

namespace gwes {

WorkflowHandler::WorkflowHandler(GWES* gwesP, Workflow* workflowP, const string& userId) : _logger(fhg::log::getLogger("gwes")) {
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
	_simulation = false;
	if (workflowP->getProperties().contains("simulation") && (workflowP->getProperties().get("simulation")).compare("false") != 0) {
		_simulation = true;
		LOG_INFO(_logger, getID() << ": Simulation is ON");
		LOG_DEBUG(_logger, *workflowP);
	}
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

void WorkflowHandler::setStatus(WorkflowHandler::status_t status) {
	if (status == _status) return;
	status_t oldStatus = _status;
	_status = status;
	if (_status != STATUS_UNDEFINED) {
		_wfP->getProperties().put("status", getStatusAsString(_status));
	}
	
	LOG_DEBUG(_logger, "status of workflow \"" << getID() << "\" changed from " << getStatusAsString(oldStatus) << " to " << getStatusAsString());
	
	// notify observers
	Gwes2Sdpa* sdpaP = _gwesP->getSdpaHandler();
	if (sdpaP != NULL) {
		switch (_status) {
		case (STATUS_ACTIVE): 
			break;
		case (STATUS_COMPLETED):
			sdpaP->workflowFinished(_id);
			break;
		case (STATUS_TERMINATED):
			if (_abort) {
				sdpaP->workflowCanceled(_id);
			} else {
				sdpaP->workflowFailed(_id);
			}
			break;
		case (STATUS_FAILED):
			break;
		case (STATUS_INITIATED):
			break;
		case (STATUS_RUNNING):
			break;
		case (STATUS_SUSPENDED):
			break;
		case (STATUS_UNDEFINED):
			break;
		}
	}
	
	// ToDo: depricated - remove?
	if (_channels.size()>0) {
		Event event(_id,Event::EVENT_WORKFLOW,getStatusAsString());
		for (size_t i = 0; i<_channels.size(); i++ ) {
			_channels[i]->_sourceP->update(event);
		}
	}
}

string WorkflowHandler::getNewActivityID() const {
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
		oss << "Invalid status " << getStatusAsString()	<< " for starting workflow \"" << getID() << "\"";
		throw StateTransitionException(oss.str());
	}

	//start workflow in own thread.
	pthread_create(&_thread, NULL, startWorkflowAsThread, (void*)this);
}

/**
 * Execute this workflow. Status should switch to RUNNING.
 */
void WorkflowHandler::executeWorkflow() throw (StateTransitionException, WorkflowFormatException) {
	LOG_DEBUG(_logger, "executeWorkflow(" << getID() << ") ...");
	//check status
	if (getStatus() != WorkflowHandler::STATUS_INITIATED) {
		ostringstream oss;
		oss << "Invalid status " << getStatusAsString() << " for starting workflow \"" << getID() << "\"";
		throw StateTransitionException(oss.str());
	}

	setStatus(WorkflowHandler::STATUS_RUNNING);

	int step=0;
	bool modification = true;

	//get enabled transitions
	vector<Transition*>& enabledTransitions = _wfP->getEnabledTransitions();
	if (enabledTransitions.size() <= 0) {
		LOG_WARN(_logger, "workflow \"" << getID() << "\" does not contain any enabled transitions!");
	}

	//loop while workflow is not to abort and there exists enabled transitions or this workflow is still active
	while ((!_abort && enabledTransitions.size()> 0) 
			|| _status == WorkflowHandler::STATUS_ACTIVE) {
		if (modification) {
			LOG_INFO(_logger, "--- step " << step << " (" << getID() << ":" << getStatusAsString()
			<< ") --- " << enabledTransitions.size()
			<< " enabled transition(s)");
			modification = false;
		}

		try {

			///ToDo: search for undecided decisions. Updates "unresolvedDecisionTransitions" and "undecidedDecisions"

			//select transition, find enabled transition with true condition. 
			// ToDo: Updates list "enabledTrueTransitions" ?
			TransitionOccurrence* selectedToP = selectTransitionOccurrence(enabledTransitions, step);
			if (selectedToP != NULL) {
				selectedToP->simulation=_simulation;
				LOG_DEBUG(_logger, "Processing " << *selectedToP << " ...");
			}

			///ToDo: if there are only transitions with unresolved decisions, then suspend the workflow!

			//suspend if breakpoint has been reached
			if (!_abort && !_suspend && selectedToP != NULL) {
				Properties transprops = selectedToP->transitionP->getProperties();
				if (transprops.contains("breakpoint")) {
					string breakstring = transprops.get("breakpoint");
					// If workflow is resumed, remove "REACHED and put "RELEASED" as value to breakpoint property
					if (breakstring=="REACHED") {
						LOG_INFO(_logger, "released breakpoint at transition "
						<< selectedToP->transitionP->getID());
						transprops.put("breakpoint", "RELEASED");
					} else {
						LOG_INFO(_logger, "reached breakpoint at transition "
						<< selectedToP->transitionP->getID());
						transprops.put("breakpoint", "REACHED");
						_suspend = true;
					}
				}
			}

			///ToDo: prorate blue transtions related to program executions

			//process selected transition.
			if (!_abort && !_suspend && selectedToP != NULL) {
				int abstractionLevel = selectedToP->transitionP->getAbstractionLevel();
				LOG_INFO(_logger, "--- step " << step << " (" << getID() << ") --- processing transition occurrence \""
				<< selectedToP->getID() << "\" (level "
				<< abstractionLevel << ") ...");
				switch (abstractionLevel) {
				case (AbstractionLevel::BLACK): // no operation
					if (processBlackTransition(selectedToP, step))
						modification = true;
				break;
				case (AbstractionLevel::GREEN): // concrete selected operation
					if (processGreenTransition(selectedToP, step))
						modification = true;
				break;
				case (AbstractionLevel::BLUE): // set of operation candidates
					if (processBlueTransition(selectedToP, step))
						modification = true;
				break;
				case (AbstractionLevel::YELLOW): // operation class
					if (processYellowTransition(selectedToP, step))
						modification = true;
				break;
				case (AbstractionLevel::RED): // unspecified operation
					if (processRedTransition(selectedToP, step))
						modification = true;
				break;
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

			// suspend if there is a deadlock because of false conditions
			if (getStatus() == STATUS_RUNNING && !modification && selectedToP == NULL) {
				modification = true;
				LOG_INFO(_logger, "Workflow suspended because all conditions of all enabled transitions are false, or because of unresolved decision (conflict)!" );
				_wfP->getProperties().put(createNewWarnID(), "Workflow suspended because all conditions of all enabled transitions are false, or because of unresolved decision (conflict)!");
				_suspend = true;
			}

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
		} catch (ActivityException e) {
			ostringstream oss;
			oss << "gwes::WorkflowHandler::WorkflowHandler(" << getID() << "): ActivityException: ERROR :" << e.message;
			LOG_ERROR(_logger, oss.str());
			_abort = true;
			_wfP->getProperties().put(createNewErrorID(), oss.str());
		}  catch (WorkflowFormatException e) {
			ostringstream oss;
			oss << "gwes::WorkflowHandler::WorkflowHandler(" << getID() << "): WorkflowFormatException: ERROR :" << e.message;
			LOG_ERROR(_logger, oss.str());
			_abort = true;
			_wfP->getProperties().put(createNewErrorID(), oss.str());
		}
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

WorkflowHandler::status_t WorkflowHandler::waitForStatusChangeFrom(WorkflowHandler::status_t oldStatus) {
	while (_status == oldStatus) {
		usleep(_sleepTime);
	}
	return _status;
}

void WorkflowHandler::waitForStatusChangeTo(WorkflowHandler::status_t newStatus) {
	while (_status != newStatus) {
		usleep(_sleepTime);
	}
}

void WorkflowHandler::waitForStatusChangeToCompletedOrTerminated() {
	while (_status != STATUS_COMPLETED && _status != STATUS_TERMINATED) {
		usleep(_sleepTime);
	}
}

string WorkflowHandler::generateID() const {
	ostringstream oss;
	oss << _userId << "_" << Utils::generateUuid();
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
 * @deprecated[Use Spda2Gwes instead]
 */
void WorkflowHandler::update(const Event& event) {
	LOG_WARN(_logger, "update() is DEPRICATED!");
	// logging
	LOG_INFO(_logger, _id << ":update(" 
			<< event._sourceId 
			<< "," << event._eventType 
			<< "," << event._message 
			<< ")");
	
	// forward events regarding activities to the corresponding activity.
	if (event._eventType==Event::EVENT_ACTIVITY_END) {
		Activity* activityP = _activityTable.get(event._sourceId);
		if (activityP!=NULL) {
			activityP->setStatus(Activity::STATUS_RUNNING);
			if (event._tokensP!=NULL) {
				/// here was activity.setOutputs
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

/////////////////////////////////////////
// Delegation from Interface Spda2Gwes //
/////////////////////////////////////////

// transition from pending to running
void WorkflowHandler::activityDispatched(const activity_id_t &activityId) throw (NoSuchActivityException) {
	_activityTable.get(activityId)->activityDispatched();
}

// transition from running to failed     
void WorkflowHandler::activityFailed(const activity_id_t &activityId, const parameter_list_t &output)  throw (NoSuchActivityException) {
	_activityTable.get(activityId)->activityFailed(output);
}

// transition from running to finished
void WorkflowHandler::activityFinished(const activity_id_t &activityId, const parameter_list_t &output) throw (NoSuchActivityException) {
	_activityTable.get(activityId)->activityFinished(output);
}

// transition from * to canceled
void WorkflowHandler::activityCanceled(const activity_id_t &activityId) throw (NoSuchActivityException) {
	_activityTable.get(activityId)->activityCanceled();
}

///////////////////////////////////////////////
/////////// PRIVATE METHODS
///////////////////////////////////////////////

/**
 * Select next enabled transtition with true conditions and build transition occurrence object.
 * This method also locks all input tokens.
 * ToDo: include support of data.group
 * ToDo: include support for transition priority
 */
TransitionOccurrence* WorkflowHandler::selectTransitionOccurrence(vector<gwdl::Transition*>& enabledTransitions,int step) {
	if (enabledTransitions.size()<= 0)
		return NULL;
	else {
		for (vector<gwdl::Transition*>::iterator it = enabledTransitions.begin(); it != enabledTransitions.end(); ++it) {
			TransitionOccurrence* toP = new TransitionOccurrence(*it);
			if (toP->checkConditions(step)) {
				toP->lockTokens();
				return toP;
			}
		}
		return NULL;
	}
}

bool WorkflowHandler::processRedTransition(TransitionOccurrence* toP, int step) {
	/// ToDo: implement!
	LOG_WARN(_logger, "processRedTransition(" << toP->getID() <<") not yet implemented! (step=" << step <<")");
	return false;
}

bool WorkflowHandler::processYellowTransition(TransitionOccurrence* toP, int step) {
	/// ToDo: implement!
	LOG_WARN(_logger, "processYellowTransition(" << toP->getID() <<") not yet implemented! (step=" << step <<")");
	return false;
}

bool WorkflowHandler::processBlueTransition(TransitionOccurrence* toP, int step) {
	/// ToDo: implement!
	LOG_WARN(_logger, "processBlueTransition(" << toP->getID() <<") not yet implemented! (step=" << step <<")");
	return false;
}

bool WorkflowHandler::processGreenTransition(TransitionOccurrence* toP, int step) {
	LOG_DEBUG(_logger, "processGreenTransitionOccurrence(" << toP->getID() << ") --- step=" << step <<" ...");
	bool modification = false;

	// select selected operation
	vector<OperationCandidate*> ocs = toP->transitionP->getOperation()->getOperationClass()->getOperationCandidates();
	OperationCandidate* operationP = NULL;
	for (size_t i=0; i<ocs.size(); i++) {
		if (ocs[i]->isSelected()) {
			operationP = ocs[i];
			break;
		}
	}
	if (operationP == NULL) {
		LOG_WARN(_logger, "ERROR: No selected operation available!");
		return modification;
	}

	// construct corresponding activity
	Activity* activityP;
	string operationType = operationP->getType();
	if (operationType == "sdpa") {
		activityP = new SdpaActivity(this, toP, operationP);
	} else if (operationType == "cli") {
		activityP = new CommandLineActivity(this, toP, operationP);
	} else if (operationType == "workflow") {
		activityP = new SubWorkflowActivity(this, toP, operationP);
	} else {
		ostringstream oss;
		oss << "Transition occurrence \"" << toP->getID()
				<< "\" is related to an operation of type \"" << operationType
				<< "\" which is not supported." << endl;
		throw WorkflowFormatException(oss.str());
	}
	if (activityP == NULL) {
		LOG_ERROR(_logger, "ERROR: Activity Pointer is NULL!");
		return modification;
	}
	toP->activityP=activityP;
	
	// attach workflow observers to activity
	for (size_t i=0; i<_channels.size(); i++) {
		activityP->attachObserver(_channels[i]->_sourceP);
	}

	setStatus(STATUS_ACTIVE);
	_activityTable.put(activityP);

	// initiate activity
	activityP->initiateActivity();

	// invoke activity
	activityP->startActivity();
	modification = true;

	return modification;
}

bool WorkflowHandler::processBlackTransition(TransitionOccurrence* toP, int step) {
	LOG_DEBUG(_logger, "gwes::WorkflowHandler::processBlackTransitionOccurrence(" << toP->getID() << ") ...");

	//evaluate edgeExpressions with XPath expressions.
	toP->evaluateXPathEdgeExpressions(step);
	
	// remove input tokens
	toP->removeInputTokens();

	// put output tokens to output places.
	try {
		toP->writeWriteTokens();
		toP->putOutputTokens();
	} catch (CapacityException e) {
		LOG_ERROR(_logger, "exception: " << e.message);
		_abort = true;
		_wfP->getProperties().put(createNewErrorID(), e.message);
	}
	
    // store occurrence sequence
    if (_wfP->getProperties().contains("occurrence.sequence")) {
    	string occurrenceSequence = _wfP->getProperties().get("occurrence.sequence");
    	if (occurrenceSequence.length()>0) occurrenceSequence += " ";
    	occurrenceSequence += toP->transitionP->getID();
    	_wfP->getProperties().put("occurrence.sequence",occurrenceSequence);
    }
	
    // cleanup
    delete toP;
    
	return true;
}

bool WorkflowHandler::checkActivityStatus(int step) throw (ActivityException) {
	bool modification = false;
	status_t tempworkflowstatus = STATUS_RUNNING;
	// loop through activities
	for (map<string,Activity*>::iterator it=_activityTable.begin(); it
			!=_activityTable.end(); ++it) {
		string activityID = it->first;
		Activity* activityP = it->second;
		int activityStatus = activityP->getStatus();
		LOG_INFO(_logger, "--- step " << step << " --- activity#" << activityID << "=" << activityP->getStatusAsString());

		// activity has completed or terminated
		if (activityStatus == Activity::STATUS_COMPLETED || activityStatus == Activity::STATUS_TERMINATED) {
			TransitionOccurrence* toP = activityP->getTransitionOccurrence();

			// set workflow warning property if there is any activity fault message
			string faultMessage = activityP->getFaultMessage();
			if (faultMessage.size()>0) {
				ostringstream oss;
				oss << "Fault in activity \"" << activityID
						<< "\" related to transition occurrence \""
						<< toP->getID() << "\": " << faultMessage
						<< endl;
				_wfP->getProperties().put(createNewWarnID(), oss.str());
			}
			
			// process XPath edge expressions
			toP->evaluateXPathEdgeExpressions(step);

			// remove the corresponding token from each input place
			toP->removeInputTokens();

			try {
				// replace write tokens
				toP->writeWriteTokens();
				//  put new token on each output place
				toP->putOutputTokens();
			} catch (CapacityException e) {
				LOG_ERROR(_logger, "CapacityException:" << e.message);;
				_abort = true;
				_wfP->getProperties().put(createNewErrorID(), e.message);
			}
			
			modification = true;

		    // store occurrence sequence
		    if (_wfP->getProperties().contains("occurrence.sequence")) {
		    	string occurrenceSequence = _wfP->getProperties().get("occurrence.sequence");
		    	if (occurrenceSequence.length()>0) occurrenceSequence += " ";
		    	occurrenceSequence += toP->transitionP->getID();
		    	_wfP->getProperties().put("occurrence.sequence",occurrenceSequence);
		    }

		    ///ToDo: set transition status

		    // cleanup
		    delete toP;
		    _activityTable.remove(activityID);

		    ///ToDo: fault management regarding the fault management policy property
		    if (activityStatus == Activity::STATUS_TERMINATED) {
		    	_abort = true;
		    }

		} else if (activityStatus == Activity::STATUS_FAILED) {
			///ToDo: implement fault management.
			LOG_WARN(_logger, "Fault management not yet implemented!");
		} else {
			// activity still not completed or terminated
			tempworkflowstatus = STATUS_ACTIVE;
			// abort active activities if workflow is to abort
			if (_abort) {
				activityP->abortActivity();
			}
		}

	}
	
	///ToDo: annotate transitions with transition status

	// set workflow status
	setStatus(tempworkflowstatus);
	return modification;
}

string WorkflowHandler::createNewErrorID() const {
	static long errorCounter = 0;
	ostringstream oss;
	oss << "error." << errorCounter++;
	return oss.str();
}

string WorkflowHandler::createNewWarnID() const {
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
