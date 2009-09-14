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

void usage();
string getUserName();

int main(int argc, char* argv[]) {
	// logger
	logger_t logger(getLogger("gwes"));
	logger.setLevel(LogLevel::INFO);
	logger.addAppender(Appender::ptr_t(new StreamAppender("console")))->setFormat(Formatter::Short());

	string workflowfn;

	int c;
	while ((c = getopt(argc, argv, "v:")) != -1) {
		switch (c) {
		case 'v':
			// TRACE, DEBUG, INFO, WARN, ERROR
			if (strcmp(optarg,"TRACE") == 0) logger.setLevel(LogLevel::TRACE);
			else if (strcmp(optarg,"DEBUG") == 0) logger.setLevel(LogLevel::DEBUG);
			else if (strcmp(optarg,"INFO") == 0) logger.setLevel(LogLevel::INFO);
			else if (strcmp(optarg,"WARN") == 0) logger.setLevel(LogLevel::WARN);
			else if (strcmp(optarg,"ERROR") == 0) logger.setLevel(LogLevel::ERROR);
			else {
				LOG_FATAL(logger, "Unknown verbose level!");
				usage();
				exit(1);
			}
			break;
		case '?':
			if (optopt == 'v')
				LOG_FATAL(logger, "Option -" << optopt << " requires an argument.");
			else if (isprint(optopt))
				LOG_FATAL(logger, "Unknown option `-" << optopt << "'.");
			else
				LOG_FATAL(logger, "Unknown option character `\\x" << optopt << "'.");
		    usage();
		    exit(1);
		default:
		    usage();
		    exit(1);
		}
	}
	
    for (int index = optind; index < argc; index++) {
       LOG_DEBUG(logger, "Non-option argument " << argv[index]);
       workflowfn = argv[index];
    }
	
	if (workflowfn.empty()) {
		LOG_FATAL(logger, "ERROR: workflow not specified!");
		usage();
		exit(1);
	} 

	try {
		GWES gwes;
		LOG_INFO(logger, "### BEGIN EXECUTION " << workflowfn);
		Workflow* wfP = new Workflow(workflowfn);

		// initiate workflow
		LOG_DEBUG(logger, "initiating workflow ...");
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
		LOG_DEBUG(logger, *wfP);
		LOG_INFO(logger, "### END EXECUTION " << workflowfn);
	}
	catch(WorkflowFormatException e) {
		LOG_ERROR(logger, "WorkflowFormatException: " << e.message);
	}

	//	//ToDo: return 0, 1, ...
}

void usage() {
	cout << "---------------------------------------------------" << endl;
	cout << "Usage: gwes [-v <VERBOSE_LEVEL>] <GWorkflowDL>" << endl;
	cout << "<VERBOSE_LEVEL>: Set the logger level." << endl;
	cout << "                 Valid levels are: TRACE, DEBUG, INFO, WARN, ERROR." << endl;
	cout << "<GWorkflowDL>  : Filename of workflow to invoke." << endl;
	cout << "---------------------------------------------------" << endl;
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

