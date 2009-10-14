/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#ifndef TESTTRANSFORMATOMICACTIVITYTOWORKFLOW_H_
#define TESTTRANSFORMATOMICACTIVITYTOWORKFLOW_H_
// gwes
#include <gwes/Types.h>
#include <gwes/Gwes2Sdpa.h>
#include <gwes/Sdpa2Gwes.h>
//cppunit
#include <cppunit/extensions/HelperMacros.h>

namespace gwes {
namespace tests {

class TestTransformAtomicActivityToWorkflow : public gwes::Gwes2Sdpa, public CPPUNIT_NS::TestFixture 
{
	CPPUNIT_TEST_SUITE( gwes::tests::TestTransformAtomicActivityToWorkflow );
	CPPUNIT_TEST( testTransform );
	CPPUNIT_TEST_SUITE_END();

public:
	typedef gwes::Sdpa2Gwes<gwes::TokenParameter> sdpa2gwes_t;
	TestTransformAtomicActivityToWorkflow();
	virtual ~TestTransformAtomicActivityToWorkflow();

	// from interface Gwes2Sdpa
	virtual gwes::activity_id_t submitActivity(gwes::activity_t &activity); 
	virtual void cancelActivity(const gwes::activity_id_t &activityId)  throw (NoSuchActivity);
	virtual void workflowFinished(const gwes::workflow_id_t &workflowId) throw (NoSuchWorkflow);
	virtual void workflowFailed(const gwes::workflow_id_t &workflowId) throw (NoSuchWorkflow);
	virtual void workflowCanceled(const gwes::workflow_id_t &workflowId) throw (NoSuchWorkflow);

protected:
	void testTransform();
	sdpa2gwes_t* _gwesP;
	workflow_id_t _workflowId; 
	bool _activityFinished;

};

} // end namespace tests
} // end namespace gwes

#endif /*TESTTRANSFORMATOMICACTIVITYTOWORKFLOW_H_*/
