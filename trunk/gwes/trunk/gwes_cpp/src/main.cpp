/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
//gwes
#include "GWES.h"
#include "WorkflowObserver.h"
#include "Channel.h"

using namespace std;
using namespace gwdl;
using namespace gwes;

void usage();
string getUserName();

int main(int argc, char* argv[]) {

	int c;
	int command=-1;
	string workflowfn;

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
		cerr << "ERROR: workflow not specified!" << endl;
		usage();
		exit(1);
	} else {
		workflowfn = argv[1];
	}

	try {
		GWES gwes;
		cout << "### BEGIN EXECUTION " << workflowfn << endl;
		Workflow* wfP = new Workflow(workflowfn);

		// initiate workflow
		cout << "initiating workflow ..." << endl;
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
		cout << *wfP << endl;
		cout << "### END EXECUTION " << workflowfn << endl;

	}
	catch(WorkflowFormatException e) {
		cerr << "WorkflowFormatException: " << e.message << endl;
	}

	//	//ToDo: return 0, 1, ...
}

void usage() {
	cout << "---------------------------------------------------" << endl;
	cout << "Usage: gwes <GWorkflowDL>" << endl;
	cout << "<GWorkflowDL>: Filename of workflow to invoke." << endl;
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

