/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
// tests
#include "SdpaDummy.h"
// gwes
#include <gwes/GWES.h>
// gwdl
#include <gwdl/Libxml2Builder.h>
//fhglog
#include <fhglog/fhglog.hpp>

using namespace gwes;
using namespace gwdl;
using namespace fhg::log;
using namespace std;

SdpaDummy::SdpaDummy() {
	LOG_INFO(logger_t(getLogger("gwes")), "SdpaDummy() ... ");
	_gwesP = new GWES();
	_gwesP->registerHandler(this);
}

SdpaDummy::~SdpaDummy() {
	LOG_INFO(logger_t(getLogger("gwes")), "~SdpaDummy() ... ");
	delete _gwesP;
}

///////////////////////////////////
// Interface Gwes2Sdpa
///////////////////////////////////

/**
 * Submit an atomic activity or an activity refering a subworkflow to the SDPA.
 * This method is to be called by the GWES in order to delegate
 * the execution of activities or subworkflows.
 * The SDPA will use the callback handler Sdpa2Gwes in order
 * to notify the GWES about activity status transitions.
 */
activity_id_t SdpaDummy::submitActivity(activity_t &activity) {
	logger_t logger(getLogger("gwes"));
	
	// remember activityId and workflowId 
	activity_id_t activityId = activity.getID();
	LOG_INFO(logger, "submitActivity(" << activityId << ")...");
	workflow_id_t workflowId = static_cast<Activity&>(activity).getWorkflowHandler()->getID();
	
	// get operation name and resource name
	string operationName = static_cast<Activity&>(activity).getOperationCandidate()->getOperationName();
	string resourceName = static_cast<Activity&>(activity).getOperationCandidate()->getResourceName();
	
	// decide if this is atomic activity or sub workflow
	string operationType = static_cast<Activity&>(activity).getOperationCandidate()->getType();
	if (operationType == "sdpa") {                 // atomic activity
		parameter_list_t* parameters = static_cast<Activity&>(activity).getTransitionOccurrence()->getTokens();
		executeAtomicActivity(activityId, workflowId, operationName, resourceName, parameters);
	} else if (operationType == "sdpa/workflow") { // sub workflow
		executeSubWorkflow(activityId, workflowId, activity);
	}

	return activityId;
}
	

/**
 * The real sdpa should serialize the workflow object and send it to corresponding GWES.
 */
void SdpaDummy::executeSubWorkflow(
		const activity_id_t &activityId, 
		const workflow_id_t &workflowId, 
		activity_t &activity
		) {

	logger_t logger(getLogger("gwes"));
	
	// get parameters
	parameter_list_t* parameters = static_cast<Activity&>(activity).getTransitionOccurrence()->getTokens();
	
	// transform activity to workflow
	workflow_t::ptr_t subworkflow = activity.transform2Workflow();
	
	// set worklfow ID (if not set here, it will be automatically set when initializing workflow.
// 		subworkflow->setID("sdpa-dummy-worklfow-uuid");
	
	// submit workflow
	workflow_id_t subWorkflowId = submitWorkflow(subworkflow);
	
	// notify gwes that activity has been dispatched
	_gwesP->activityDispatched(workflowId, activityId);
	
	// poll for completion of workflow
	int timeout = 100; // 10 Seconds
	ogsa_bes_status_t status = getWorkflowStatus(subWorkflowId);
	while (timeout-- > 0 && (status == PENDING || status == RUNNING)) {
		usleep(100000);
		status = getWorkflowStatus(subWorkflowId);
		LOG_INFO(logger, "status " << subWorkflowId << " = " << status);
	}

	string edgeExpression;

	try {
		switch(status) {
		case FINISHED:
			// copy write/output tokens back to parameter list of parent activity regarding the edge expressions
			for (parameter_list_t::iterator it=parameters->begin(); it!=parameters->end(); ++it) {
				switch (it->scope) {
				case (TokenParameter::SCOPE_READ):
				case (TokenParameter::SCOPE_INPUT):
					continue;
				case (TokenParameter::SCOPE_WRITE):
				case (TokenParameter::SCOPE_OUTPUT):	
					edgeExpression = it->edgeP->getExpression();
				if (edgeExpression.find("$")==edgeExpression.npos) { // ignore XPath expressions
					gwdl::Place::ptr_t placeP = subworkflow->getPlace(edgeExpression);
					it->tokenP = placeP->getTokens()[0]->deepCopy();
					LOG_INFO(logger, "copy token " << it->tokenP->getID() << " to parent activity parameter list...");
				}
				break;
				}
			}

			// notify gwes that activity finished and include parameter list
			_gwesP->activityFinished(workflowId, activityId, *parameters);
			break;
		case FAILED:
			_gwesP->activityFailed(workflowId, activityId, *parameters);
			break;
		case CANCELED:
			_gwesP->activityCanceled(workflowId, activityId);
			break;
		case PENDING:
			LOG_ERROR(logger, "Subworkflow is still in status PENDING after timeout!");
			_gwesP->activityCanceled(workflowId, activityId);
			break;
		case RUNNING:
			LOG_ERROR(logger, "Subworkflow is still in status RUNNING after timeout!");
			_gwesP->activityCanceled(workflowId, activityId);
			break;
		}

	} catch (const NoSuchWorkflowElement &e) {
		LOG_ERROR(logger, "Subworkflow does not contain place that matches edgeExpression \"" << edgeExpression << "\": " << e.what());
		_gwesP->activityFailed(workflowId, activityId, *parameters);
	}

}

