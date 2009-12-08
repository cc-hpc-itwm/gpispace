/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
//gwes
#include <gwes/ActivityTable.h>
// std
#include <iostream>
#include <sstream>

using namespace std;

namespace gwes
{

struct mutex_lock {
	mutex_lock(pthread_mutex_t &mutex) : mtx(mutex) { pthread_mutex_lock(&mtx); }
	~mutex_lock() { pthread_mutex_unlock(&mtx); }
	pthread_mutex_t &mtx;
};

ActivityTable::ActivityTable() {
    pthread_mutex_init(&_monitorLock, NULL);
}

ActivityTable::~ActivityTable() {
	for (map<string,Activity*>::iterator it=begin(); it!=end(); ++it) {
		Activity* activityP = it->second;
		delete activityP;
	}
	clear();
	pthread_mutex_destroy(&_monitorLock);
}

string ActivityTable::put(Activity* activityP) {
    mutex_lock lock(_monitorLock);
	string id = activityP->getID();
	insert(pair<string, Activity*>(id, activityP));
	return id;
}

Activity* ActivityTable::get(const string& id) throw (NoSuchActivityException) {
    mutex_lock lock(_monitorLock);
	iterator iter=find(id); 
	if (iter==end()) {
	     // no such activity
	     ostringstream message; 
		 message << "Activity with id=\"" << id << "\" is not available!";
	     throw NoSuchActivityException(message.str());
	}
	return iter->second;
}

void ActivityTable::remove(const string& id) {
    mutex_lock lock(_monitorLock);
	iterator iter=find(id);
	if (iter==end()) {
	     // no such activity
	     ostringstream message; 
		 message << "Activity with id=\"" << id << "\" is not available!";
	     throw NoSuchActivityException(message.str());
	}
	delete iter->second;
	erase(iter);
}

}
