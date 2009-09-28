// std
#include <iostream>
#include <ostream>
#include <unistd.h>
#include <map>
// test
#include "PreStackPro.h"
//fhglog
#include <fhglog/fhglog.hpp>

using namespace std;
using namespace fhg::log;
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
void PreStackPro::update(const gwes::Event& event)
{
	// logging
	logger_t logger(getLogger("gwes"));
	LOG_INFO(logger, "gwes::PreStackPro::update(" << event._sourceId << "," << event._eventType << "," << event._message << ")");
	
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
void PreStackPro::execute(const Event& inputEvent) {
	logger_t logger(getLogger("gwes"));
	// analyse message (example message="loadTraceHeaders@phastgrid")
	string algName=inputEvent._message.substr(0,inputEvent._message.find_first_of("@"));
	string algResource=inputEvent._message.substr(inputEvent._message.find_first_of("@")+1);
	LOG_INFO(logger, "algorithm name: " << algName);
	LOG_INFO(logger, "algorithm resource: " << algResource);
	
//	map<string,gwdl::Token*>* inputs = inputEvent._tokensP;
//	map<string,gwdl::Token*> outputs;
//	if (algName=="loadTraceHeaders") {
//		// check input data
//		for (map<string,gwdl::Token*>::iterator iter=inputs->begin(); iter!=inputs->end(); ++iter) {
//			string name = iter->first;
//			gwdl::Token* input = iter->second;
//			assert(input!=NULL);
//			if (name=="file") assert(input->getData()->getType()==gwdl::Data::TYPE_FILE);
//			else assert(false);
//		}
//		// simulate execution
//		usleep(500000);
//		// generate faked output token
//		gwdl::Token* outputToken = new gwdl::Token(new gwdl::Data("<data><volume>THD_5</volume></data>"));
//		outputs.insert(pair<string,gwdl::Token*>("thd",outputToken));
//	} 
//
//	else if (algName=="calcGeoReferenceData") {
//		// check input data
//		for (map<string,gwdl::Token*>::iterator iter=inputs->begin(); iter!=inputs->end(); ++iter) {
//			string name = iter->first;
//			gwdl::Token* input = iter->second;
//			assert(input!=NULL);
//			if (name=="thd") assert(input->getData()->getType()==gwdl::Data::TYPE_VOLUME);
//			else assert(false);
//		}
//		// simulate execution
//		usleep(1000000);
//		// generate faked output data
//		gwdl::Token* outputData = new gwdl::Token(new gwdl::Data("<data><volume>GRD_8</volume></data>"));
//		outputs.insert(pair<string,gwdl::Token*>("grd",outputData));
//	} 
//	else if (algName=="selectProjectData") {
//		// check input data
//		for (map<string,gwdl::Token*>::iterator iter=inputs->begin(); iter!=inputs->end(); ++iter) {
//			string name = iter->first;
//			gwdl::Token* input = iter->second;
//			assert(input!=NULL);
//			if (name=="grd") assert(input->getData()->getType()==gwdl::Data::TYPE_VOLUME);
//			else if (name=="parameter") assert(input->getData()->getType()==gwdl::Data::TYPE_PARAMETER);
//			else assert(false);
//		}
//		// simulate execution
//		usleep(500000);
//		// generate faked output data
//		gwdl::Token* outputToken = new gwdl::Token(new gwdl::Data("<data><volume>FRD_10</volume></data>"));
//		outputs.insert(pair<string,gwdl::Token*>("frd",outputToken));
//	} 
//	else {
//		cerr << "WARNING: algorithm " << algName << " is not implemented nor simulated.");
//	}
//	// create output event
//	gwes::Event outputEvent(inputEvent._sourceId,Event::EVENT_ACTIVITY_END,"COMPLETED",&outputs);
//	// notify destination observer
//	_destinationObserverP->update(outputEvent);
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
