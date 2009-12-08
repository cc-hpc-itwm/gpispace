/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#include <iostream>
#include <sstream>

// xerces-c
#include <xercesc/dom/DOM.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/PlatformUtils.hpp>
// gwdl
#include <gwdl/Workflow.h>
#include <gwdl/Place.h>
#include <gwdl/Token.h>
#include <gwdl/XMLUtils.h>
//tests
#include "TestParser.h"
//fhglog
#include <fhglog/fhglog.hpp>

using namespace std;
using namespace gwdl;
using namespace fhg::log;
using namespace gwdl::tests;
XERCES_CPP_NAMESPACE_USE

CPPUNIT_TEST_SUITE_REGISTRATION( gwdl::tests::ParserTest );

void ParserTest::testParser()
{
	logger_t logger(getLogger("gwdl"));
	LOG_INFO(logger, "============== test PARSER =============");
	Workflow *wf = new Workflow();
	wf->setID("test_workflow");

	// description
	LOG_INFO(logger, "  description...");
	wf->setDescription("This is the description of the workflow");
	CPPUNIT_ASSERT(wf->getDescription()=="This is the description of the workflow") ;

	// properties
	LOG_INFO(logger, "  properties...");
	wf->getProperties().put("b_name1","value1");	
	wf->getProperties().put("a_name2","value2");	
	CPPUNIT_ASSERT(wf->getProperties().get("b_name1")=="value1");
	
	// places
	LOG_INFO(logger, "  places...");
	Place* p0 = new Place("p0");
	Place* p1 = new Place("p1");
	wf->addPlace(p0);
	wf->addPlace(p1);
	CPPUNIT_ASSERT(wf->placeCount()==2);
	
	// transition
	LOG_INFO(logger, "  transition...");
	Transition* t0 = new Transition("t0");
	t0->getProperties().put("t0-key","t0-value");
	// input edge from p0 to t0
	LOG_INFO(logger, "  input edge...");
	Edge* arc0 = new Edge(wf->getPlace("p0"));
	t0->addInEdge(arc0);
	// output edge from t0 to p1
	LOG_INFO(logger, "  output edge...");
	Edge* arc1 = new Edge(wf->getPlace("p1"));
	t0->addOutEdge(arc1);
	// add  transition	
	wf->addTransition(t0);

	// transition is not enabled
	CPPUNIT_ASSERT(wf->getTransition("t0")->isEnabled()==false);	
			
	// add token
	LOG_INFO(logger, "  token...");
	Token* d0 = new Token();
	wf->getPlace("p0")->addToken(d0);
	Token* d1 = new Token();
	wf->getPlace("p0")->addToken(d1);

	// transition is now enabled
	CPPUNIT_ASSERT(wf->getTransition("t0")->isEnabled()==true);
	
	// add operation to transition
	LOG_INFO(logger, "  operation...");
	LOG_INFO(logger, "  set operation...");
	Operation* op = new Operation();
	wf->getTransition("t0")->setOperation(op);	
	LOG_INFO(logger, "  set operation class...");
	OperationClass* opc = new OperationClass();
	opc->setName("mean-value");
	wf->getTransition("t0")->getOperation()->setOperationClass(opc);
	LOG_INFO(logger, "  add operation candidate...");
	OperationCandidate* opcand = new OperationCandidate();
	opcand->setType("psp");
	opcand->setOperationName("alg-mean-value");
	opcand->setResourceName("phastgrid");
	opcand->setSelected(true);
	wf->getTransition("t0")->getOperation()->getOperationClass()->addOperationCandidate(opcand);
	
	DOMDocument* doc = wf->toDocument();
	string* s = XMLUtils::Instance()->serialize(doc,true);
	
	// print workflow to stdout
	LOG_INFO(logger, "workflow in:");
    LOG_INFO(logger, *s);
    
    LOG_INFO(logger, "workflow in parsing");
    DOMElement* n = (XMLUtils::Instance()->deserialize(*s))->getDocumentElement();
    //LOG_INFO(logger, "wf element: \n" << *(XMLUtils::Instance()->serialize((DOMNode*)n, true)));
    Workflow* wf1 = new Workflow(n);
    
    LOG_INFO(logger, "workflow out to Document");
    DOMDocument* doc2 = wf1->toDocument();
    string* s2 = XMLUtils::Instance()->serialize(doc2,true);
    
    LOG_INFO(logger, "workflow out:");
    LOG_INFO(logger, *s2);
    CPPUNIT_ASSERT(*s == *s2);
    
	delete wf;
	delete wf1;
	
	LOG_INFO(logger, "============== END test PARSER =============");
	
}

