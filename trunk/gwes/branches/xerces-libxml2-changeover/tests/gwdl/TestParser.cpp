/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#include <iostream>
#include <sstream>

// gwdl
#include <gwdl/Workflow.h>
#include <gwdl/Libxml2Builder.h>
//tests
#include "TestParser.h"
//fhglog
#include <fhglog/fhglog.hpp>

using namespace std;
using namespace gwdl;
using namespace fhg::log;
using namespace gwdl::tests;

CPPUNIT_TEST_SUITE_REGISTRATION( gwdl::tests::ParserTest );

void ParserTest::testParser()
{
	logger_t logger(getLogger("gwdl"));
	LOG_INFO(logger, "============== test PARSER =============");
	Workflow::ptr_t wf = Workflow::ptr_t(new Workflow("test_workflow"));

	// description
	LOG_INFO(logger, "  description...");
	wf->setDescription("This is the description of the workflow");
	CPPUNIT_ASSERT(wf->getDescription()=="This is the description of the workflow") ;

	// properties
	LOG_INFO(logger, "  properties...");
	wf->getProperties()->put("b_name1","value1");	
	wf->getProperties()->put("a_name2","value2");	
	CPPUNIT_ASSERT(wf->getProperties()->get("b_name1")=="value1");
	
	// places
	LOG_INFO(logger, "  places...");
	Place::ptr_t p0 = Place::ptr_t(new Place("p0"));
	Place::ptr_t p1 = Place::ptr_t(new Place("p1"));
	wf->addPlace(p0);
	wf->addPlace(p1);
	CPPUNIT_ASSERT(wf->placeCount()==2);
	
	// transition
	LOG_INFO(logger, "  transition...");
	Transition::ptr_t t0 = Transition::ptr_t(new Transition("t0"));
	t0->getProperties()->put("t0-key","t0-value");
	// input edge from p0 to t0
	LOG_INFO(logger, "  input edge...");
	Edge::ptr_t arc0 = Edge::ptr_t(new Edge(Edge::SCOPE_INPUT, wf->getPlace("p0")));
	t0->addEdge(arc0);
	// output edge from t0 to p1
	LOG_INFO(logger, "  output edge...");
	Edge::ptr_t arc1 = Edge::ptr_t(new Edge(Edge::SCOPE_OUTPUT, wf->getPlace("p1")));
	t0->addEdge(arc1);
	// add  transition	
	wf->addTransition(t0);

	// transition is not enabled
	CPPUNIT_ASSERT(wf->getTransition("t0")->isEnabled()==false);	
			
	// add token
	LOG_INFO(logger, "  token...");
	Token::ptr_t d0 = Token::ptr_t(new Token());
	wf->getPlace("p0")->addToken(d0);
	Token::ptr_t d1 = Token::ptr_t(new Token());
	wf->getPlace("p0")->addToken(d1);

	// transition is now enabled
	CPPUNIT_ASSERT(wf->getTransition("t0")->isEnabled()==true);
	
	// add operation to transition
	LOG_INFO(logger, "  operation...");
	LOG_INFO(logger, "  set operation...");
	Operation::ptr_t op = Operation::ptr_t(new Operation());
	wf->getTransition("t0")->setOperation(op);	
	LOG_INFO(logger, "  set operation class...");
	OperationClass::ptr_t opc = OperationClass::ptr_t(new OperationClass());
	opc->setName("mean-value");
	wf->getTransition("t0")->getOperation()->setOperationClass(opc);
	LOG_INFO(logger, "  add operation candidate...");
	OperationCandidate::ptr_t opcand = OperationCandidate::ptr_t(new OperationCandidate());
	opcand->setType("psp");
	opcand->setOperationName("alg-mean-value");
	opcand->setResourceName("phastgrid");
	opcand->setSelected(true);
	wf->getTransition("t0")->getOperation()->getOperationClass()->addOperationCandidate(opcand);
	
	LOG_WARN(logger, "///ToDo: Migrate to libxml2///");
	
//	DOMDocument* doc = wf->toDocument();
//	string* s = XMLUtils::Instance()->serialize(doc,true);
//	
//	// print workflow to stdout
//	LOG_INFO(logger, "workflow in:");
//    LOG_INFO(logger, *s);
//    
//    LOG_INFO(logger, "workflow in parsing");
//    DOMElement* n = (XMLUtils::Instance()->deserialize(*s))->getDocumentElement();
//    //LOG_INFO(logger, "wf element: \n" << *(XMLUtils::Instance()->serialize((DOMNode*)n, true)));
//    Workflow::ptr_t wf1 = Workflow::ptr_t(new Workflow(n);
//    
//    LOG_INFO(logger, "workflow out to Document");
//    DOMDocument* doc2 = wf1->toDocument();
//    string* s2 = XMLUtils::Instance()->serialize(doc2,true);
//    
//    LOG_INFO(logger, "workflow out:");
//    LOG_INFO(logger, *s2);
//    CPPUNIT_ASSERT(*s == *s2);
//    
//	delete wf;
//	delete wf1;
	
	LOG_INFO(logger, "============== END test PARSER =============");
	
}

