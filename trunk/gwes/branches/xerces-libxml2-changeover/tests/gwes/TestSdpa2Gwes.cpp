/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
// tests
#include "TestSdpa2Gwes.h"
#include "SdpaDummy.h"
#include "TestGWES.h"
// gwdl
#include <gwdl/Workflow.h>
#include <gwdl/Libxml2Builder.h>
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

CPPUNIT_TEST_SUITE_REGISTRATION( gwes::tests::TestSdpa2Gwes );

void TestSdpa2Gwes::testWorkflowWithSdpaActivity() {
	logger_t logger(getLogger("gwes"));
	LOG_INFO(logger, "============== BEGIN testWorkflowWithSdpaActivity =============");

	// create SDPA dummy. SDPA dummy creates own local gwes.
	SdpaDummy sdpa;

	// create workflow_t object from file
	string fn = Utils::expandEnv("${GWES_CPP_HOME}/workflows/test/simple-sdpa-test.gwdl"); 
	LOG_INFO(logger, "reading workflow '"+fn+"'...");
	Libxml2Builder builder;
	workflow_t::ptr_t wfP;
	try {
		wfP = builder.deserializeWorkflowFromFile(fn);
	} catch (const WorkflowFormatException &e) {
		LOG_ERROR(logger, "WorkflowFormatException: " << e.what());
		throw;
	}
	CPPUNIT_ASSERT(wfP->getEnabledTransitions().size() == 1);
			
	// start workflow
	workflow_id_t wfId = sdpa.submitWorkflow(wfP);

	// poll for completion of workflow
	int timeout = 50; // 5 Seconds
	SdpaDummy::ogsa_bes_status_t status = sdpa.getWorkflowStatus(wfId);
	while (timeout-- > 0 && (status == SdpaDummy::PENDING || status == SdpaDummy::RUNNING)) {
		usleep(100000);
		status = sdpa.getWorkflowStatus(wfId);
		LOG_INFO(logger, "status " << wfId << " = " << status);
	}

	// print out workflow XML of finished workflow.
	LOG_INFO(logger, "Finished workflow:\n" << *wfP);

	// check status
	CPPUNIT_ASSERT_EQUAL_MESSAGE("Workflow status", SdpaDummy::FINISHED, status);

	// get and check output
	std::vector<Token::ptr_t> outputTokens = wfP->getPlace("output")->getTokens();
	CPPUNIT_ASSERT_EQUAL_MESSAGE("number output tokens",(std::size_t) 1, outputTokens.size());
	CPPUNIT_ASSERT_MESSAGE("is data", outputTokens[0]->isData());
	CPPUNIT_ASSERT_EQUAL_MESSAGE("data contents", string("<output xmlns=\"\">15</output>"), outputTokens[0]->getData()->serialize());
	
	// deallocate workflowHandler and workflow
	sdpa.removeWorkflow(wfId);
	wfP.reset();

	LOG_INFO(logger, "============== END testWorkflowWithSdpaActivity =============");
}

void TestSdpa2Gwes::testWorkflowWithSdpaSubWorkflow() {
	logger_t logger(getLogger("gwes"));
	LOG_INFO(logger, "============== BEGIN TestSdpa2Gwes::testWorkflowWithSdpaSubWorkflow =============");

	try {
		// create SDPA dummy. SDPA dummy creates own local gwes.
		SdpaDummy sdpa;

		// create workflow_t object from file
		string fn = Utils::expandEnv("${GWES_CPP_HOME}/workflows/test/masterworkflow-sdpa-test.gwdl"); 
		LOG_INFO(logger, "reading workflow '"+fn+"'...");

		Workflow::ptr_t wfP;
		Libxml2Builder builder;
		wfP = builder.deserializeWorkflowFromFile(fn);
		CPPUNIT_ASSERT(wfP->getEnabledTransitions().size() == 1);

		// start workflow
		workflow_id_t wfId = sdpa.submitWorkflow(wfP);

		// poll for completion of workflow
		int timeout = 100; // 10 Seconds
		SdpaDummy::ogsa_bes_status_t status = sdpa.getWorkflowStatus(wfId);
		while (timeout-- > 0 && (status == SdpaDummy::PENDING || status == SdpaDummy::RUNNING)) {
			usleep(100000);
			status = sdpa.getWorkflowStatus(wfId);
			LOG_INFO(logger, "status " << wfId << " = " << status);
		}

		// print out workflow XML of finished workflow.
		LOG_INFO(logger, "Finished workflow:\n" << *wfP);

		// check status
		CPPUNIT_ASSERT_EQUAL_MESSAGE("Workflow status", SdpaDummy::FINISHED, status);

		// get and check output
		std::vector<Token::ptr_t> outputTokens = wfP->getPlace("master-output")->getTokens();
		CPPUNIT_ASSERT_EQUAL_MESSAGE("number output tokens",(std::size_t) 1, outputTokens.size());
		CPPUNIT_ASSERT_MESSAGE("is data", outputTokens[0]->isData());
		CPPUNIT_ASSERT_EQUAL_MESSAGE("data contents", string("<output xmlns=\"\">15</output>"), outputTokens[0]->getData()->serialize());
		
		// deallocate workflowHandler and workflow
		sdpa.removeWorkflow(wfId);
		wfP.reset();
		
	} catch (const WorkflowFormatException &e) {
		LOG_ERROR(logger, "WorkflowFormatException: " << e.what());
		throw;
	} catch (const NoSuchWorkflowElement &e) {
		LOG_ERROR(logger, "NoSuchWorkflowElement: " << e.what());
		throw;
	}
	
	LOG_INFO(logger, "============== END TestSdpa2Gwes::testWorkflowWithSdpaSubWorkflow =============");
}
