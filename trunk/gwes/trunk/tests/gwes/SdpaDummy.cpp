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
//fhglog
#include <fhglog/fhglog.hpp>

using namespace gwes;
using namespace gwdl;
using namespace fhg::log;
using namespace std;

SdpaDummy::SdpaDummy() {
	LOG_INFO(Logger::get("gwes"), "SdpaDummy() ... ");
	_gwesP = new GWES();
	_gwesP->registerHandler(this);
}

SdpaDummy::~SdpaDummy() {
	LOG_INFO(Logger::get("gwes"), "~SdpaDummy() ... ");
	delete _gwesP;
}

gwes::Gwes2Sdpa::~Gwes2Sdpa() {}

/**
 * The SDPA dummy uses the local gwes for submitting sub-workflows.
 */
workflow_id_t SdpaDummy::submitWorkflow(workflow_t &workflow) {
	LOG_INFO(Logger::get("gwes"), "submitWorkflow(" << workflow.getID() << ")...");
	return _gwesP->submitWorkflow(workflow);
}

/**
 * Submit an atomic activity to the SDPA.
 * This method is to be called by the GWES in order to delegate
 * the execution of activities.
 * The SDPA will use the callback handler Sdpa2Gwes in order
 * to notify the GWES about activity status transitions.
 */
activity_id_t SdpaDummy::submitActivity(activity_t &activity) {
	LOG_INFO(Logger::get("gwes"), "submitActivity(" << activity.getID() << ")...");
	workflow_id_t workflowId = activity.getWorkflowHandler()->getID();
	// a real SDPA implementation should really dispatch the activity here
	try {
		_gwesP->activityDispatched(workflowId, activity.getID());
		// find and fill output tokens
		parameter_list_t tokens = activity.getTransitionOccurrence()->tokens;
		for (parameter_list_t::iterator it=tokens.begin(); it!=tokens.end(); ++it) {
			switch (it->scope) {
			case (TokenParameter::SCOPE_READ):
			case (TokenParameter::SCOPE_INPUT):
			case (TokenParameter::SCOPE_WRITE):
				continue;
			case (TokenParameter::SCOPE_OUTPUT):
				it->tokenP = new Token("<data><sdpaOutput>15</sdpaOutput></data>"); 
				break;
			}
		}
		_gwesP->activityFinished(workflowId, activity.getID(), tokens);
	} catch (NoSuchWorkflowException e) {
		LOG_WARN(Logger::get("gwes"), "exception: " << e.message);
	} catch (NoSuchActivityException e) {
		LOG_WARN(Logger::get("gwes"), "exception: " << e.message);
	}
	return activity.getID();
}

/**
 * Cancel a sub workflow that has previously been submitted to
 * the SDPA. The parent job has to cancel all children.
 */
void SdpaDummy::cancelWorkflow(const workflow_id_t &workflowId) throw (NoSuchWorkflowException) {
	LOG_INFO(Logger::get("gwes"), "cancelWorkflow(" << workflowId << ")...");
	_gwesP->cancelWorkflow(workflowId);
}

/**
 * Cancel an atomic activity that has previously been submitted to
 * the SDPA.
 */
void SdpaDummy::cancelActivity(const activity_id_t &activityId)  throw (NoSuchActivityException) {
	LOG_INFO(Logger::get("gwes"), "cancelActivity(" << activityId << ")...");
	// ToDo: implement!

}

/**
 * Notify the SDPA that a workflow finished (state transition
 * from running to finished).
 */
void SdpaDummy::workflowFinished(const workflow_id_t &workflowId) throw (NoSuchWorkflowException) {
	LOG_INFO(Logger::get("gwes"), "workflowFinished(" << workflowId << ").");
}

/**
 * Notify the SDPA that a workflow failed (state transition
 * from running to failed).
 */
void SdpaDummy::workflowFailed(const workflow_id_t &workflowId) throw (NoSuchWorkflowException) {
	LOG_INFO(Logger::get("gwes"), "workflowFailed(" << workflowId << ").");
}

/**
 * Notify the SDPA that a workflow has been canceled (state
 * transition from * to terminated.
 */ 
void SdpaDummy::workflowCanceled(const workflow_id_t &workflowId) throw (NoSuchWorkflowException) {
	LOG_INFO(Logger::get("gwes"), "workflowCanceled(" << workflowId << ").");
}
