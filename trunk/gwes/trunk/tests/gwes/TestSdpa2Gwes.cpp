/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
// tests
#include <gwes/internal_types.h>

#include "TestSdpa2Gwes.h"
#include "SdpaDummy.h"
#include "TestGWES.h"
// gwdl
#include <gwdl/Workflow.h>
// gwes
#include <gwes/Utils.h>
#include <gwes/Types.h>
//fhglog
#include <fhglog/fhglog.hpp>
// std
#include <iostream>
#include <ostream>

using namespace gwes;
using namespace gwdl;
using namespace fhg::log;
using namespace std;
using namespace gwes::tests;

CPPUNIT_TEST_SUITE_REGISTRATION( gwes::tests::Sdpa2GwesAPITest );

void Sdpa2GwesAPITest::testWorkflowWithSdpaActivity() {
	logger_t log(getLogger("gwes"));
	LOG_INFO(log, "============== BEGIN testWorkflowWithSdpaActivity =============");
	
	// create SDPA dummy. SDPA dummy creates own local gwes.
	SdpaDummy sdpa;
	
	// create workflow_t object from file
	string fn = Utils::expandEnv("${GWES_CPP_HOME}/workflows/test/simple-sdpa-test.gwdl"); 
	LOG_INFO(log, "reading workflow '"+fn+"'...");
	Workflow wf(fn);
	CPPUNIT_ASSERT(wf.getEnabledTransitions().size() == 1);
			
	// start workflow
	workflow_id_t wfId = sdpa.submitWorkflow(wf);
	
	// poll for completion of workflow
	SdpaDummy::ogsa_bes_status_t status = sdpa.getWorkflowStatus(wfId);
	while (status == SdpaDummy::PENDING || status == SdpaDummy::RUNNING) {
		usleep(100000);
		status = sdpa.getWorkflowStatus(wfId);
		LOG_INFO(log, "status " << wfId << " = " << status);
	}
	
	// print out workflow XML of finished workflow.
	LOG_INFO(log, "Finished workflow:\n" << wf);
	
	// get and check output
	std::vector<Token*> outputTokens = wf.getPlace("output")->getTokens();
	CPPUNIT_ASSERT_EQUAL_MESSAGE("number output tokens",(std::size_t) 1, outputTokens.size());
	
	LOG_INFO(log, "============== END testWorkflowWithSdpaActivity =============");
}

void Sdpa2GwesAPITest::testSdpa2Gwes() {
	logger_t logger(getLogger("gwes"));

	LOG_INFO(logger, "============== BEGIN SDPA2GWES TEST =============");
	
	LOG_WARN(logger, "======= TEST CURRENTLY DEACTIVATED! ========");

	/// ToDo: Redesign of Interface GWES <-> SDPA 
//	try {
//		// create SDPA dummy and register to GWES
//		SdpaDummy* sdpaP = new SdpaDummy();
//
//		// parse workflow from file
//		string fn = Utils::expandEnv("${GWES_CPP_HOME}/workflows/test/simple.gwdl"); 
//		LOG_INFO(logger, "testSdpa2Gwes(): reading workflow '"+fn+"'...");
//		Workflow wf = (Workflow) fn;
//
//		// submit workflow
//		string workflowId = sdpaP->submitWorkflow(wf);
//		LOG_INFO(logger, "workflowId = " << workflowId);
//
//		// wait one second until worklfow finished in order to prevent segfault because of 
//		// destroyed workflow object.
//		// todo: this should be replaced by a waiting monitor on SdpaDummy
//		usleep(1000000);
//
//		string status = wf.getProperties().get("status");
//		LOG_INFO(logger, "TEST " << status);
//		CPPUNIT_ASSERT(status == "COMPLETED");
//	} catch (WorkflowFormatException e) {
//		LOG_FATAL(logger, "WorkflowFormatException: " << e.message);
//		CPPUNIT_ASSERT(false);
//	}

	LOG_INFO(logger, "============== END SDPA2GWES TEST =============");
}

void Sdpa2GwesAPITest::testGwes2Sdpa() {
	logger_t logger(getLogger("gwes"));
	LOG_INFO(logger, "============== BEGIN GWES2SDPA TEST =============");

	/// ToDo: Redesign of Interface GWES <-> SDPA 
//	try {
//		// create SDPA dummy and register to GWES
//		SdpaDummy* sdpaP = new SdpaDummy();
//		
//		// create activity
//		Activity* activity = new SubWorkflowActivity(handlerP,transitionOccurenceP,operationP);
//
//		// submit activity
//
//		// wait one second until worklfow finished in order to prevent segfault because of 
//		// destroyed workflow object.
//		// todo: this should be replaced by a waiting monitor on SdpaDummy
//		usleep(1000000);
//
//		// ToDo: the following line results in segfault!
//		//LOG_INFO(logger, wf);
//		string status = wf.getProperties().get("status");
//		LOG_INFO(logger, "TEST " << status);
//		CPPUNIT_ASSERT(status == "COMPLETED");
//	} catch (WorkflowFormatException e) {
//		LOG_FATAL(logger, "WorkflowFormatException: " << e.message);
//		CPPUNIT_ASSERT(false);
//	}

	LOG_INFO(logger, "============== END GWES2SDPA TEST =============");
}
