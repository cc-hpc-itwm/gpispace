// std
#include <iostream>
#include <fstream>
#include <sstream>
// tests
#include "TestGWES.h"
// gwes
#include <gwes/Utils.h>
#include <gwes/WorkflowObserver.h>
//fhglog
#include <fhglog/fhglog.hpp>
// std

using namespace std;
using namespace fhg::log;
using namespace gwdl;
using namespace gwes;
 using namespace gwes::tests;

CPPUNIT_TEST_SUITE_REGISTRATION( gwes::tests::GWESTest );

GWESTest::GWESTest() 
{
}

GWESTest::~GWESTest() 
{}

void GWESTest::testGWES() 
{
	logger_t logger(getLogger("gwes"));

	LOG_INFO(logger, "============== BEGIN GWES TEST =============");
	LOG_INFO(logger, "create workflow ...");
   
    Workflow *wf = new Workflow();
    // p0
    Place* p0 = new Place("p0");
    Token* token0 = new Token(true);
	p0->addToken(token0);
	wf->addPlace(p0);
	// p1
	Place* p1 = new Place("p1");
	wf->addPlace(p1);
	// p2
	Place* p2 = new Place("p2");
	wf->addPlace(p2);
	// p3
	Place* p3 = new Place("p3");
	Data* da1 = new Data("<data><param>param</param></data>");
	Token* token1 = new Token(da1);
	p3->addToken(token1);
	wf->addPlace(p3);
	// t0
	Transition* t0 = new Transition("t0");
	Edge* arc0 = new Edge(wf->getPlace("p0"));
	t0->addInEdge(arc0);
	Edge* arc1 = new Edge(wf->getPlace("p1"));
    t0->addOutEdge(arc1);
	wf->addTransition(t0);
	// t1
	Transition* t1 = new Transition("t1");
	Edge* arc2 = new Edge(wf->getPlace("p1"));
	t1->addInEdge(arc2);
	Edge* arc4 = new Edge(wf->getPlace("p3"));
	//arc4->setExpression("input");
	t1->addInEdge(arc4);
	Edge* arc3 = new Edge(wf->getPlace("p2"));
	t1->addOutEdge(arc3);
	wf->addTransition(t1);
	// operations
	Operation* op = new Operation();
	wf->getTransition("t1")->setOperation(op);	
	OperationClass* opc = new OperationClass();
	opc->setName("date");
	wf->getTransition("t1")->getOperation()->setOperationClass(opc);
	OperationCandidate* opcand = new OperationCandidate();
	opcand->setType("cli");
	opcand->setOperationName("date");
	opcand->setResourceName("/bin/date");
	opcand->setSelected(true);
	wf->getTransition("t1")->getOperation()->getOperationClass()->addOperationCandidate(opcand);
	
	wf->getProperties().put("occurrence.sequence","");

	LOG_INFO(logger, "initiate workflow ...");
	string id = m_gwes.initiate(*wf, "test");
	
	// print workflow to stdout	
	LOG_DEBUG(logger, *wf);
	CPPUNIT_ASSERT(m_gwes.getStatusAsString(id)=="INITIATED");
	
	WorkflowHandlerTable& wfht = m_gwes.getWorkflowHandlerTable();
	CPPUNIT_ASSERT(wfht.get(id)->getID()==id);
	
	LOG_INFO(logger, "execute workflow ...");
	m_gwes.execute(*wf);
	// print workflow to stdout	
	LOG_DEBUG(logger, *wf);
	CPPUNIT_ASSERT(m_gwes.getStatusAsString(id)=="COMPLETED");
	Place* placeP = wf->getPlace("p2"); 
	CPPUNIT_ASSERT(placeP->getTokenNumber() == 1);
	Token* tokenP = placeP->getTokens()[0];
	CPPUNIT_ASSERT(!tokenP->isData());
	CPPUNIT_ASSERT(tokenP->getControl());

    LOG_INFO(logger, "============== END GWES TEST =============");
   
}

