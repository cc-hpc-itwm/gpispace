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
// gwes
#include <gwes/Utils.h>
//fhglog
#include <fhglog/fhglog.hpp>
// std
#include <iostream>
#include <ostream>
#include <assert.h>

using namespace gwes;
using namespace gwdl;
using namespace fhg::log;
using namespace std;

void testSdpa2Gwes() {
	logger_t logger(getLogger("gwes"));

	LOG_INFO(logger, "============== BEGIN SDPA2GWES TEST =============");

	try {
		// create SDPA dummy and register to GWES
		SdpaDummy* sdpaP = new SdpaDummy();

		// parse workflow from file
		string fn = Utils::expandEnv("${GWES_CPP_HOME}/workflows/test/simple.gwdl"); 
		LOG_INFO(logger, "testSdpa2Gwes(): reading workflow '"+fn+"'...");
		Workflow wf = (Workflow) fn;

		// submit workflow
		string workflowId = sdpaP->submitWorkflow(wf);
		LOG_INFO(logger, "workflowId = " << workflowId);

		// wait one second until worklfow finished in order to prevent segfault because of 
		// destroyed workflow object.
		// todo: this should be replaced by a waiting monitor on SdpaDummy
		usleep(1000000);

		string status = wf.getProperties().get("status");
		LOG_INFO(logger, "TEST " << status);
		assert(status == "COMPLETED");
	} catch (WorkflowFormatException e) {
		LOG_WARN(logger, "WorkflowFormatException: " << e.message);
///		assert(false);
	}

	LOG_INFO(logger, "============== END SDPA2GWES TEST =============");
}

void testGwes2Sdpa() {
	logger_t logger(getLogger("gwes"));
	LOG_INFO(logger, "============== BEGIN GWES2SDPA TEST =============");

	try {
		// create SDPA dummy and register to GWES
		SdpaDummy* sdpaP = new SdpaDummy();

		// parse workflow from file
		string fn = Utils::expandEnv("${GWES_CPP_HOME}/workflows/test/simple-sdpa-test.gwdl"); 
		LOG_INFO(logger, "testGwes2Sdpa(): reading workflow '"+fn+"'...");
		Workflow wf = (Workflow) fn;

		// submit workflow
		string workflowId = sdpaP->submitWorkflow(wf);
		LOG_INFO(logger, "workflowId = " << workflowId);

		// wait one second until worklfow finished in order to prevent segfault because of 
		// destroyed workflow object.
		// todo: this should be replaced by a waiting monitor on SdpaDummy
		usleep(1000000);

		// ToDo: the following line results in segfault!
		//LOG_INFO(logger, wf);
		string status = wf.getProperties().get("status");
		LOG_INFO(logger, "TEST " << status);
		assert(status == "COMPLETED");
	} catch (WorkflowFormatException e) {
		LOG_WARN(logger, "WorkflowFormatException: " << e.message);
		///		assert(false);
	}

	LOG_INFO(logger, "============== END GWES2SDPA TEST =============");
}
