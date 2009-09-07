/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
//gwes
#include <gwes/GWES.h>
#include <gwes/WorkflowObserver.h>
#include <gwes/Channel.h>
//fhglog
#include <fhglog/fhglog.hpp>

using namespace std;
using namespace gwdl;
using namespace gwes;
using namespace fhg::log;

void usage(LoggerApi logger);
string getUserName();

int main(int argc, char* argv[]) {
	LoggerApi logger(Logger::get("gwes"));
	logger.setLevel(LogLevel::INFO);
	Appender::ptr_t appender = Appender::ptr_t(new StreamAppender());
	Formatter::ptr_t formatter = Formatter::ptr_t(Formatter::ShortFormatter());
	appender->setFormat(formatter);
	logger.addAppender(appender);

	string workflowfn;

//	int c;
//	int command=-1;
	//	while ((c = getopt(argc, argv, "s:")) != -1) {
	//		switch (c) {
	//		case 'i':
	//			command = COMMAND_INITIATE;
	//			workflowfn = optarg;
	//			break;
	//		case '?':
	//			if (optopt == 'i')
	//				fprintf(stderr, "Option -%c requires an argument.\n", optopt);
	//			else if (isprint(optopt))
	//				fprintf(stderr, "Unknown option `-%c'.\n", optopt);
	//			else
	//				fprintf(stderr,
	//				"Unknown option character `\\x%x'.\n",
	//				optopt);
	//			return 1;
	//		default:
	//			abort();
	//		}
	//	}

	if (argc < 2) {
		LOG_WARN(logger, "ERROR: workflow not specified!");
		usage(logger);
		exit(1);
	} else {
		workflowfn = argv[1];
	}

	try {
		GWES gwes;
		LOG_INFO(logger, "### BEGIN EXECUTION " << workflowfn);
		Workflow* wfP = new Workflow(workflowfn);

		// initiate workflow
		LOG_INFO(logger, "initiating workflow ...");
		string workflowId = gwes.initiate(*wfP, getUserName());

		// register channel with source observer
		WorkflowObserver* observerP = new WorkflowObserver();
		Channel* channelP = new Channel(observerP);
		gwes.connect(channelP, workflowId);

		// execute workflow
		gwes.execute(workflowId);

		//	// wait for workflow to end
		//	WorkflowHandler* wfhP = gwes.getWorkflowHandlerTable().get(workflowId);
		//	wfhP->waitForStatusChangeToCompletedOrTerminated();

		// print workflow
		LOG_INFO(logger, *wfP);
		LOG_INFO(logger, "### END EXECUTION " << workflowfn);

	}
	catch(WorkflowFormatException e) {
		LOG_WARN(logger, "WorkflowFormatException: " << e.message);
	}

	//	//ToDo: return 0, 1, ...
}

void usage(LoggerApi logger) {
	LOG_INFO(logger, "---------------------------------------------------");
	LOG_INFO(logger, "Usage: gwes <GWorkflowDL>");
	LOG_INFO(logger, "<GWorkflowDL>: Filename of workflow to invoke.");
	LOG_INFO(logger, "---------------------------------------------------");
}

string getUserName() {
	char* userNameP = getenv("USER");
	if (userNameP != NULL) {
		string userName(userNameP);
		if (userName.size() > 0) {
			return userName;
		}
	}
	return "NN";
}

