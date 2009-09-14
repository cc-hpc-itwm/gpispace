// std
#include <fstream>
//fhglog
#include <fhglog/fhglog.hpp>
// gwes
#include <gwes/Channel.h>
// test
#include "PreStackPro.h"
#include "TestPreStackProWorkflow.h"

using namespace std;
using namespace fhg::log;
using namespace gwdl;
using namespace gwes;
 
Workflow& testPreStackProWorkflow(string workflowfn, gwes::GWES &gwes) {
	logger_t logger(Logger::get("gwdl"));

	LOG_INFO(logger, "============== BEGIN EXECUTION " << workflowfn << "==============");

	Workflow* wfP = new Workflow(workflowfn);
	
	// initiate workflow
    LOG_INFO(logger, "initiating workflow ...");
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
    
    LOG_INFO(logger, "waiting for workflow to complete...");
    wfhP->waitForStatusChangeToCompletedOrTerminated();
	
    // print workflow
    LOG_INFO(logger, *wfP);
	LOG_INFO(logger, "============== END EXECUTION " << workflowfn << "==============");
    
    return *wfP;
}
