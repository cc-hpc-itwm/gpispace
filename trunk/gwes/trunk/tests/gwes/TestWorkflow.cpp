// std
#include <iostream>
#include <fstream>
#include <sstream>
// gwes
#include "../../gwes_cpp/src/WorkflowObserver.h"
#include "../../gwes_cpp/src/Channel.h"
// test
#include "TestWorkflow.h"

using namespace std;
using namespace gwdl;
using namespace gwes;
 
Workflow& testWorkflow(string workflowfn, gwes::GWES &gwes) {
	cout << "============== BEGIN EXECUTION " << workflowfn << "==============" << endl;

	Workflow* wfP = new Workflow(workflowfn);
	
	// initiate workflow
    cout << "initiating workflow ..." << endl;
    string workflowId = gwes.initiate(*wfP,"test");
    
	// register channel with source observer
    WorkflowObserver* observerP = new WorkflowObserver();
    Channel* channelP = new Channel(observerP);
    gwes.connect(channelP, workflowId);
    
	// start workflow
    gwes.start(workflowId);
    
    // wait for workflow to end
    WorkflowHandler* wfhP = gwes.getWorkflowHandlerTable().get(workflowId);
    
    wfhP->waitForStatusChangeToCompletedOrTerminated();
	
    // print workflow
    cout << *wfP << endl;
	cout << "============== END EXECUTION " << workflowfn << "==============" << endl;
    
    return *wfP;
}
