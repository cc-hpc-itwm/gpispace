/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
//gwes
#include <gwes/WorkflowHandlerTable.h>
// std
#include <iostream>
#include <sstream>

using namespace std;

namespace gwes {

struct mutex_lock {
	mutex_lock(pthread_mutex_t &mutex) : mtx(mutex) { pthread_mutex_lock(&mtx); }
	~mutex_lock() { pthread_mutex_unlock(&mtx); }
	pthread_mutex_t &mtx;
};

/**
 * Constructor.
 */
WorkflowHandlerTable::WorkflowHandlerTable() {
    pthread_mutex_init(&_monitorLock, NULL);
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
	pthread_mutex_destroy(&_monitorLock);
}

string WorkflowHandlerTable::put(WorkflowHandler* wfhP) {
    mutex_lock lock(_monitorLock);
	string id = wfhP->getID();
	insert(pair<string, WorkflowHandler*>(id, wfhP));
	return id;
}

WorkflowHandler* WorkflowHandlerTable::get(const string& id) throw (NoSuchWorkflowException) {
    mutex_lock lock(_monitorLock);
	iterator iter=find(id); 
	if (iter==end()) {
	     // no such workflow
	     ostringstream message; 
		 message << "Workflow with id=\"" << id << "\" is not available!";
	     throw NoSuchWorkflowException(message.str());
	}
	return iter->second;
}

void WorkflowHandlerTable::remove(const string& id) {
    mutex_lock lock(_monitorLock);
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
