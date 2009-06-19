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
   Workflow& workflow = testWorkflow(twd + "/concatenateIt.gwdl",gwes);
   assert(workflow.getProperties().get("status")=="COMPLETED");
   assert(gwes.getStatusAsString(workflow)=="COMPLETED");

   workflow = testPreStackProWorkflow(twd + "/psp-grd.gwdl",gwes);
   workflow.saveToFile("tests/gwes/psp-grd_COMPLETED.gwdl");
   
   return 0;
}

