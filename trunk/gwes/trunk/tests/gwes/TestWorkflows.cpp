// std
#include <iostream>
#include <fstream>
#include <sstream>
// tests
#include "TestWorkflows.h"
// gwes
#include <gwes/Utils.h>
#include <gwes/WorkflowObserver.h>
// gwdl
#include <gwdl/Libxml2Builder.h>
//fhglog
#include <fhglog/fhglog.hpp>
// std

using namespace std;
using namespace fhg::log;
using namespace gwdl;
using namespace gwes;
using namespace gwes::tests;

CPPUNIT_TEST_SUITE_REGISTRATION( gwes::tests::TestWorkflows );

TestWorkflows::TestWorkflows() 
{
}

TestWorkflows::~TestWorkflows() 
{}

//////////////////////////////////////////
// Test methods
//////////////////////////////////////////

/**
 * simple.gwdl
 */
void TestWorkflows::testWorkflowSimpleGwdl() 
{
	logger_t logger(getLogger("gwes"));
	Workflow::ptr_t workflow;
	CPPUNIT_ASSERT_NO_THROW(workflow = _testWorkflow(Utils::expandEnv("${GWES_CPP_HOME}/workflows/test/simple.gwdl"),m_gwes));
	CPPUNIT_ASSERT(m_gwes.getStatusAsString(workflow)=="COMPLETED");
	CPPUNIT_ASSERT( workflow->readProperties()->get("occurrence.sequence").compare("t") == 0 );
}

/**
 * split-token.gwdl
 */
void TestWorkflows::testWorkflowSplitToken() 
{
	logger_t logger(getLogger("gwes"));
	Workflow::ptr_t workflow;

	CPPUNIT_ASSERT_NO_THROW(workflow = _testWorkflow(Utils::expandEnv("${GWES_CPP_HOME}/workflows/test/split-token.gwdl"),m_gwes));
	CPPUNIT_ASSERT(m_gwes.getStatusAsString(workflow)=="COMPLETED");
	CPPUNIT_ASSERT( workflow->readProperties()->get("occurrence.sequence").compare("joinSplitTokens") == 0 );

	gwdl::Place::ptr_t placeP = workflow->getPlace("value"); 
	CPPUNIT_ASSERT(placeP->getTokenNumber() == 1);
	gwdl::Token::ptr_t tokenP = placeP->getTokens().front();
	CPPUNIT_ASSERT(tokenP->isData());
	LOG_INFO(logger, *(tokenP->getData()));
	CPPUNIT_ASSERT_EQUAL(string("<value><x>15</x><y>23</y></value>\n  <value><x>16</x><y>24</y></value>"),
			tokenP->getData()->getContent());

	placeP = workflow->getPlace("x"); 
	CPPUNIT_ASSERT(placeP->getTokenNumber() == 1);
	tokenP = placeP->getTokens().front();
	CPPUNIT_ASSERT(tokenP->isData());
	LOG_INFO(logger, *(tokenP->getData()));
	CPPUNIT_ASSERT_EQUAL(string("<x>15</x>\n  <x>16</x>"), tokenP->getData()->getContent());

	placeP = workflow->getPlace("y"); 
	CPPUNIT_ASSERT(placeP->getTokenNumber() == 1);
	tokenP = placeP->getTokens().front();
	CPPUNIT_ASSERT(tokenP->isData());
	LOG_INFO(logger, *(tokenP->getData()));
	CPPUNIT_ASSERT_EQUAL(string("<y>23</y>\n  <y>24</y>"), tokenP->getData()->getContent());
}

/**
 * exclusive-choice.gwdl
 */
