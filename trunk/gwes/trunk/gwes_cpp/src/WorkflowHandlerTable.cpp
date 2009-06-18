/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
// std
#include <iostream>
#include <sstream>
//gwes
#include "WorkflowHandlerTable.h"

using namespace std;

namespace gwes {

/**
 * Constructor.
 */
WorkflowHandlerTable::WorkflowHandlerTable() {
}

/**
 * Destructor.
 */
WorkflowHandlerTable::~WorkflowHandlerTable() {
	for (map<string,WorkflowHandler*>::iterator it=begin(); it!=end(); ++it) {
		WorkflowHandler* wfhP = it->second;
		delete wfhP;
	}
	clear();
}

string WorkflowHandlerTable::put(WorkflowHandler* wfhP) {
	string id = wfhP->getID();
	insert(pair<string, WorkflowHandler*>(id, wfhP));
	return id;
}

WorkflowHandler* WorkflowHandlerTable::get(string id) throw (NoSuchWorkflowException) {
	iterator iter=find(id); 
	if (iter==end()) {
	     // no such workflow
	     ostringstream message; 
		 message << "Workflow with id=\"" << id << "\" is not available!";
	     throw NoSuchWorkflowException(message.str());
	}
	return iter->second;
}

void WorkflowHandlerTable::remove(string id) {
	iterator iter=find(id);
	if (iter==end()) {
	     // no such workflow
	     ostringstream message; 
		 message << "Workflow with id=\"" << id << "\" is not available!";
	     throw NoSuchWorkflowException(message.str());
	}
	delete iter->second;
	erase(id);
}

}
