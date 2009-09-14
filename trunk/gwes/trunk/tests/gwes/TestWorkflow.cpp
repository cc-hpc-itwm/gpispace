// std
#include <iostream>
#include <fstream>
#include <sstream>
//fhglog
#include <fhglog/fhglog.hpp>
// gwdl 
#include <gwdl/WorkflowFormatException.h>
// gwes
#include <gwes/WorkflowObserver.h>
#include <gwes/Channel.h>
// test
#include "TestWorkflow.h"

using namespace std;
using namespace fhg::log;
using namespace gwdl;
using namespace gwes;
 
Workflow& testWorkflow(string workflowfn, gwes::GWES &gwes) {
	logger_t logger(Logger::get("gwdl"));

	LOG_INFO(logger, "============== BEGIN EXECUTION " << workflowfn << "==============");

	try {
		Workflow* wfP = new Workflow(workflowfn);

		// initiate workflow
		LOG_INFO(logger, "initiating workflow ...");
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
		LOG_DEBUG(logger, *wfP);
		LOG_INFO(logger, "============== END EXECUTION " << workflowfn << "==============");
		return *wfP;
	} catch (WorkflowFormatException e) {
		LOG_WARN(logger, "WorkflowFormatException: " << e.message);
		assert(false);
	}
}
