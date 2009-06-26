/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
// gwes
#include <gwes/WorkflowObserver.h>
// std
#include <iostream>
#include <ostream>

using namespace std;

namespace gwes
{

WorkflowObserver::WorkflowObserver()
{
}

WorkflowObserver::~WorkflowObserver()
{
}

void WorkflowObserver::update(const Event& event)
{
	cout << "gwes::WorkflowObserver::update(" << event._sourceId << "," << event._eventType << "," << event._message ;
	if (event._tokensP!=NULL) {
		cout << ",";
		map<string,gwdl::Token*>* dP = event._tokensP;
		for (map<string,gwdl::Token*>::iterator it=dP->begin(); it!=dP->end(); ++it) {
			cout << "[" << it->first << "]";
		}
	}
	cout << ")" << endl;
}

}
