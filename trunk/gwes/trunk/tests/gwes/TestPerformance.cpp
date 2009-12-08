// std
#include <iostream>
#include <fstream>
#include <sstream>
// tests
#include "TestPerformance.h"
// gwes
#include <gwes/Utils.h>
#include <gwes/WorkflowObserver.h>
// gwdl
#include <gwdl/XMLUtils.h>
#include <gwdl/Libxml2Builder.h>
//fhglog
#include <fhglog/fhglog.hpp>
// boost
#include <boost/timer.hpp>
// std
#include <time.h>
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
	
	time_t before = time (NULL);
	boost::timer t;

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

	time_t after = time (NULL);
	double duration = t.elapsed();

	_loggerAsBefore();
	LOG_INFO(logger, "workflow.duration["<< imax << " x simple.gwdl] = " << after-before << "s (" << duration << "s)");
	LOG_INFO(logger, "workflow.duration.expected = 3s (0s) (r1504 on poseidon)");

	wfMasterP.reset();
}

void TestPerformance::testManyConcurrentWorkflows() {
	logger_t logger(getLogger("gwes"));
	LOG_INFO(logger, "============== TestPerformance::testManyConcurrentWorkflows() ... ==============");
	_loggerShutup();

	Libxml2Builder builder;
	Workflow::ptr_t wfMasterP = builder.deserializeWorkflowFromFile(Utils::expandEnv("${GWES_CPP_HOME}/workflows/test/sleep2s.gwdl"));
	
	// start timers
	time_t before = time (NULL);
	boost::timer t;

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

	// end timers
	time_t after = time (NULL);
	double duration = t.elapsed();

	_loggerAsBefore();
	LOG_INFO(logger, "workflow.duration["<< imax << " x sleep2s.gwdl] = " << after-before << "s (" << duration << "s)");
	LOG_INFO(logger, "workflow.duration.expected = 4s (0.1s) (r1504 on poseidon)");

	wfMasterP.reset();
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
		time_t before = time (NULL);
		boost::timer t;
		_executeWorkflow(wfP,gwes);
		time_t after = time (NULL);
		double duration = t.elapsed();

		// print workflow
		LOG_DEBUG(logger, *wfP);
		LOG_INFO(logger, "executing workflow " << file << " ... done.");
		LOG_WARN(logger, "workflow.duration[" << file << "] = " << after-before << "s (" << duration << "s)");
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
