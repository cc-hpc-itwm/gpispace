/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
// tests
#include "TestXPathEvaluation.h"
#include "TestGWES.h"
#include "TestSdpa2Gwes.h"
#include "TestWorkflow.h"
#include "TestPreStackProWorkflow.h"
// 
#include <iostream>
#include <ostream>

using namespace std;
using namespace gwdl;
using namespace gwes;
 
int main() 
{
   gwes::GWES gwes;
   
   testXPathEvaluator();

   // test basic GWES features 
   testGWES(gwes);
   
   // test SDPA2GWES communication
//   testSdpa2Gwes();
//   testGwes2Sdpa();
   
   // test various workflows
   string twd = getTestWorkflowDirectory();
   Workflow workflow;

   workflow = testWorkflow(twd + "/simple.gwdl",gwes);
   assert(gwes.getStatusAsString(workflow)=="COMPLETED");
   
//   workflow = testWorkflow(twd + "/split-token.gwdl",gwes);
//   assert(gwes.getStatusAsString(workflow)=="COMPLETED");
//   // ToDo: test on correct values on output tokens

   workflow = testWorkflow(twd + "/exclusive-choice.gwdl", gwes);
   assert(gwes.getStatusAsString(workflow)=="COMPLETED");
   assert(workflow.getPlace("end_A")->getTokenNumber() == 0);
   assert(workflow.getPlace("end_B")->getTokenNumber() == 1);
   
   workflow = testWorkflow(twd + "/condition-test.gwdl", gwes);
   assert(gwes.getStatusAsString(workflow)=="COMPLETED");
   assert(workflow.getPlace("end_A")->getTokenNumber() == 2);
   assert(workflow.getPlace("end_B")->getTokenNumber() == 1);
   //ToDo: Data is not copied from input edge "x" to output edge "x"
   
   //  workflow = testWorkflow(twd + "/control-loop.gwdl",gwes);
//  assert(gwes.getStatusAsString(workflow)=="COMPLETED");

// Will not work on auto build because shell scripts are not installed there.
//   workflow = testWorkflow(twd + "/concatenateIt.gwdl",gwes);
//   assert(workflow.getProperties().get("status")=="COMPLETED");
//   assert(gwes.getStatusAsString(workflow)=="COMPLETED");

//   workflow = testWorkflow(twd + "/concatenateIt_fail.gwdl",gwes);
//   assert(gwes.getStatusAsString(workflow)=="TERMINATED");
//
//   workflow = testPreStackProWorkflow(twd + "/psp-grd.gwdl",gwes);
//   
   return 0;
}

