// std
#include <iostream>
#include <ostream>
// gwes
#include "WorkflowObserver.h"

using namespace std;

namespace gwes
{

WorkflowObserver::WorkflowObserver()
{
}

WorkflowObserver::~WorkflowObserver()
{
}

void WorkflowObserver::update(Event event)
{
	cout << "gwes::WorkflowObserver::update(" << event._sourceId << "," << event._eventType << "," << event._message ;
	if (event._dataP!=NULL) {
		cout << ",";
		map<string,gwdl::Data*>* dP = event._dataP;
		for (map<string,gwdl::Data*>::iterator it=dP->begin(); it!=dP->end(); ++it) {
			cout << "[" << it->first << "]";
		}
	}
	cout << ")" << endl;
}

}
