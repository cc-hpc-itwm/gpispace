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

class SdpaDummy : public gwes::Gwes2Sdpa
{
private:
	gwes::Sdpa2Gwes* _gwesP;
	
public:

	SdpaDummy();
	virtual ~SdpaDummy();
	
	// from interface Gwes2Sdpa
	virtual gwes::activity_id_t submitActivity(gwes::activity_t &activity); 
	virtual void cancelActivity(const gwes::activity_id_t &activityId)  throw (gwes::NoSuchActivityException);
	virtual void workflowFinished(const gwes::workflow_id_t &workflowId) throw (gwes::NoSuchWorkflowException);
	virtual void workflowFailed(const gwes::workflow_id_t &workflowId) throw (gwes::NoSuchWorkflowException);
	virtual void workflowCanceled(const gwes::workflow_id_t &workflowId) throw (gwes::NoSuchWorkflowException);

};

#endif /*SDPADUMMY_H_*/