void GWESTest::testSimpleGwdl() 
{
  logger_t logger(getLogger("gwes"));
  Workflow workflow;

  // simple.gwdl
  CPPUNIT_ASSERT_NO_THROW(workflow = _testWorkflow(Utils::expandEnv("${GWES_CPP_HOME}/workflows/test/simple.gwdl"),m_gwes));
  CPPUNIT_ASSERT(m_gwes.getStatusAsString(workflow)=="COMPLETED");
  CPPUNIT_ASSERT( workflow.getProperties().get("occurrence.sequence").compare("t") == 0 );

  // split-token.gwdl
  CPPUNIT_ASSERT_NO_THROW(workflow = _testWorkflow(Utils::expandEnv("${GWES_CPP_HOME}/workflows/test/split-token.gwdl"),m_gwes));
  CPPUNIT_ASSERT(m_gwes.getStatusAsString(workflow)=="COMPLETED");
  CPPUNIT_ASSERT( workflow.getProperties().get("occurrence.sequence").compare("joinSplitTokens") == 0 );

  gwdl::Place* placeP = workflow.getPlace("value"); 
  CPPUNIT_ASSERT(placeP->getTokenNumber() == 1);
  gwdl::Token* tokenP = placeP->getTokens().front();
  CPPUNIT_ASSERT(tokenP->isData());
  LOG_INFO(logger, *(tokenP->getData()));
  // ToDo: improve pretty printing (too much spaces).
  CPPUNIT_ASSERT(tokenP->getData()->toString()->compare("<data>\n  <value>\n          <x>15</x>\n          <y>23</y>\n        </value>\n  <value>\n          <x>16</x>\n          <y>24</y>\n        </value>\n</data>") == 0);

  placeP = workflow.getPlace("x"); 
  CPPUNIT_ASSERT(placeP->getTokenNumber() == 1);
  tokenP = placeP->getTokens().front();
  CPPUNIT_ASSERT(tokenP->isData());
  LOG_INFO(logger, *(tokenP->getData()));
  CPPUNIT_ASSERT(tokenP->getData()->toString()->compare("<data>\n  <x>15</x>\n  <x>16</x>\n</data>") == 0);

  placeP = workflow.getPlace("y"); 
  CPPUNIT_ASSERT(placeP->getTokenNumber() == 1);
  tokenP = placeP->getTokens().front();
  CPPUNIT_ASSERT(tokenP->isData());
  LOG_INFO(logger, *(tokenP->getData()));
  CPPUNIT_ASSERT(tokenP->getData()->toString()->compare("<data>\n  <y>23</y>\n  <y>24</y>\n</data>") == 0);

  // exclusive-choice.gwdl
  CPPUNIT_ASSERT_NO_THROW(workflow = _testWorkflow(Utils::expandEnv("${GWES_CPP_HOME}/workflows/test/exclusive-choice.gwdl"), m_gwes));
  CPPUNIT_ASSERT(m_gwes.getStatusAsString(workflow)=="COMPLETED");
  CPPUNIT_ASSERT( workflow.getProperties().get("occurrence.sequence").compare("B") == 0 );
  CPPUNIT_ASSERT(workflow.getPlace("end_A")->getTokenNumber() == 0);
  placeP = workflow.getPlace("end_B"); 
  CPPUNIT_ASSERT(placeP->getTokenNumber() == 1);
  tokenP = placeP->getTokens().front();
  CPPUNIT_ASSERT(!tokenP->isData());
  CPPUNIT_ASSERT(tokenP->getControl());

  // condition-test.gwdl
  CPPUNIT_ASSERT_NO_THROW(workflow = _testWorkflow(Utils::expandEnv("${GWES_CPP_HOME}/workflows/test/condition-test.gwdl"), m_gwes));
  CPPUNIT_ASSERT(m_gwes.getStatusAsString(workflow)=="COMPLETED");
  CPPUNIT_ASSERT( workflow.getProperties().get("occurrence.sequence").compare("B A A") == 0 );
		
  placeP = workflow.getPlace("end_A"); 
  CPPUNIT_ASSERT(placeP->getTokenNumber() == 2);
  tokenP = placeP->getTokens()[0];
  CPPUNIT_ASSERT(tokenP->isData());
  LOG_INFO(logger, *(tokenP->getData()));
  CPPUNIT_ASSERT(tokenP->getData()->toString()->compare("<data>\n  <x>6</x>\n</data>") == 0);
  tokenP = placeP->getTokens()[1];
  CPPUNIT_ASSERT(tokenP->isData());
  LOG_INFO(logger, *(tokenP->getData()));
  CPPUNIT_ASSERT(tokenP->getData()->toString()->compare("<data>\n  <x>7</x>\n</data>") == 0);
		
  placeP = workflow.getPlace("end_B"); 
  CPPUNIT_ASSERT(placeP->getTokenNumber() == 1);
  tokenP = placeP->getTokens()[0];
  CPPUNIT_ASSERT(tokenP->isData());
  LOG_INFO(logger, *(tokenP->getData()));
  CPPUNIT_ASSERT(tokenP->getData()->toString()->compare("<data>\n  <x>5</x>\n</data>") == 0);

  // control-loop.gwdl
  CPPUNIT_ASSERT_NO_THROW(workflow = _testWorkflow(Utils::expandEnv("${GWES_CPP_HOME}/workflows/test/control-loop.gwdl"),m_gwes));
  CPPUNIT_ASSERT(m_gwes.getStatusAsString(workflow)=="COMPLETED");
  CPPUNIT_ASSERT( workflow.getProperties().get("occurrence.sequence").compare("i_plus_plus i_plus_plus i_plus_plus i_plus_plus i_plus_plus i_plus_plus i_plus_plus i_plus_plus i_plus_plus i_plus_plus break") == 0 );
  placeP = workflow.getPlace("end"); 
  CPPUNIT_ASSERT(placeP->getTokenNumber() == 1);
  tokenP = placeP->getTokens()[0];
  CPPUNIT_ASSERT(tokenP->isData());
  LOG_INFO(logger, *(tokenP->getData()));
  CPPUNIT_ASSERT(tokenP->getData()->toString()->compare("<data>\n  <a>10</a>\n</data>") == 0);

  // Will not work on auto build because shell scripts are not installed there.
  //   CPPUNIT_ASSERT_NO_THROW(workflow = _testWorkflow(twd + "/concatenateIt.gwdl",m_gwes));
  //   CPPUNIT_ASSERT(workflow.getProperties().get("status")=="COMPLETED");
  //   CPPUNIT_ASSERT(m_gwes.getStatusAsString(workflow)=="COMPLETED");

  //   CPPUNIT_ASSERT_NO_THROW(workflow = _testWorkflow(twd + "/concatenateIt_fail.gwdl",m_gwes));
  //   CPPUNIT_ASSERT(m_gwes.getStatusAsString(workflow)=="TERMINATED");
  //

  // Test of PSTM use case.
  CPPUNIT_ASSERT_NO_THROW(workflow = _testWorkflow(Utils::expandEnv("${GWES_CPP_HOME}/workflows/pstm-0.gwdl"),m_gwes));
  CPPUNIT_ASSERT(m_gwes.getStatusAsString(workflow)=="COMPLETED");
  CPPUNIT_ASSERT(workflow.getProperties().get("occurrence.sequence").compare("preStackTimeMigration") == 0);
}

Workflow& GWESTest::_testWorkflow(string workflowfn, gwes::GWES &gwes) {
  logger_t logger(getLogger("gwdl"));

  LOG_INFO(logger, "============== BEGIN EXECUTION " << workflowfn << "==============");

  try {
    Workflow* wfP = new Workflow(workflowfn);

    // initiate workflow
    LOG_INFO(logger, "initiating workflow ...");
    string workflowId = gwes.initiate(*wfP,"test");

    // register channel with source observer
    WorkflowObserver* observerP = new WorkflowObserver();
    Channel* channelP = new Channel(observerP);
    gwes.connect(channelP, workflowId);

    // start workflow
    gwes.start(workflowId);

    // wait for workflow to end
    WorkflowHandler* wfhP = gwes.getWorkflowHandlerTable().get(workflowId);

    wfhP->waitForStatusChangeToCompletedOrTerminated();
    
    // print workflow
    LOG_DEBUG(logger, *wfP);
    LOG_INFO(logger, "============== END EXECUTION " << workflowfn << "==============");
    return *wfP;
  } catch (WorkflowFormatException e) {
    LOG_WARN(logger, "WorkflowFormatException: " << e.message);
	throw;
  }
}
