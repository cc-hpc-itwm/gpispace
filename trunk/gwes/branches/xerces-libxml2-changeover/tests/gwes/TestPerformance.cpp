// std
#include <iostream>
#include <fstream>
#include <sstream>
// tests
#include "TestPerformance.h"
#include "SdpaDummy.h"
// gwes
#include <gwes/Utils.h>
#include <gwes/WorkflowObserver.h>
// gwdl
#include <gwdl/XMLUtils.h>
#include <gwdl/Libxml2Builder.h>
//fhglog
#include <fhglog/fhglog.hpp>
// std
#include <vector>

using namespace std;
using namespace fhg::log;
using namespace gwdl;
using namespace gwes;
using namespace gwes::tests;

CPPUNIT_TEST_SUITE_REGISTRATION( gwes::tests::TestPerformance );

TestPerformance::TestPerformance() 
{
}

TestPerformance::~TestPerformance() 
{
}

//////////////////////////////////////////
// Test methods
//////////////////////////////////////////

/**
 * control-loop.gwdl
 */
void TestPerformance::testWorkflowControlLoop() 
{
	logger_t logger(getLogger("gwes"));
	LOG_INFO(logger, "============== TestPerformance::testWorkflowControlLoop() ... ==============");
	_loggerShutup();

	Workflow::ptr_t workflow;
	CPPUNIT_ASSERT_NO_THROW(workflow = _testWorkflow(Utils::expandEnv("${GWES_CPP_HOME}/workflows/test/control-loop_10000.gwdl"),m_gwes));
	CPPUNIT_ASSERT(m_gwes.getStatusAsString(workflow)=="COMPLETED");
	Token::ptr_t tokenP = workflow->getPlace("end")->getTokens()[0];
	CPPUNIT_ASSERT_EQUAL(string("<a>10000</a>"), tokenP->getData()->getContent());
	m_gwes.remove(workflow->getID());
	
	_loggerAsBefore();
	LOG_INFO(logger, "workflow.duration.expected = 3s (3.0s) (r1504 on poseidon)");
}

void TestPerformance::testManySimpleWorkflows() {
	logger_t logger(getLogger("gwes"));
	LOG_INFO(logger, "============== TestPerformance::testManySimpleWorkflows() ... ==============");
	_loggerShutup();

	Libxml2Builder builder;
	Workflow::ptr_t wfMasterP = builder.deserializeWorkflowFromFile(Utils::expandEnv("${GWES_CPP_HOME}/workflows/test/simple.gwdl"));
	
	_measureBefore();

	const int imax = 200;
	for (int i=0; i<imax; i++) {
		// make copy of workflow object
		string wfStr = builder.serializeWorkflow(*wfMasterP);
		Workflow::ptr_t wfP = builder.deserializeWorkflow(wfStr);

		// execute workflow
		string workflowId = _executeWorkflow(wfP, m_gwes);
		
		// deallocate workflowHandler and workflow
		m_gwes.remove(workflowId);
		wfP.reset();
	}

	string duration = _measureAfter();
	_loggerAsBefore();
	LOG_INFO(logger, "workflow.duration["<< imax << " x simple.gwdl] = " << duration);
	LOG_INFO(logger, "workflow.duration.expected = 3s (0s) (r1504 on poseidon)");

	wfMasterP.reset();
}

