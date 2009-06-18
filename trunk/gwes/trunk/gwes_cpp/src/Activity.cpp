//std
#include <iostream>
#include <sstream>
#include <unistd.h>
//gwes
#include "Activity.h"
#include "WorkflowHandler.h"

using namespace std;

namespace gwes {

Activity::Activity(WorkflowHandler* handler, string activityImpl, gwdl::OperationCandidate* operation) {
	_status=STATUS_UNDEFINED;
	_wfhP = handler;
	_activityImpl = activityImpl;
	_id = handler->getNewActivityID();
	_operation = operation;
	_abort = false;
	_suspend = false;
}

Activity::~Activity() {
}

void Activity::setStatus(int status) {
	if (status == _status) return; 
//	int oldStatus = _status;
	_status = status;
//	cout << "gwes::Activity: status of activity \"" << _id
//			<< "\" changed from " << getStatusAsString(oldStatus) << " to "
//			<< getStatusAsString() << endl;
	
	// notify observers
	if (_observers.size()>0) {
		Event event(_id,Event::EVENT_ACTIVITY,getStatusAsString());
		for (unsigned int i = 0; i<_observers.size(); i++ ) {
			_observers[i]->update(event);
		}
	}
}

string Activity::getStatusAsString(int status) {
	switch (status) {
	case (STATUS_UNDEFINED): return "UNDEFINED";
	case (STATUS_RUNNING): return "RUNNING";
	case (STATUS_INITIATED): return "INITIATED";
	case (STATUS_SUSPENDED): return "SUSPENDED";
	case (STATUS_ACTIVE): return "ACTIVE";
	case (STATUS_TERMINATED): return "TERMINATED";
	case (STATUS_COMPLETED): return "COMPLETED";
	case (STATUS_FAILED): return "FAILED";
	default:
		cerr << "gwes::Activity: Unknown activity status code: " << status << endl;
		return "";
	}
}

int Activity::waitForStatusChangeFrom(int oldStatus) {
    while (_status == oldStatus) {
    	// ToDo: replace by monitor waiting.
    	usleep(_wfhP->getSleepTime());
    }
    return _status;
}

void Activity::waitForStatusChangeTo(int newStatus) {
    while (_status != newStatus) {
    	// ToDo: replace by monitor waiting.
    	usleep(_wfhP->getSleepTime());
    }
}

void Activity::waitForStatusChangeToCompletedOrTerminated() {
    while (_status != STATUS_COMPLETED && _status != STATUS_TERMINATED) {
    	// ToDo: replace by monitor waiting.
    	usleep(_wfhP->getSleepTime());
    }
}

void Activity::attachObserver(Observer* observerP) {
	_observers.push_back(observerP);
}

// notify observers
void Activity::notifyObservers(int type, string message,map<string,gwdl::Data*>* data) {
	Event event(_id,type,message,data);
	for (unsigned int i = 0; i<_observers.size(); i++ ) {
		_observers[i]->update(event);
	}
}

} // end namespace gwes