void TestWorkflows::testWorkflowExclusiveChoice() 
{
	logger_t logger(getLogger("gwes"));
	Workflow::ptr_t workflow;
	CPPUNIT_ASSERT_NO_THROW(workflow = _testWorkflow(Utils::expandEnv("${GWES_CPP_HOME}/workflows/test/exclusive-choice.gwdl"), m_gwes));
	CPPUNIT_ASSERT(m_gwes.getStatusAsString(workflow)=="COMPLETED");
	CPPUNIT_ASSERT( workflow->readProperties()->get("occurrence.sequence").compare("B") == 0 );
	CPPUNIT_ASSERT(workflow->getPlace("end_A")->getTokenNumber() == 0);
	Place::ptr_t placeP = workflow->getPlace("end_B"); 
	CPPUNIT_ASSERT(placeP->getTokenNumber() == 1);
	Token::ptr_t tokenP = placeP->getTokens().front();
	CPPUNIT_ASSERT(!tokenP->isData());
	CPPUNIT_ASSERT(tokenP->getControl());
}

/**
 * condition-test.gwdl
 */
void TestWorkflows::testWorkflowConditionTest() 
{
	logger_t logger(getLogger("gwes"));
	Workflow::ptr_t workflow;
	CPPUNIT_ASSERT_NO_THROW(workflow = _testWorkflow(Utils::expandEnv("${GWES_CPP_HOME}/workflows/test/condition-test.gwdl"), m_gwes));
	CPPUNIT_ASSERT(m_gwes.getStatusAsString(workflow)=="COMPLETED");
	CPPUNIT_ASSERT( workflow->readProperties()->get("occurrence.sequence").compare("B A A") == 0 );

	Place::ptr_t placeP = workflow->getPlace("end_A"); 
	CPPUNIT_ASSERT(placeP->getTokenNumber() == 2);
	Token::ptr_t tokenP = placeP->getTokens()[0];
	CPPUNIT_ASSERT(tokenP->isData());
	LOG_INFO(logger, *(tokenP->getData()));
	CPPUNIT_ASSERT_EQUAL(string("<x>6</x>"), tokenP->getData()->getContent());
	tokenP = placeP->getTokens()[1];
	CPPUNIT_ASSERT(tokenP->isData());
	LOG_INFO(logger, *(tokenP->getData()));
	CPPUNIT_ASSERT_EQUAL(string("<x>7</x>") , tokenP->getData()->getContent());

	placeP = workflow->getPlace("end_B"); 
	CPPUNIT_ASSERT(placeP->getTokenNumber() == 1);
	tokenP = placeP->getTokens()[0];
	CPPUNIT_ASSERT(tokenP->isData());
	LOG_INFO(logger, *(tokenP->getData()));
	CPPUNIT_ASSERT_EQUAL(string("<x>5</x>"), tokenP->getData()->getContent());
}

/**
 * control-loop.gwdl
 */
void TestWorkflows::testWorkflowControlLoop() 
{
	logger_t logger(getLogger("gwes"));
	Workflow::ptr_t workflow;
	CPPUNIT_ASSERT_NO_THROW(workflow = _testWorkflow(Utils::expandEnv("${GWES_CPP_HOME}/workflows/test/control-loop.gwdl"),m_gwes));
	CPPUNIT_ASSERT(m_gwes.getStatusAsString(workflow)=="COMPLETED");
	CPPUNIT_ASSERT( workflow->readProperties()->get("occurrence.sequence").compare("i_plus_plus i_plus_plus i_plus_plus i_plus_plus i_plus_plus i_plus_plus i_plus_plus i_plus_plus i_plus_plus i_plus_plus break") == 0 );
	Place::ptr_t placeP = workflow->getPlace("end"); 
	CPPUNIT_ASSERT(placeP->getTokenNumber() == 1);
	Token::ptr_t tokenP = placeP->getTokens()[0];
	CPPUNIT_ASSERT(tokenP->isData());
	LOG_INFO(logger, *(tokenP->getData()));
	CPPUNIT_ASSERT_EQUAL(string("<a>10</a>"), tokenP->getData()->getContent());
}

/**
 * Will not work on auto build because shell scripts are not installed there.
 * concatenateIt.gwdl
 */
void TestWorkflows::testWorkflowConcatenateIt() 
{
	logger_t logger(getLogger("gwes"));
	Workflow::ptr_t workflow;
	CPPUNIT_ASSERT_NO_THROW(workflow = _testWorkflow(Utils::expandEnv("${GWES_CPP_HOME}/workflows/test/concatenateIt.gwdl"),m_gwes));
	CPPUNIT_ASSERT(workflow->readProperties()->get("status")=="COMPLETED");
	CPPUNIT_ASSERT(m_gwes.getStatusAsString(workflow)=="COMPLETED");
}