void TestPerformance::testManyConcurrentWorkflows() {
	logger_t logger(getLogger("gwes"));
	LOG_INFO(logger, "============== TestPerformance::testManyConcurrentWorkflows() ... ==============");
	_loggerShutup();

	Libxml2Builder builder;
	Workflow::ptr_t wfMasterP = builder.deserializeWorkflowFromFile(Utils::expandEnv("${GWES_CPP_HOME}/workflows/test/sleep2s.gwdl"));
	
	_measureBefore();

	// copy and initiate workflows
	vector<string> wfIds;
	size_t imax = 100;
	for (size_t i=0; i<imax; i++) {
		// make copy of workflow object
		string wfStr = builder.serializeWorkflow(*wfMasterP);
		Workflow::ptr_t wfP = builder.deserializeWorkflow(wfStr);
		// initiate workflow
		string workflowId = m_gwes.initiate(wfP,"testManyConcurrentWorkflows");
		wfIds.push_back(workflowId);
	}
	
	CPPUNIT_ASSERT_EQUAL_MESSAGE("Number of workflows", imax, wfIds.size());
	CPPUNIT_ASSERT_EQUAL_MESSAGE("Number of workflows in GWES", imax, m_gwes.getWorkflowIDs().size());
	
	// start workflow
	for (size_t i=0; i<wfIds.size(); i++) {
		m_gwes.start(wfIds[i]);
	}
	
	// wait for end of workflows
	for (size_t i=0; i<wfIds.size(); i++) {
		WorkflowHandler* wfhP = m_gwes.getWorkflowHandlerTable().get(wfIds[i]);
		CPPUNIT_ASSERT_EQUAL_MESSAGE("workflow id", wfIds[i], wfhP->getID());
		WorkflowHandler::status_t exitstatus = _monitorWorkflow(WorkflowHandler::STATUS_INITIATED, wfhP);
		CPPUNIT_ASSERT_EQUAL_MESSAGE(wfIds[i], WorkflowHandler::STATUS_COMPLETED, exitstatus);
	}
		
	// deallocate workflowHandlers
	for (size_t i=0; i<wfIds.size(); i++) {
		m_gwes.remove(wfIds[i]);
	}

	string duration = _measureAfter();
	_loggerAsBefore();
	LOG_INFO(logger, "workflow.duration["<< imax << " x sleep2s.gwdl] = " << duration);
	LOG_INFO(logger, "workflow.duration.expected = 4s (0.1s) (r1504 on poseidon)");

	wfMasterP.reset();
}

void TestPerformance::testWorkflowWithManyActivities() 
{
	logger_t logger(getLogger("gwes"));
	LOG_INFO(logger, "============== TestPerformance::testWorkflowWithManyActivities() ... ==============");
	_loggerShutup();

	Workflow::ptr_t workflow;
	CPPUNIT_ASSERT_NO_THROW(workflow = _testWorkflow(Utils::expandEnv("${GWES_CPP_HOME}/workflows/test/sleep3x2s.gwdl"),m_gwes));
	CPPUNIT_ASSERT(m_gwes.getStatusAsString(workflow)=="COMPLETED");
	
	CPPUNIT_ASSERT_EQUAL_MESSAGE("Number tokens on exitcode", (size_t) 3, workflow->getPlace("exitcode")->getTokens().size());
	m_gwes.remove(workflow->getID());
	
	_loggerAsBefore();
	LOG_INFO(logger, "workflow.duration.expected = 6s (0.01s) (r1504 on poseidon)");
	LOG_WARN(logger, "Concurrent execution of local activities not yet implemented!");
}

/**
 * ToDo: this test works well with 50 concurrent activities, but under eclipse I get an undefined error with 100 activities.
 * Works well with 100 activities when executing "make test" on the command line.
 */
void TestPerformance::testWorkflowWithManySdpaActivities() 
{
	logger_t logger(getLogger("gwes"));
	LOG_INFO(logger, "============== TestPerformance::testWorkflowWithManySdpaActivities() ... ==============");
	_loggerShutup();

	// create SDPA dummy. SDPA dummy creates own local gwes.
	SdpaDummy sdpa;

	// deserialize workflow
	Libxml2Builder builder;
	Workflow::ptr_t wfP = builder.deserializeWorkflowFromFile(Utils::expandEnv("${GWES_CPP_HOME}/workflows/test/parallel-sdpa-test.gwdl"));
	
	_measureBefore();
	
	// start workflow
	workflow_id_t wfId = sdpa.submitWorkflow(wfP);

	// poll for completion of workflow
	int timeout = 100; // 10 Seconds
	SdpaDummy::ogsa_bes_status_t status = sdpa.getWorkflowStatus(wfId);
	while (timeout-- > 0 && (status == SdpaDummy::PENDING || status == SdpaDummy::RUNNING)) {
		usleep(100000);
		status = sdpa.getWorkflowStatus(wfId);
		LOG_INFO(logger, "status " << wfId << " = " << status);
	}
	
	CPPUNIT_ASSERT_EQUAL_MESSAGE("Number tokens on output", (size_t) 50, wfP->getPlace("output")->getTokens().size());
	CPPUNIT_ASSERT_EQUAL_MESSAGE("Exit status of workflow", SdpaDummy::FINISHED, status);
	
	sdpa.removeWorkflow(wfId);
	wfP.reset();
	
	_loggerAsBefore();
	string duration = _measureAfter();
	LOG_INFO(logger, "workflow.duration[parallel-sdpa-test.gwdl] = " << duration);
	LOG_INFO(logger, "workflow.duration.expected = XXXs (XXXs) (r1504 on poseidon)");
}

