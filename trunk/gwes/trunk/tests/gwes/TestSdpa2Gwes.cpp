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
// std
#include <iostream>
#include <ostream>
#include <assert.h>

using namespace gwes;
using namespace gwdl;
using namespace std;

void testSdpa2Gwes() {
	cout << "============== BEGIN SDPA2GWES TEST =============" << endl;
	
	// create SDPA dummy and register to GWES
	SdpaDummy* sdpaP = new SdpaDummy();
	
	// parse workflow from file
	string fn = getTestWorkflowDirectory() + "/simple.gwdl"; 
	cout << "testSdpa2Gwes(): reading workflow '"+fn+"'..." << endl;
	Workflow wf = (Workflow) fn;
	
	// submit workflow
	string workflowId = sdpaP->submitWorkflow(wf);
	cout << "workflowId = " << workflowId << endl;
	
	// wait one second until worklfow finished in order to prevent segfault because of 
	// destroyed workflow object.
	// todo: this should be replaced by a waiting monitor on SdpaDummy
	usleep(1000000);

	string status = wf.getProperties().get("status");
	cout << "TEST " << status << endl;
	assert(status == "COMPLETED");

	cout << "============== END SDPA2GWES TEST =============" << endl;
}

void testGwes2Sdpa() {
	cout << "============== BEGIN GWES2SDPA TEST =============" << endl;
	
	// create SDPA dummy and register to GWES
	SdpaDummy* sdpaP = new SdpaDummy();
	
	// parse workflow from file
	string fn = getTestWorkflowDirectory() + "/simple-sdpa-test.gwdl"; 
	cout << "testGwes2Sdpa(): reading workflow '"+fn+"'..." << endl;
	Workflow wf = (Workflow) fn;
	
	// submit workflow
	string workflowId = sdpaP->submitWorkflow(wf);
	cout << "workflowId = " << workflowId << endl;
	
	// wait one second until worklfow finished in order to prevent segfault because of 
	// destroyed workflow object.
	// todo: this should be replaced by a waiting monitor on SdpaDummy
	usleep(1000000);
	
	// ToDo: the following line results in segfault!
	//cout << wf << endl;
	string status = wf.getProperties().get("status");
	cout << "TEST " << status << endl;
	assert(status == "COMPLETED");

	cout << "============== END GWES2SDPA TEST =============" << endl;
}
