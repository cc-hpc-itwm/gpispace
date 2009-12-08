/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
// tests
#include "SdpaActivityExecutorDummy.h"
// gwes
#include <gwes/NoSuchWorkflowException.h>
#include <gwes/NoSuchActivityException.h>
//fhglog
#include <fhglog/fhglog.hpp>

using namespace gwes;
using namespace gwdl;
using namespace fhg::log;
using namespace std;
using namespace gwes::tests;

SdpaActivityExecutorDummy::SdpaActivityExecutorDummy(		
		sdpa2gwes_t* gwesP,
		const activity_id_t &activityId, 
		const workflow_id_t &workflowId, 
		const string& operationName, 
		const string& resourceName, 
		parameter_list_t* parameters
	) {
	_gwesP = gwesP;
	_activityId = activityId;
	_workflowId = workflowId;
	_operationName = operationName;
	_resourceName = resourceName;
	_parameters = parameters;
	LOG_DEBUG(logger_t(getLogger("gwes")), "SdpaActivityExecutorDummy() ... ");
}

SdpaActivityExecutorDummy::~SdpaActivityExecutorDummy() {
	LOG_DEBUG(logger_t(getLogger("gwes")), "~SdpaActivityExecutorDummy() ... ");
}

/**
 * A real SDPA implementation should really dispatch the activity asynchronously here.
 */
void SdpaActivityExecutorDummy::startActivity() {
	logger_t logger(getLogger("gwes"));
	LOG_INFO(logger, "starting " << _operationName << " on " << _resourceName);
	
	//start activity in own thread.
	pthread_create(&_thread, NULL, startActivityAsThread, (void*)this);
}

void SdpaActivityExecutorDummy::executeActivity() {
	logger_t logger(getLogger("gwes"));
	LOG_INFO(logger, "executing " << _operationName << " on " << _resourceName);
	
	try {
		// notify gwes that activity has been dispatched
		_gwesP->activityDispatched(_workflowId, _activityId);
		
		// wait some time
		usleep(1000000);
		
		// find and fill dummy output tokens
		// iterate though parameter list, which contains all input/output parameters.
		for (parameter_list_t::iterator it=_parameters->begin(); it!=_parameters->end(); ++it) {
			switch (it->scope) {
			case (TokenParameter::SCOPE_READ):
			case (TokenParameter::SCOPE_INPUT):
			case (TokenParameter::SCOPE_WRITE):
				continue;
			case (TokenParameter::SCOPE_OUTPUT):
				it->tokenP = Token::ptr_t(new Token(Data::ptr_t(new Data(string("<output xmlns=\"\">15</output>")))));
				break;
			}
		}
		
		// notify gwes that activity finished and include parameter list
		_gwesP->activityFinished(_workflowId, _activityId, *_parameters);
	} catch (const NoSuchWorkflowException &e) {
		LOG_ERROR(logger, "exception: " << e.what());
	} catch (const NoSuchActivityException &e) {
		LOG_ERROR(logger, "exception: " << e.what());
	}
}

/**
 * Cancel an atomic activity that has previously been submitted to
 * the SDPA.
 */
void SdpaActivityExecutorDummy::cancelActivity() {
	LOG_ERROR(logger_t(getLogger("gwes")), "cancelActivity(" << _activityId << ")... NOT YET IMPLEMENTED");
	// ToDo: implement!
}

void *startActivityAsThread(void *aP) {
	SdpaActivityExecutorDummy* _aP = (SdpaActivityExecutorDummy*) aP;
	LOG_DEBUG(getLogger("gwes"), "start sdpa activity as new thread...");
	_aP->executeActivity();
	return 0;
}

