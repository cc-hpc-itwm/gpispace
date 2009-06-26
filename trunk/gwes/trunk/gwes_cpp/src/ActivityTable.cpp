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

ActivityTable::ActivityTable()
{
}

ActivityTable::~ActivityTable()
{
	for (map<string,Activity*>::iterator it=begin(); it!=end(); ++it) {
		Activity* activityP = it->second;
		delete activityP;
	}
	clear();
}

string ActivityTable::put(Activity* activityP) {
	string id = activityP->getID();
	insert(pair<string, Activity*>(id, activityP));
	return id;
}

Activity* ActivityTable::get(const string& id) throw (NoSuchActivityException) {
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
	iterator iter=find(id);
	if (iter==end()) {
	     // no such activity
	     ostringstream message; 
		 message << "Activity with id=\"" << id << "\" is not available!";
	     throw NoSuchActivityException(message.str());
	}
	delete iter->second;
	erase(id);
}

}
