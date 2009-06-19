// std
#include <iostream>
#include <ostream>
#include <unistd.h>
#include <map>
// gwdl
#include "../../gworkflowdl_cpp/src/Data.h"
// test
#include "PreStackPro.h"

using namespace std;
using namespace gwes;

PreStackPro::PreStackPro()
{
}

PreStackPro::~PreStackPro()
{
}

/**
 * This method is called by the WorkflowHandler each time the workflow or an activity changes.
 */
void PreStackPro::update(gwes::Event event)
{
	// logging
	cout << "gwes::PreStackPro::update(" << event._sourceId << "," << event._eventType << "," << event._message ;
	if (event._dataP!=NULL) {
		cout << ",";
		map<string,gwdl::Data*>* dP = event._dataP;
		for (map<string,gwdl::Data*>::iterator it=dP->begin(); it!=dP->end(); ++it) {
			cout << "[" << it->first << "]";
		}
	}
	cout << ")" << endl;
	
	// invocation of algorithm (own thread)
	if (event._eventType == Event::EVENT_ACTIVITY_START) {
		//start algoritm in own thread.
		_data.psp=this;
		_data.event=event;
		pthread_create(&_thread, NULL, executeAlgorithm, (void*)&_data);
	}
}

/**
 * For the back channel communication.
 */
void PreStackPro::setDestinationObserver(Observer* destinationP) {
	_destinationObserverP = destinationP;
}

/**
 * Execute an algorithm. The inputEvent contains the algorithm name and the input data.
 */ 
void PreStackPro::execute(Event& inputEvent) {
	// analyse message (example message="loadTraceHeaders@phastgrid")
	string algName=inputEvent._message.substr(0,inputEvent._message.find_first_of("@"));
	string algResource=inputEvent._message.substr(inputEvent._message.find_first_of("@")+1);
	cout << "algorithm name: " << algName << endl;
	cout << "algorithm resource: " << algResource << endl;
	
	map<string,gwdl::Data*>* inputs = inputEvent._dataP;
	map<string,gwdl::Data*> outputs;
	if (algName=="loadTraceHeaders") {
		// check input data
		for (map<string,gwdl::Data*>::iterator iter=inputs->begin(); iter!=inputs->end(); ++iter) {
			string name = iter->first;
			gwdl::Data* input = iter->second;
			assert(input!=NULL);
			if (name=="file") assert(input->getType()==gwdl::Data::TYPE_FILE);
			else assert(false);
		}
		// simulate execution
		usleep(500000);
		// generate faked output data
		gwdl::Data* outputData = new gwdl::Data("<data><volume>THD_5</volume></data>");
		outputs.insert(pair<string,gwdl::Data*>("thd",outputData));
	} 

	else if (algName=="calcGeoReferenceData") {
		// check input data
		for (map<string,gwdl::Data*>::iterator iter=inputs->begin(); iter!=inputs->end(); ++iter) {
			string name = iter->first;
			gwdl::Data* input = iter->second;
			assert(input!=NULL);
			if (name=="thd") assert(input->getType()==gwdl::Data::TYPE_VOLUME);
			else assert(false);
		}
		// simulate execution
		usleep(1000000);
		// generate faked output data
		gwdl::Data* outputData = new gwdl::Data("<data><volume>GRD_8</volume></data>");
		outputs.insert(pair<string,gwdl::Data*>("grd",outputData));
	} 
	else if (algName=="selectProjectData") {
		// check input data
		for (map<string,gwdl::Data*>::iterator iter=inputs->begin(); iter!=inputs->end(); ++iter) {
			string name = iter->first;
			gwdl::Data* input = iter->second;
			assert(input!=NULL);
			if (name=="grd") assert(input->getType()==gwdl::Data::TYPE_VOLUME);
			else if (name=="parameter") assert(input->getType()==gwdl::Data::TYPE_PARAMETER);
			else assert(false);
		}
		// simulate execution
		usleep(500000);
		// generate faked output data
		gwdl::Data* outputData = new gwdl::Data("<data><volume>FRD_10</volume></data>");
		outputs.insert(pair<string,gwdl::Data*>("frd",outputData));
	} 
	else {
		cerr << "WARNING: algorithm " << algName << " is not implemented nor simulated." << endl;
	}
	// create output event
	gwes::Event outputEvent(inputEvent._sourceId,Event::EVENT_ACTIVITY_END,"COMPLETED",&outputs);
	// notify destination observer
	_destinationObserverP->update(outputEvent);
}

/**
 * This method is invoked using new pthread_create.
 */
void* executeAlgorithm(void* arg) {
	struct PreStackPro::thread_data* data;
	data = (struct PreStackPro::thread_data*) arg;
	PreStackPro* psp = data->psp;
	psp->execute(data->event);
	return 0;
}
