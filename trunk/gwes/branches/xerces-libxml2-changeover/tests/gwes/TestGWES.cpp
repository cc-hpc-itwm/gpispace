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
    Token* token0 = new Token();
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
	Data::ptr_t da1 = Data::ptr_t(new Data("<data><param>param</param></data>"));
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
    GWES m_gwes;
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