/**
 * A real SDPA implementation should really dispatch the activity asynchronously here.
 */
void SdpaDummy::executeAtomicActivity(
		const activity_id_t &activityId, 
		const workflow_id_t &workflowId, 
		const string& operationName, 
		const string& resourceName, 
		parameter_list_t* parameters
		) {
	logger_t logger(getLogger("gwes"));
	LOG_INFO(logger, "executing " << operationName << " on " << resourceName);
	try {
		// notify gwes that activity has been dispatched
		_gwesP->activityDispatched(workflowId, activityId);
		
		// find and fill dummy output tokens
		// iterate though parameter list, which contains all input/output parameters.
		for (parameter_list_t::iterator it=parameters->begin(); it!=parameters->end(); ++it) {
			switch (it->scope) {
			case (TokenParameter::SCOPE_READ):
			case (TokenParameter::SCOPE_INPUT):
			case (TokenParameter::SCOPE_WRITE):
				continue;
			case (TokenParameter::SCOPE_OUTPUT):
				it->tokenP = Token::ptr_t(new Token(Data::ptr_t(new Data(string("<output xmlns=\"\">15</output>")))));
				LOG_INFO(logger, "generated dummy output token: \n" << *it->tokenP);
				break;
			}
		}
		
		// notify gwes that activity finished and include parameter list
		_gwesP->activityFinished(workflowId, activityId, *parameters);
	} catch (const NoSuchWorkflowException &e) {
		LOG_ERROR(logger, "exception: " << e.what());
	} catch (const NoSuchActivityException &e) {
		LOG_ERROR(logger, "exception: " << e.what());
	}
}

/**
 * Cancel an atomic activity that has previously been submitted to
 * the SDPA.
 */
void SdpaDummy::cancelActivity(const activity_id_t &activityId)  throw (NoSuchActivity) {
	LOG_INFO(logger_t(getLogger("gwes")), "cancelActivity(" << activityId << ")...");
	// ToDo: implement!
}

/**
 * Notify the SDPA that a workflow finished (state transition
 * from running to finished).
 */
void SdpaDummy::workflowFinished(const workflow_id_t &workflowId, const gwdl::workflow_result_t &r) throw (NoSuchWorkflow) {
	LOG_INFO(logger_t(getLogger("gwes")), "workflowFinished(" << workflowId << ").");
	_wfStatusMap.erase(workflowId);
	_wfStatusMap.insert(pair<workflow_id_t,ogsa_bes_status_t>(workflowId,FINISHED));
	logWorkflowStatus();
    gwdl::deallocate_workflow_result(const_cast<gwdl::workflow_result_t&>(r));
}

/**
 * Notify the SDPA that a workflow failed (state transition
 * from running to failed).
 */
void SdpaDummy::workflowFailed(const workflow_id_t &workflowId, const gwdl::workflow_result_t &r) throw (NoSuchWorkflow) {
	LOG_INFO(logger_t(getLogger("gwes")), "workflowFailed(" << workflowId << ").");
	_wfStatusMap.erase(workflowId);
	_wfStatusMap.insert(pair<workflow_id_t,ogsa_bes_status_t>(workflowId,FAILED));
	logWorkflowStatus();
    gwdl::deallocate_workflow_result(const_cast<gwdl::workflow_result_t&>(r));
}

/**
 * Notify the SDPA that a workflow has been canceled (state
 * transition from * to terminated.
 */ 
void SdpaDummy::workflowCanceled(const workflow_id_t &workflowId, const gwdl::workflow_result_t &r) throw (NoSuchWorkflow) {
	LOG_INFO(logger_t(getLogger("gwes")), "workflowCanceled(" << workflowId << ").");
	_wfStatusMap.erase(workflowId);
	_wfStatusMap.insert(pair<workflow_id_t,ogsa_bes_status_t>(workflowId,CANCELED));
	logWorkflowStatus();
    gwdl::deallocate_workflow_result(const_cast<gwdl::workflow_result_t&>(r));
}

/////////////////////////////
// helper methods
/////////////////////////////
void SdpaDummy::logWorkflowStatus() {
	logger_t logger = getLogger("gwes");
	map<gwes::workflow_id_t,ogsa_bes_status_t>::iterator it;
	for ( it=_wfStatusMap.begin() ; it != _wfStatusMap.end(); it++ ) {
	    LOG_INFO(logger, "Workflow status: " << (*it).first << " => " << (*it).second);
	}
}

workflow_id_t SdpaDummy::submitWorkflow(workflow_t::ptr_t workflowP) {
	logger_t logger = getLogger("gwes");
	workflow_id_t workflowId = _gwesP->submitWorkflow(workflowP);
	_wfStatusMap.insert(pair<workflow_id_t,ogsa_bes_status_t>(workflowId,RUNNING));
	logWorkflowStatus();
	return workflowId;
}

void SdpaDummy::removeWorkflow(const workflow_id_t &workflowId) {
    _gwesP->removeWorkflow(workflowId);
}

SdpaDummy::ogsa_bes_status_t SdpaDummy::getWorkflowStatus(workflow_id_t workflowId) {
	return _wfStatusMap.find(workflowId)->second;
}
