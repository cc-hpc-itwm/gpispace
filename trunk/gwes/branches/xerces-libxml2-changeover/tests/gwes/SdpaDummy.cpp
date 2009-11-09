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
 * Submit an atomic activity to the SDPA.
 * This method is to be called by the GWES in order to delegate
 * the execution of activities.
 * The SDPA will use the callback handler Sdpa2Gwes in order
 * to notify the GWES about activity status transitions.
 */
activity_id_t SdpaDummy::submitActivity(activity_t &activity) {
	LOG_INFO(logger_t(getLogger("gwes")), "submitActivity(" << activity.getID() << ")...");
	workflow_id_t workflowId = activity.getWorkflowHandler()->getID();
	// a real SDPA implementation should really dispatch the activity here
	try {
		_gwesP->activityDispatched(workflowId, activity.getID());
		// find and fill dummy output tokens
		parameter_list_t* tokensP = activity.getTransitionOccurrence()->getTokens();
		for (parameter_list_t::iterator it=tokensP->begin(); it!=tokensP->end(); ++it) {
			switch (it->scope) {
			case (TokenParameter::SCOPE_READ):
			case (TokenParameter::SCOPE_INPUT):
			case (TokenParameter::SCOPE_WRITE):
				continue;
			case (TokenParameter::SCOPE_OUTPUT):
				it->tokenP = new Token(Data::ptr_t(new Data(string("<data><output xmlns=\"\">15</output></data>"))));
				LOG_INFO(logger_t(getLogger("gwes")), "Generated dummy output token: " << *it->tokenP);
				break;
			}
		}
		_gwesP->activityFinished(workflowId, activity.getID(), *tokensP);
	} catch (const NoSuchWorkflowException &e) {
		LOG_ERROR(logger_t(getLogger("gwes")), "exception: " << e.message);
	} catch (const NoSuchActivityException &e) {
		LOG_ERROR(logger_t(getLogger("gwes")), "exception: " << e.message);
	}
	return activity.getID();
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
void SdpaDummy::workflowFinished(const workflow_id_t &workflowId) throw (NoSuchWorkflow) {
	LOG_INFO(logger_t(getLogger("gwes")), "workflowFinished(" << workflowId << ").");
	_wfStatusMap.erase(workflowId);
	_wfStatusMap.insert(pair<workflow_id_t,ogsa_bes_status_t>(workflowId,FINISHED));
	logWorkflowStatus();
}

/**
 * Notify the SDPA that a workflow failed (state transition
 * from running to failed).
 */
void SdpaDummy::workflowFailed(const workflow_id_t &workflowId) throw (NoSuchWorkflow) {
	LOG_INFO(logger_t(getLogger("gwes")), "workflowFailed(" << workflowId << ").");
	_wfStatusMap.erase(workflowId);
	_wfStatusMap.insert(pair<workflow_id_t,ogsa_bes_status_t>(workflowId,FAILED));
	logWorkflowStatus();
}

/**
 * Notify the SDPA that a workflow has been canceled (state
 * transition from * to terminated.
 */ 
void SdpaDummy::workflowCanceled(const workflow_id_t &workflowId) throw (NoSuchWorkflow) {
	LOG_INFO(logger_t(getLogger("gwes")), "workflowCanceled(" << workflowId << ").");
	_wfStatusMap.erase(workflowId);
	_wfStatusMap.insert(pair<workflow_id_t,ogsa_bes_status_t>(workflowId,CANCELED));
	logWorkflowStatus();
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

workflow_id_t SdpaDummy::submitWorkflow(workflow_t &workflow) {
	workflow_id_t workflowId = _gwesP->submitWorkflow(workflow);
	_wfStatusMap.insert(pair<workflow_id_t,ogsa_bes_status_t>(workflowId,RUNNING));
	logWorkflowStatus();
	return workflowId;
}

SdpaDummy::ogsa_bes_status_t SdpaDummy::getWorkflowStatus(workflow_id_t workflowId) {
	return _wfStatusMap.find(workflowId)->second;
}
