#include <iostream>
#include <ostream>

#include "TestGWES.h"
#include "TestWorkflow.h"
#include "TestPreStackProWorkflow.h"

using namespace std;
using namespace gwdl;
using namespace gwes;
 
int main() 
{
   gwes::GWES gwes;
   
   testGWES(gwes);
   
   string twd = getTestWorkflowDirectory();
   Workflow workflow;

   workflow = testWorkflow(twd + "/simple.gwdl",gwes);
   assert(gwes.getStatusAsString(workflow)=="COMPLETED");
   
   workflow = testWorkflow(twd + "/split-token.gwdl",gwes);
   assert(gwes.getStatusAsString(workflow)=="COMPLETED");
   // ToDo: test on correct values on output tokens

   //  workflow = testWorkflow(twd + "/control-loop.gwdl",gwes);
//  assert(gwes.getStatusAsString(workflow)=="COMPLETED");

// Will not work on auto build because shell scripts are not installed there.
//   workflow = testWorkflow(twd + "/concatenateIt.gwdl",gwes);
//   assert(workflow.getProperties().get("status")=="COMPLETED");
//   assert(gwes.getStatusAsString(workflow)=="COMPLETED");

   workflow = testWorkflow(twd + "/concatenateIt_fail.gwdl",gwes);
   assert(gwes.getStatusAsString(workflow)=="TERMINATED");

   workflow = testPreStackProWorkflow(twd + "/psp-grd.gwdl",gwes);
   
   return 0;
}

