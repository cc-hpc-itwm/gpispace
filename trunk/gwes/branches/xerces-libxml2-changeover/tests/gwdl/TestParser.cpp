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
#include <gwdl/XMLUtils.h>
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
	wf->putProperty("b_name1","value1");	
	wf->putProperty("a_name2","value2");	
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
	t0->putProperty("t0-key","t0-value");
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

	LOG_INFO(logger, "-------------- test serialize/deserialize... --------------");
	Libxml2Builder builder;
	string str = builder.serializeWorkflow(*wf);
	Workflow::ptr_t wf2 = builder.deserializeWorkflow(str);
	string str2 = builder.serializeWorkflow(*wf2);
	LOG_INFO(logger, "original workflow:\n" << *wf);
	LOG_INFO(logger, "serialized/deserialized workflow:\n" << *wf2);
	CPPUNIT_ASSERT_EQUAL_MESSAGE("serialize/deserialize", str, str2);
	
	LOG_INFO(logger, "-------------- test deserialize invalid XML... --------------");
	str = string("<element1>test1</element1>\n<element2>INVALID: ONLY ONE ROOT ELEMENT ALLOWED IN XML!</element2>");
	xmlDocPtr xmlDocP = XMLUtils::Instance()->deserializeLibxml2(str);
	xmlErrorPtr error = xmlGetLastError();
	if (error) {
		LOG_INFO(logger, "XML ERROR (line:" << error->line << "/column:" << error->int2 << "): " << error->message);
		CPPUNIT_ASSERT_EQUAL_MESSAGE("error domain", (int) XML_FROM_PARSER, (int) error->domain);
		xmlResetError(error);
	} else {
		CPPUNIT_ASSERT(false);
	}
	
	LOG_WARN(logger, "Serialize libxml2 DOM document...");
	str = XMLUtils::Instance()->serializeLibxml2Doc(xmlDocP, true);
	LOG_INFO(logger, "\n" << str);
	
	LOG_INFO(logger, "============== END test PARSER =============");

}