/**
 * Will not work on auto build because shell scripts are not installed there.
 * concatenateIt_fail.gwdl
 */
void TestWorkflows::testWorkflowConcatenateItFail() 
{
	logger_t logger(getLogger("gwes"));
	Workflow::ptr_t workflow;
	CPPUNIT_ASSERT_NO_THROW(workflow = _testWorkflow(Utils::expandEnv("${GWES_CPP_HOME}/workflows/test/concatenateIt_fail.gwdl"),m_gwes));
	CPPUNIT_ASSERT(m_gwes.getStatusAsString(workflow)=="TERMINATED");
}

/**
 * pstm-0.gwdl
 */
void TestWorkflows::testWorkflowPstm0() 
{
	logger_t logger(getLogger("gwes"));
	LOG_WARN(logger, "///ToDo: FIX: testWorkflowPstm0() currently DEACTIVATED (segFault)///");
	Workflow::ptr_t workflow;
//	CPPUNIT_ASSERT_NO_THROW(workflow = _testWorkflow(Utils::expandEnv("${GWES_CPP_HOME}/workflows/pstm-0.gwdl"),m_gwes));
//	CPPUNIT_ASSERT(m_gwes.getStatusAsString(workflow)=="COMPLETED");
//	CPPUNIT_ASSERT(workflow->readProperties()->get("occurrence.sequence").compare("preStackTimeMigration") == 0);
}

////////////////////////////////////////////////
// helper methods
////////////////////////////////////////////////

Workflow::ptr_t TestWorkflows::_testWorkflow(string workflowfn, gwes::GWES &gwes) {
	logger_t logger(getLogger("gwes"));

	LOG_INFO(logger, "============== BEGIN EXECUTION " << workflowfn << "==============");

	try {

		Libxml2Builder builder;
		Workflow::ptr_t wfP = builder.deserializeWorkflowFromFile(workflowfn);

		// initiate workflow
		LOG_INFO(logger, "initiating workflow ...");
		string workflowId = gwes.initiate(wfP,"test");

		// register channel with source observer
		WorkflowObserver* observerP = new WorkflowObserver();
		Channel* channelP = new Channel(observerP);
		gwes.connect(channelP, workflowId);

		// start workflow
		gwes.start(workflowId);

		// wait for workflow to end
		WorkflowHandler* wfhP = gwes.getWorkflowHandlerTable().get(workflowId);
		_monitorWorkflow(WorkflowHandler::STATUS_INITIATED, wfhP);

		// print workflow
		LOG_DEBUG(logger, *wfP);
		LOG_INFO(logger, "============== END EXECUTION " << workflowfn << "==============");
		return wfP;
	} catch (const WorkflowFormatException &e) {
		LOG_ERROR(logger, "WorkflowFormatException: " << e.what());
		throw;
	}
}

WorkflowHandler::status_t TestWorkflows::_monitorWorkflow(WorkflowHandler::status_t oldStatus, WorkflowHandler* wfhP) {
	logger_t logger(getLogger("gwes"));
	WorkflowHandler::status_t status = wfhP->waitForStatusChangeFrom(oldStatus);

	switch(status) {
	case WorkflowHandler::STATUS_UNDEFINED:
	case WorkflowHandler::STATUS_INITIATED:
	case WorkflowHandler::STATUS_RUNNING:
	case WorkflowHandler::STATUS_ACTIVE:
	case WorkflowHandler::STATUS_FAILED:
		LOG_INFO(logger, "Transient status = " << WorkflowHandler::getStatusAsString(status));
		_monitorWorkflow(status, wfhP);
		break;
	case WorkflowHandler::STATUS_SUSPENDED:
	case WorkflowHandler::STATUS_COMPLETED:
	case WorkflowHandler::STATUS_TERMINATED:
		LOG_INFO(logger, "Final status = " << WorkflowHandler::getStatusAsString(status));
		return status;
	}

	return status;
}
