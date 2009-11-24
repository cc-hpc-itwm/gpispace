/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
//gwes
#include <gwes/Activity.h>
#include <gwes/WorkflowHandler.h>
#include <gwes/Utils.h>
// we need the real GWES now -> workflow_directory function
#include <gwes/GWES.h>

//gwdl
#include <gwdl/WorkflowFormatException.h>
#include <gwdl/NoSuchWorkflowElement.h>
#include <gwdl/Libxml2Builder.h>
//std
#include <iostream>
#include <sstream>
#include <unistd.h>

#include <stdexcept> // runtime_error

using namespace std;

namespace gwes {

Activity::Activity(WorkflowHandler* handler, TransitionOccurrence* toP, const string& activityImpl, gwdl::OperationCandidate::ptr_t operationP) : _logger(fhg::log::getLogger("gwes")) {
	_status=STATUS_UNDEFINED;
	_wfhP = handler;
	_toP = toP;
	_activityImpl = activityImpl;
	_id = handler->getNewActivityID();
	_operation = operationP;
	_abort = false;
	_suspend = false;
	LOG_DEBUG(_logger, "Activity[" << _id << "]");
}

Activity::~Activity() {
	LOG_DEBUG(_logger, "~Activity[" << _id << "]");
}

/////////////////////////////////////////
// IActivity interface methods 
/////////////////////////////////////////

/** 
 * Generates workflow object that corresponds to this activity.
 */
gwdl::Workflow::ptr_t Activity::transform2Workflow() const throw(std::exception) {
	LOG_DEBUG(_logger, "transforming activity " << _id << " to workflow object...");

	gwdl::Workflow::ptr_t subworkflowP;
	
	string type = _operation->getType();
	if (type == "workflow" || type == "sdpa/workflow") {  // activity of type workflow
        const std::string original_path(_operation->getOperationName());
        typedef std::list<std::string> path_list_t;
        path_list_t search_path;

        search_path.push_back(original_path);
        search_path.push_back(_wfhP->getGWES()->workflow_directory() + "/" + original_path);
        search_path.push_back(Utils::expandEnv(original_path));

        for (path_list_t::const_iterator path(search_path.begin()); path != search_path.end(); ++path) {
        	DLLOG(DEBUG, _logger, "trying to load sub-workflow from file: " << *path);

        	// parse workflow file
        	try {
        		gwdl::Libxml2Builder builder;
        		subworkflowP = builder.deserializeWorkflowFromFile(*path);
        		if (subworkflowP == NULL) {
        			throw gwdl::WorkflowFormatException("gwd::Workflow("+*path+") returned NULL pointer");
        		}
        		break; // take the first valid sub-workflow for now...
        	} catch (const gwdl::WorkflowFormatException &e) {
        		ostringstream message; 
        		message << "Unable to build subworkflow activity: " << e.what();
        		LOG_WARN(_logger, message.str());
        	} catch (const std::exception &ex) {
        		LOG_WARN(_logger, "Unable to build subworkflow activity: " << ex.what());
        	} catch (...) {
        		LOG_WARN(_logger, "Unable to build subworkflow activity (unknown reason)");
        	}
        }
        if (subworkflowP == NULL) {
        	LOG_FATAL(_logger, "Could not build subworkflow activity, giving up.");
        	throw ActivityException("Could not build subworkflow activity (check the logs for more information)"); 
        }

		// copy read/input/write tokens to places in sub workflow regarding the edge expressions of the parent workflow
		string edgeExpression;
		try {
			gwdl::Place::ptr_t placeP;
			gwdl::Token::ptr_t tokenCloneP;
			for (parameter_list_t::iterator it=_toP->tokens.begin(); it!=_toP->tokens.end(); ++it) {
				switch (it->scope) {
				case (TokenParameter::SCOPE_READ):
				case (TokenParameter::SCOPE_INPUT):
				case (TokenParameter::SCOPE_WRITE):
					edgeExpression = it->edgeP->getExpression();
					LOG_DEBUG(_logger, _id << ": copy token " << it->tokenP->getID() << " from activity to sub workflow ..."); 
					placeP = subworkflowP->getPlace(edgeExpression);
					tokenCloneP = it->tokenP->deepCopy();
					placeP->addToken(tokenCloneP);
					break;
				case (TokenParameter::SCOPE_OUTPUT):	
					continue;
				}
			}
		} catch (const gwdl::NoSuchWorkflowElement &e) {
			ostringstream message; 
			message << "Subworkflow does not contain place that matches edgeExpression \"" << edgeExpression << "\": " << e.what();
			LOG_ERROR(_logger, message.str());
			throw ActivityException(message.str()); 
		}

	} else {                                     // atomic activity
		LOG_DEBUG(_logger, "detected atomic activity ...");
		// create empty worklfow
		subworkflowP = gwdl::Workflow::ptr_t(new gwdl::Workflow(_id));
		
		// description
		ostringstream message; 
		message << "workflow automatically generated from atomic activity " << _id;
		subworkflowP->setDescription(message.str());

		// operation candidate
		gwdl::OperationCandidate::ptr_t opcP = gwdl::OperationCandidate::ptr_t(new gwdl::OperationCandidate());
		opcP->setType(_operation->getType());
		opcP->setOperationName(_operation->getOperationName());
		opcP->setResourceName(_operation->getResourceName());
		opcP->setSelected(true);

		// operation class
		gwdl::OperationClass::ptr_t opc = gwdl::OperationClass::ptr_t(new gwdl::OperationClass());
		opc->setName(_operation->getOperationName());
		opc->addOperationCandidate(opcP);
		
		// operation
		gwdl::Operation::ptr_t op = gwdl::Operation::ptr_t(new gwdl::Operation());
		op->setOperationClass(opc);

		// transition
		gwdl::Transition::ptr_t tP = gwdl::Transition::ptr_t(new gwdl::Transition(_operation->getOperationName()));
		tP->setOperation(op);	
		subworkflowP->addTransition(tP);

		// places edges and tokens
		string edgeExpression;
		gwdl::Place::ptr_t placeP;
		gwdl::Edge::ptr_t edgeP;
		for (parameter_list_t::iterator it=_toP->tokens.begin(); it!=_toP->tokens.end(); ++it) {
			edgeExpression = it->edgeP->getExpression();
			placeP = gwdl::Place::ptr_t(new gwdl::Place(it->edgeP->getPlaceID()));
			if (it->tokenP) {
				gwdl::Token::ptr_t tokenCloneP = it->tokenP->deepCopy();
				placeP->addToken(tokenCloneP);
			}

			subworkflowP->addPlace(placeP);

			switch (it->scope) {
			case (TokenParameter::SCOPE_READ):
				edgeP = gwdl::Edge::ptr_t(new gwdl::Edge(gwdl::Edge::SCOPE_READ, placeP,edgeExpression));
				break;
			case (TokenParameter::SCOPE_INPUT):
				edgeP = gwdl::Edge::ptr_t(new gwdl::Edge(gwdl::Edge::SCOPE_INPUT, placeP,edgeExpression));
				break;
			case (TokenParameter::SCOPE_WRITE):
				edgeP = gwdl::Edge::ptr_t(new gwdl::Edge(gwdl::Edge::SCOPE_WRITE, placeP,edgeExpression));
				break;
			case (TokenParameter::SCOPE_OUTPUT):	
				edgeP = gwdl::Edge::ptr_t(new gwdl::Edge(gwdl::Edge::SCOPE_OUTPUT, placeP,edgeExpression));
				break;
			}
			tP->addEdge(edgeP);
		}
	}

	// delegate simulation flag to sub workflow.
	if (_toP->simulation) {
		subworkflowP->putProperty("simulation","true");
	}

	LOG_DEBUG(_logger,"generated workflow from activity:");
	LOG_DEBUG(_logger, *subworkflowP);
	return subworkflowP;
}

const gwdl::Workflow::workflow_id_t &Activity::getOwnerWorkflowID() const {
	return _wfhP->getID();
}

/////////////////////////////////////////
// GWES internal methods 
/////////////////////////////////////////

void Activity::setStatus(Activity::status_t status) throw (StateTransitionException) {
	if (status == _status) return; 

    // StateMachine:
    // UNDEFINED -> any
    // RUNNING -> TERMINATED | 
    const status_t from_state(_status);
    const status_t to_state(status);

	LOG_DEBUG(_logger, "attempting to change status of activity " << _id << " from " << getStatusAsString(from_state) << " to " << getStatusAsString(to_state));

    if (from_state == STATUS_UNDEFINED)
    {
    }
    else if (from_state == STATUS_RUNNING)
    {
      if (to_state == STATUS_UNDEFINED) throw StateTransitionException("RUNNING -> UNDEFINED");
    }
    else if (from_state == STATUS_INITIATED)
    {
    }
    else if (from_state == STATUS_COMPLETED
          || from_state == STATUS_TERMINATED
          || from_state == STATUS_FAILED)
    {
      // we are done and that's it
      LOG_WARN(_logger, "invalid state transition!");
      throw StateTransitionException(std::string("final state -> ") + getStatusAsString(to_state));
    }

	_status = status;

	LOG_DEBUG(_logger, "updated status of activity " << _id << " to: " << getStatusAsString(_status));
	
	// notify observers
	if (_observers.size()>0) {
		Event event(_id,Event::EVENT_ACTIVITY,getStatusAsString());
		for (unsigned int i = 0; i<_observers.size(); i++ ) {
			_observers[i]->update(event);
		}
	}
}

string Activity::getStatusAsString(Activity::status_t status) const {
	switch (status) {
	case (STATUS_UNDEFINED): return "UNDEFINED";
	case (STATUS_RUNNING): return "RUNNING";
	case (STATUS_INITIATED): return "INITIATED";
	case (STATUS_SUSPENDED): return "SUSPENDED";
	case (STATUS_ACTIVE): return "ACTIVE";
	case (STATUS_TERMINATED): return "TERMINATED";
	case (STATUS_COMPLETED): return "COMPLETED";
	case (STATUS_FAILED): return "FAILED";
	}
	return "UNDEFINED";
}

Activity::status_t Activity::waitForStatusChangeFrom(Activity::status_t oldStatus) const {
    while (_status == oldStatus) {
    	// ToDo: replace by monitor waiting.
    	usleep(_wfhP->getSleepTime());
    }
    return _status;
}

void Activity::waitForStatusChangeTo(Activity::status_t newStatus) const {
    while (_status != newStatus) {
    	// ToDo: replace by monitor waiting.
    	usleep(_wfhP->getSleepTime());
    }
}

void Activity::waitForStatusChangeToCompletedOrTerminated() const {
    while (_status != STATUS_COMPLETED && _status != STATUS_TERMINATED) {
    	// ToDo: replace by monitor waiting.
    	usleep(_wfhP->getSleepTime());
    }
}

void Activity::attachObserver(Observer* observerP) {
	_observers.push_back(observerP);
}

// notify observers
void Activity::notifyObservers(int type, const string& message, parameter_list_t* tokensP) {
	Event event(_id,type,message,tokensP);
	for (unsigned int i = 0; i<_observers.size(); i++ ) {
		_observers[i]->update(event);
	}
}

/////////////////////////////////////////
// Delegation from Interface Spda2Gwes -> GWES -> WorkflowHandler
/////////////////////////////////////////

void Activity::activityDispatched() {
	LOG_DEBUG(_logger, _id << ": activityDispatched()...");
	setStatus(Activity::STATUS_ACTIVE);
}

void Activity::activityFailed(const parameter_list_t &output) {
	LOG_DEBUG(_logger, _id << ": activityFailed()...");
	// set outputs
	LOG_DEBUG(_logger, "Setting result parameters (parameter list size=" << output.size() << ") ...");
	_toP->tokens = output;
	setStatus(Activity::STATUS_TERMINATED);
}

void Activity::activityFinished(const parameter_list_t &output) {
	LOG_DEBUG(_logger, _id << ": activityFinished()...");
	// set outputs
	LOG_DEBUG(_logger, "Setting result parameters (parameter list size=" << output.size() << ") ...");
	_toP->tokens = output;
	setStatus(Activity::STATUS_COMPLETED);
}

void Activity::activityCanceled() {
	LOG_DEBUG(_logger, _id << ": activityCanceled()...");
	setStatus(Activity::STATUS_TERMINATED);
}

} // end namespace gwes
