// std
#include <iostream>
#include <fstream>
#include <sstream>
// tests
#include "TestGWES.h"
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
   
    Workflow::ptr_t wf = Workflow::ptr_t(new Workflow(""));
    // p0
    Place::ptr_t p0 = Place::ptr_t(new Place("p0"));
    Token::ptr_t token0 = Token::ptr_t(new Token());
	p0->addToken(token0);
	wf->addPlace(p0);
	// p1
	Place::ptr_t p1 = Place::ptr_t(new Place("p1"));
	wf->addPlace(p1);
	// p2
	Place::ptr_t p2 = Place::ptr_t(new Place("p2"));
	wf->addPlace(p2);
	// p3
	Place::ptr_t p3 = Place::ptr_t(new Place("p3"));
	Data::ptr_t da1 = Data::ptr_t(new Data("<data><param>param</param></data>"));
	Token::ptr_t token1 = Token::ptr_t(new Token(da1));
	p3->addToken(token1);
	wf->addPlace(p3);
	// t0
	Transition::ptr_t t0 = Transition::ptr_t(new Transition("t0"));
	Edge::ptr_t arc0 = Edge::ptr_t(new Edge(Edge::SCOPE_INPUT, wf->getPlace("p0")));
	t0->addEdge(arc0);
	Edge::ptr_t arc1 = Edge::ptr_t(new Edge(Edge::SCOPE_OUTPUT, wf->getPlace("p1")));
    t0->addEdge(arc1);
	wf->addTransition(t0);
	// t1
	Transition::ptr_t t1 = Transition::ptr_t(new Transition("t1"));
	Edge::ptr_t arc2 = Edge::ptr_t(new Edge(Edge::SCOPE_INPUT, wf->getPlace("p1")));
	t1->addEdge(arc2);
	Edge::ptr_t arc4 = Edge::ptr_t(new Edge(Edge::SCOPE_INPUT, wf->getPlace("p3")));
	//arc4->setExpression("input");
	t1->addEdge(arc4);
	Edge::ptr_t arc3 = Edge::ptr_t(new Edge(Edge::SCOPE_OUTPUT, wf->getPlace("p2")));
	t1->addEdge(arc3);
	wf->addTransition(t1);
	// operations
	Operation::ptr_t op = Operation::ptr_t(new Operation());
	wf->getTransition("t1")->setOperation(op);	
	OperationClass::ptr_t opc = OperationClass::ptr_t(new OperationClass());
	opc->setName("date");
	wf->getTransition("t1")->getOperation()->setOperationClass(opc);
	OperationCandidate::ptr_t opcand = OperationCandidate::ptr_t(new OperationCandidate());
	opcand->setType("cli");
	opcand->setOperationName("date");
	opcand->setResourceName("/bin/date");
	opcand->setSelected(true);
	wf->getTransition("t1")->getOperation()->getOperationClass()->addOperationCandidate(opcand);
	
	wf->getProperties()->put("occurrence.sequence","");

	LOG_INFO(logger, "initiate workflow ...");
    GWES m_gwes;
	string id = m_gwes.initiate(wf, "test");
	
	// print workflow to stdout	
	LOG_DEBUG(logger, *wf);
	CPPUNIT_ASSERT(m_gwes.getStatusAsString(id)=="INITIATED");
	
	WorkflowHandlerTable& wfht = m_gwes.getWorkflowHandlerTable();
	CPPUNIT_ASSERT(wfht.get(id)->getID()==id);
	
	LOG_INFO(logger, "execute workflow ...");
	m_gwes.execute(wf);
	// print workflow to stdout	
	LOG_DEBUG(logger, *wf);
	CPPUNIT_ASSERT(m_gwes.getStatusAsString(id)=="COMPLETED");
	Place::ptr_t placeP = wf->getPlace("p2"); 
	CPPUNIT_ASSERT(placeP->getTokenNumber() == 1);
	Token::ptr_t tokenP = placeP->getTokens()[0];
	CPPUNIT_ASSERT(!tokenP->isData());
	CPPUNIT_ASSERT(tokenP->getControl());

    LOG_INFO(logger, "============== END GWES TEST =============");
   
}

