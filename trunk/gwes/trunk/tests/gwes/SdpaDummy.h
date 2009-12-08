/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#ifndef SDPADUMMY_H_
#define SDPADUMMY_H_
// gwes
#include <gwes/Types.h>
#include <gwes/Gwes2Sdpa.h>
#include <gwes/Sdpa2Gwes.h>
// std
#include <map>

class SdpaDummy : public gwes::Gwes2Sdpa
{

public:
    typedef gwes::Sdpa2Gwes<gwes::TokenParameter> sdpa2gwes_t;
	
	enum ogsa_bes_status_t {
	    PENDING = 0,
	    RUNNING = 1,
	    FINISHED = 2,
	    FAILED = 3,
	    CANCELED = 4
	};

	SdpaDummy();
	virtual ~SdpaDummy();
	
	sdpa2gwes_t *getGwes() { return _gwesP; }
	
	// from interface Gwes2Sdpa
	virtual gwes::activity_id_t submitActivity(gwes::activity_t &activity); 
	virtual void cancelActivity(const gwes::activity_id_t &activityId)  throw (NoSuchActivity);
	virtual void workflowFinished(const gwes::workflow_id_t &workflowId, const gwdl::workflow_result_t &) throw (NoSuchWorkflow);
	virtual void workflowFailed(const gwes::workflow_id_t &workflowId, const gwdl::workflow_result_t &) throw (NoSuchWorkflow);
	virtual void workflowCanceled(const gwes::workflow_id_t &workflowId, const gwdl::workflow_result_t &) throw (NoSuchWorkflow);
	
	// helper method
	gwes::workflow_id_t submitWorkflow(gwes::workflow_t::ptr_t workflowP);
	ogsa_bes_status_t getWorkflowStatus(gwes::workflow_id_t workflowId);
	void removeWorkflow(const gwes::workflow_id_t &workflowId);

private:
	sdpa2gwes_t* _gwesP;
	std::map<gwes::workflow_id_t,ogsa_bes_status_t> _wfStatusMap;
	void logWorkflowStatus();
	void executeSubWorkflow(
			const activity_id_t &activityId, 
			const workflow_id_t &workflowId, 
			gwes::activity_t &activity
			);
	void executeAtomicActivity(
			const gwes::activity_id_t &activityId, 
			const gwes::workflow_id_t &workflowId, 
			const std::string& operationName, 
			const std::string& resourceName, 
			gwes::parameter_list_t* parameters
			);
};

#endif /*SDPADUMMY_H_*/
