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

using namespace gwes;
using namespace gwdl;
using namespace std;

SdpaDummy::SdpaDummy() {
	cout << "SdpaDummy::SdpaDummy() ... " << endl;
	_gwesP = new GWES();
	_gwesP->registerHandler(this);
}

SdpaDummy::~SdpaDummy() {
	cout << "SdpaDummy::~SdpaDummy() ... " << endl;
	delete _gwesP;
}

gwes::Gwes2Sdpa::~Gwes2Sdpa() {}

/**
 * The SDPA dummy uses the local gwes for submitting sub-workflows.
 */
workflow_id_t SdpaDummy::submitWorkflow(workflow_t &workflow) {
	cout << "SdpaDummy::submitWorkflow(" << workflow.getID() << ")... " << endl;
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
	cout << "SdpaDummy::submitActivity(" << activity.getID() << ")... " << endl;
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
		cerr << "exception: " << e.message << endl;
	} catch (NoSuchActivityException e) {
		cerr << "exception: " << e.message << endl;
	}
	return activity.getID();
}

/**
 * Cancel a sub workflow that has previously been submitted to
 * the SDPA. The parent job has to cancel all children.
 */
void SdpaDummy::cancelWorkflow(const workflow_id_t &workflowId) throw (NoSuchWorkflowException) {
	cout << "SdpaDummy::cancelWorkflow(" << workflowId << ")... " << endl;
	_gwesP->cancelWorkflow(workflowId);
}

/**
 * Cancel an atomic activity that has previously been submitted to
 * the SDPA.
 */
void SdpaDummy::cancelActivity(const activity_id_t &activityId)  throw (NoSuchActivityException) {
	cout << "SdpaDummy::cancelActivity(" << activityId << ")... " << endl;
	// ToDo: implement!
	
}

/**
 * Notify the SDPA that a workflow finished (state transition
 * from running to finished).
 */
void SdpaDummy::workflowFinished(const workflow_id_t &workflowId) throw (NoSuchWorkflowException) {
	cout << "SdpaDummy::workflowFinished(" << workflowId << ")." << endl;
}

/**
 * Notify the SDPA that a workflow failed (state transition
 * from running to failed).
 */
void SdpaDummy::workflowFailed(const workflow_id_t &workflowId) throw (NoSuchWorkflowException) {
	cout << "SdpaDummy::workflowFailed(" << workflowId << ")." << endl;
}

/**
 * Notify the SDPA that a workflow has been canceled (state
 * transition from * to terminated.
 */ 
void SdpaDummy::workflowCanceled(const workflow_id_t &workflowId) throw (NoSuchWorkflowException) {
	cout << "SdpaDummy::workflowCanceled(" << workflowId << ")." << endl;
}
