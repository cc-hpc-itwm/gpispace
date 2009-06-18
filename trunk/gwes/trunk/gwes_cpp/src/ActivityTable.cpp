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
#include "ActivityTable.h"

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

Activity* ActivityTable::get(string id) throw (NoSuchActivityException) {
	iterator iter=find(id); 
	if (iter==end()) {
	     // no such activity
	     ostringstream message; 
		 message << "Activity with id=\"" << id << "\" is not available!";
	     throw NoSuchActivityException(message.str());
	}
	return iter->second;
}

void ActivityTable::remove(string id) {
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
