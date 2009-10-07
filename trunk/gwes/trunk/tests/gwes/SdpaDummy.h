/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#ifndef SDPADUMMY_H_
#define SDPADUMMY_H_
// gwes
#include <gwes/Gwes2Sdpa.h>
#include <gwes/Sdpa2Gwes.h>
#include <gwes/Types.h>
// std
#include <map>

class SdpaDummy : public gwes::Gwes2Sdpa
{

public:
	
	enum ogsa_bes_status_t {
	    PENDING = 0,
	    RUNNING = 1,
	    FINISHED = 2,
	    FAILED = 3,
	    CANCELED = 4
	};

	SdpaDummy();
	virtual ~SdpaDummy();
	
	gwes::Sdpa2Gwes* getGwes() { return _gwesP; }
	
	// from interface Gwes2Sdpa
	virtual gwes::activity_id_t submitActivity(gwes::activity_t &activity); 
	virtual void cancelActivity(const gwes::activity_id_t &activityId)  throw (gwes::NoSuchActivityException);
	virtual void workflowFinished(const gwes::workflow_id_t &workflowId) throw (gwes::NoSuchWorkflowException);
	virtual void workflowFailed(const gwes::workflow_id_t &workflowId) throw (gwes::NoSuchWorkflowException);
	virtual void workflowCanceled(const gwes::workflow_id_t &workflowId) throw (gwes::NoSuchWorkflowException);
	
	// helper method
	gwes::workflow_id_t submitWorkflow(gwes::workflow_t &workflow);
	ogsa_bes_status_t getWorkflowStatus(gwes::workflow_id_t workflowId);

private:
	gwes::Sdpa2Gwes* _gwesP;
	std::map<gwes::workflow_id_t,ogsa_bes_status_t> _wfStatusMap;
	void logWorkflowStatus();
	
};

#endif /*SDPADUMMY_H_*/
