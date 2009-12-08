/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
// tests
#include "TestTransformAtomicActivityToWorkflow.h"
// gwes
#include <gwes/GWES.h>
#include <gwes/Utils.h>
// gwdl
#include <gwdl/Libxml2Builder.h>
//fhglog
#include <fhglog/fhglog.hpp>

using namespace gwes;
using namespace gwdl;
using namespace gwes::tests;
using namespace fhg::log;
using namespace std;

CPPUNIT_TEST_SUITE_REGISTRATION( gwes::tests::TestTransformAtomicActivityToWorkflow );

TestTransformAtomicActivityToWorkflow::TestTransformAtomicActivityToWorkflow() {
	LOG_INFO(logger_t(getLogger("gwes")), "TestTransformAtomicActivityToWorkflow() ... ");
	_gwesP = new GWES();
	_gwesP->registerHandler(this);
	_activityFinished = false;
	_workflowCanceled = false;
}

TestTransformAtomicActivityToWorkflow::~TestTransformAtomicActivityToWorkflow() {
	LOG_INFO(logger_t(getLogger("gwes")), "~TestTransformAtomicActivityToWorkflow() ... ");
	delete _gwesP;
}

///////////////////////////////////
// Test methods
///////////////////////////////////
void TestTransformAtomicActivityToWorkflow::testTransform() {
	logger_t logger(getLogger("gwes"));
	LOG_INFO(logger, "============== BEGIN TestTransformAtomicActivityToWorkflow::testTransform =============");
	// create workflow_t object from file
	string fn = Utils::expandEnv("${GWES_CPP_HOME}/workflows/test/simple-sdpa-test.gwdl"); 
	LOG_INFO(logger, "reading workflow '"+fn+"'...");

	Libxml2Builder builder;
	Workflow::ptr_t wfP;
	try {
		wfP = builder.deserializeWorkflowFromFile(fn);
	} catch (const WorkflowFormatException &e) {
		LOG_ERROR(logger, "WorkflowFormatException: " << e.what());
		throw;
	}
	// start workflow
	_workflowId = _gwesP->submitWorkflow(wfP);

	// test continues on callback handler (refer to method "submitActivity")
	while (!_activityFinished) {
		usleep(100000);
	}
	
	_gwesP->cancelWorkflow(_workflowId);
	while (!_workflowCanceled) {
		LOG_INFO(logger, "Waiting for workflow cancel notification...");
		usleep(100000);
	}
	
	LOG_INFO(logger, "============== END TestTransformAtomicActivityToWorkflow::testTransform =============");
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
activity_id_t TestTransformAtomicActivityToWorkflow::submitActivity(activity_t &activity) {
	logger_t logger(getLogger("gwes"));
	
	activity_id_t activityId = activity.getID();
	LOG_INFO(logger, "submitActivity(" << activityId << ")...");
	
	workflow_t::ptr_t subworkflow = activity.transform2Workflow();
	LOG_INFO(logger, "resulting subworkflow generated from atomic activity:");
	Workflow wf = static_cast<Workflow&>(*subworkflow);
	
	// check workflow.
	LOG_INFO(logger, wf);
	
	CPPUNIT_ASSERT_EQUAL_MESSAGE("Number of places", (size_t)2, wf.getPlaces().size());
	CPPUNIT_ASSERT_EQUAL_MESSAGE("Number of transitions", (size_t) 1, wf.getTransitions().size());
	// ToDo: include more tests 
	
	_activityFinished = true;
	return activityId;
}

/**
 * Cancel an atomic activity that has previously been submitted to
 * the SDPA.
 */
void TestTransformAtomicActivityToWorkflow::cancelActivity(const activity_id_t &activityId)  throw (NoSuchActivity) {
	LOG_INFO(logger_t(getLogger("gwes")), "cancelActivity(" << activityId << ")...");
	_gwesP->activityCanceled(_workflowId,activityId);
}

/**
 * Notify the SDPA that a workflow finished (state transition
 * from running to finished).
 */
void TestTransformAtomicActivityToWorkflow::workflowFinished(const workflow_id_t &workflowId, const gwdl::workflow_result_t &r) throw (NoSuchWorkflow) {
	LOG_INFO(logger_t(getLogger("gwes")), "workflowFinished(" << workflowId << ").");
    gwdl::deallocate_workflow_result(const_cast<gwdl::workflow_result_t&>(r));
}

/**
 * Notify the SDPA that a workflow failed (state transition
 * from running to failed).
 */
void TestTransformAtomicActivityToWorkflow::workflowFailed(const workflow_id_t &workflowId, const gwdl::workflow_result_t &r) throw (NoSuchWorkflow) {
	LOG_INFO(logger_t(getLogger("gwes")), "workflowFailed(" << workflowId << ").");
    gwdl::deallocate_workflow_result(const_cast<gwdl::workflow_result_t&>(r));
}

/**
 * Notify the SDPA that a workflow has been canceled (state
 * transition from * to terminated.
 */ 
void TestTransformAtomicActivityToWorkflow::workflowCanceled(const workflow_id_t &workflowId, const gwdl::workflow_result_t &r) throw (NoSuchWorkflow) {
	LOG_INFO(logger_t(getLogger("gwes")), "workflowCanceled(" << workflowId << ").");
	_workflowCanceled = true;
    gwdl::deallocate_workflow_result(const_cast<gwdl::workflow_result_t&>(r));
}