////////////////////////////////////////////////
// helper methods
// Memory measurement refer to
// rusage, getrusage
// sdpa/daemon/nre/messages.hpp Ping::execute
////////////////////////////////////////////////

Workflow::ptr_t TestPerformance::_testWorkflow(string workflowfn, gwes::GWES &gwes) {
	logger_t logger(getLogger("gwes"));
	
	string file = workflowfn.substr(workflowfn.find_last_of("/")+1);

	LOG_INFO(logger, "executing workflow " << file << " ... ");
	try {
		Libxml2Builder builder;
		Workflow::ptr_t wfP = builder.deserializeWorkflowFromFile(workflowfn);

		// execute workflow
		_measureBefore();
		_executeWorkflow(wfP,gwes);
		string duration = _measureAfter();

		// print workflow
		LOG_DEBUG(logger, *wfP);
		LOG_INFO(logger, "executing workflow " << file << " ... done.");
		LOG_WARN(logger, "workflow.duration[" << file << "] = " << duration);
		return wfP;
	} catch (const WorkflowFormatException &e) {
		LOG_ERROR(logger, "WorkflowFormatException: " << e.what());
		throw;
	}
}

string TestPerformance::_executeWorkflow(Workflow::ptr_t wfP, gwes::GWES &gwes) {
	try {
		// initiate workflow
		string workflowId = gwes.initiate(wfP,"test");

		// start workflow
		gwes.start(workflowId);

		// wait for workflow to end
		WorkflowHandler* wfhP = gwes.getWorkflowHandlerTable().get(workflowId);
		_monitorWorkflow(WorkflowHandler::STATUS_INITIATED, wfhP);
		
		return workflowId;
	} catch (const WorkflowFormatException &e) {
		LOG_ERROR(getLogger("gwes"), "WorkflowFormatException: " << e.what());
		throw;
	}
}

WorkflowHandler::status_t TestPerformance::_monitorWorkflow(WorkflowHandler::status_t oldStatus, WorkflowHandler* wfhP) {
	logger_t logger(getLogger("gwes"));
	WorkflowHandler::status_t status = wfhP->waitForStatusChangeFrom(oldStatus);

	switch(status) {
	case WorkflowHandler::STATUS_UNDEFINED:
	case WorkflowHandler::STATUS_INITIATED:
	case WorkflowHandler::STATUS_RUNNING:
	case WorkflowHandler::STATUS_ACTIVE:
	case WorkflowHandler::STATUS_FAILED:
		LOG_DEBUG(logger, "Transient status [" << wfhP->getID() << "] = " << WorkflowHandler::getStatusAsString(status));
		status = _monitorWorkflow(status, wfhP);
		break;
	case WorkflowHandler::STATUS_SUSPENDED:
	case WorkflowHandler::STATUS_COMPLETED:
	case WorkflowHandler::STATUS_TERMINATED:
		LOG_DEBUG(logger, "Final status [" << wfhP->getID() << "] = " << WorkflowHandler::getStatusAsString(status));
		return status;
	}

	return status;
}

void TestPerformance::_loggerShutup() {
	_oldGwesLevel = getLogger("gwes").getLevel();
	_oldGwdlLevel = getLogger("gwdl").getLevel();
	getLogger("gwes").setLevel(LogLevel::WARN);
	getLogger("gwdl").setLevel(LogLevel::WARN);
}

void TestPerformance::_loggerAsBefore() {
	getLogger("gwes").setLevel(_oldGwesLevel);
	getLogger("gwdl").setLevel(_oldGwdlLevel);
}

void TestPerformance::_measureBefore() {
	_boosttimer.restart();
	_timeBefore = time (NULL);
	getrusage(RUSAGE_SELF, &_usageBefore);
}

string TestPerformance::_measureAfter() {
	double boostduration = _boosttimer.elapsed();
	_timeAfter = time (NULL);
	getrusage(RUSAGE_SELF, &_usageAfter);
	ostringstream oss;
	oss << "time:" << _timeAfter-_timeBefore << "s; boost:" << boostduration << "s" 
		<< "; stack:" << _usageAfter.ru_isrss - _usageBefore.ru_isrss << "kB"
		<< "; data:" << _usageAfter.ru_idrss - _usageBefore.ru_idrss << "kB";
	return oss.str();
}

