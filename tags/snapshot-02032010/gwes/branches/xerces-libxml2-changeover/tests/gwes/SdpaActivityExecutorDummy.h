/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#ifndef SDPAACTIVITYEXECUTORDUMMY_H_
#define SDPAACTIVITYEXECUTORDUMMY_H_
// gwes
#include <gwes/Types.h>
#include <gwes/Sdpa2Gwes.h>
// std
#include <pthread.h>

namespace gwes {
namespace tests {

class SdpaActivityExecutorDummy {

public:

	typedef gwes::Sdpa2Gwes<gwes::TokenParameter> sdpa2gwes_t;

	explicit SdpaActivityExecutorDummy(
			sdpa2gwes_t* gwesP,
			const gwes::activity_id_t &activityId, 
			const gwes::workflow_id_t &workflowId, 
			const std::string& operationName, 
			const std::string& resourceName, 
			gwes::parameter_list_t* parameters);

	virtual ~SdpaActivityExecutorDummy();

	void startActivity();
	void executeActivity();
	void cancelActivity();

private:
	/**
	 * The start workflow thread.
	 */
	pthread_t _thread;

	sdpa2gwes_t* _gwesP;
	gwes::activity_id_t _activityId; 
	gwes::workflow_id_t _workflowId; 
	std::string _operationName; 
	std::string _resourceName; 
	gwes::parameter_list_t* _parameters;

};

} // end namespace tests
} //end namespace gwes

/**
 * This method is used by pthread_create in order to start this activity in an own thread.
 */
void *startActivityAsThread(void *activityP);

#endif /*SDPAACTIVITYEXECUTORDUMMY_H_*/
