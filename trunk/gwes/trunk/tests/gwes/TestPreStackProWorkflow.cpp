// std
#include <iostream>
#include <fstream>
#include <sstream>
// gwes
#include <gwes/Channel.h>
// test
#include "PreStackPro.h"
#include "TestPreStackProWorkflow.h"

using namespace std;
using namespace gwdl;
using namespace gwes;
 
Workflow& testPreStackProWorkflow(string workflowfn, gwes::GWES &gwes) {
	cout << "============== BEGIN EXECUTION " << workflowfn << "==============" << endl;

	Workflow* wfP = new Workflow(workflowfn);
	
	// initiate workflow
    cout << "initiating workflow ..." << endl;
    string workflowId = gwes.initiate(*wfP,"test");
    
	// register channel with source observer (=PreStackPro)
    PreStackPro* psp = new PreStackPro();
    Channel* channelP = new Channel(psp);
    // the connect method also sets destination observer (=WorkflowHandler) in channel
    gwes.connect(channelP, workflowId);
    // now we can set the destination observer (=WorkflowHandler) in PreStackPro.  
    psp->setDestinationObserver(channelP->_destinationP);
    
	// start workflow
    gwes.start(workflowId);
    
    // wait for workflow to end
    WorkflowHandler* wfhP = gwes.getWorkflowHandlerTable().get(workflowId);
    
    cout << "waiting for workflow to complete..." << endl;
    wfhP->waitForStatusChangeToCompletedOrTerminated();
	
    // print workflow
    cout << *wfP << endl;
	cout << "============== END EXECUTION " << workflowfn << "==============" << endl;
    
    return *wfP;
}
