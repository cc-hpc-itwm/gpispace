/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
// tests
#include "TestWorkflow.h"
//fhglog
#include <fhglog/fhglog.hpp>

using namespace std;
using namespace fhg::log;
using namespace gwdl;
using namespace gwdl::tests;

CPPUNIT_TEST_SUITE_REGISTRATION( gwdl::tests::WorkflowTest );

void WorkflowTest::testWorkflow()
{
	logger_t logger(getLogger("gwdl"));

	LOG_INFO(logger, "============== BEGIN WORKFLOW TEST =============");
	// create empty workflow
	Workflow *wf = new Workflow();
	wf->setID("test-workflow");

	// add description
	LOG_INFO(logger, "  description...");
	wf->setDescription("This is the description of the workflow");
	CPPUNIT_ASSERT(wf->getDescription()=="This is the description of the workflow") ;

	// add properties
	LOG_INFO(logger, "  properties...");
	wf->getProperties().put("b_name1","value1");	
	wf->getProperties().put("a_name2","value2");	
	CPPUNIT_ASSERT(wf->getProperties().get("b_name1")=="value1");
	
	// add places
	LOG_INFO(logger, "  places...");
	Place::ptr_t p0 = Place::ptr_t(new Place("p0"));
	Place::ptr_t p1 = Place::ptr_t(new Place("p1"));
	wf->addPlace(p0);
	wf->addPlace(p1);
	CPPUNIT_ASSERT(wf->placeCount()==2);
	
	// create transition
	LOG_INFO(logger, "  transition...");
	Transition* t0 = new Transition("t0");
	// input edge from p0 to t0
	LOG_INFO(logger, "  input edge...");
	Edge::ptr_t arc0 = Edge::ptr_t(new Edge(wf->getPlace("p0")));
	t0->addInEdge(arc0);
	// output edge from t0 to p1
	LOG_INFO(logger, "  output edge...");
	Edge::ptr_t arc1 = Edge::ptr_t(new Edge(wf->getPlace("p1")));
	t0->addOutEdge(arc1);
	
	// add  transition	
	wf->addTransition(t0);

	// transition is not enabled
	CPPUNIT_ASSERT(wf->getTransition("t0")->isEnabled()==false);	
			
	// add token
	LOG_INFO(logger, "  token...");
	Token::ptr_t d0 = Token::ptr_t(new Token(Token::CONTROL_TRUE));
	wf->getPlace("p0")->addToken(d0);

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
	OperationCandidate* opcand1 = new OperationCandidate();
	opcand1->setType("psp");
	opcand1->setOperationName("alg-mean-value");
	opcand1->setResourceName("phastgrid");
	opcand1->setSelected(true);
	wf->getTransition("t0")->getOperation()->getOperationClass()->addOperationCandidate(opcand1);
	OperationCandidate* opcand2 = new OperationCandidate();
	opcand2->setType("cli");
	opcand2->setOperationName("mean");
	opcand2->setResourceName("/usr/bin/mean");
	wf->getTransition("t0")->getOperation()->getOperationClass()->addOperationCandidate(opcand2);

	// print workflow to stdout	
	LOG_INFO(logger, *wf);
	
	// test no such workflow element getTransition
	bool test = false;
	try 
	{
		wf->getTransition("t1");
	}
	catch (NoSuchWorkflowElement e)
	{	
		LOG_INFO(logger, "NoSuchWorkflowElement: " << e.message);
		test = true;
	}
	CPPUNIT_ASSERT(test);
		
	// test no such workflow element getPlace
	test = false;
	try 
	{
		wf->getPlace("p5");
	}
	catch (NoSuchWorkflowElement e)
	{	
		LOG_INFO(logger, "NoSuchWorkflowElement: " << e.message);
		test = true;
	}
	CPPUNIT_ASSERT(test);

	// test no such workflow element removePlace
	test = false;
	try 
	{
		wf->removePlace(2);
	}
	catch (NoSuchWorkflowElement e)
	{	
		LOG_INFO(logger, "NoSuchWorkflowElement: " << e.message);
		test = true;
	}
	CPPUNIT_ASSERT(test);

	// test no such workflow element removeTransition
	test = false;
	try 
	{
		wf->removeTransition(1);
	}
	catch (NoSuchWorkflowElement e)
	{	
		LOG_INFO(logger, "NoSuchWorkflowElement: " << e.message);
		test = true;
	}
	CPPUNIT_ASSERT(test);
	
	LOG_INFO(logger, "safe to file ...");
	wf->saveToFile("/tmp/wf.xml");
	
	LOG_INFO(logger, "load from file ...");
	Workflow* wf2 = new Workflow("/tmp/wf.xml");
	LOG_INFO(logger, *wf2);

	delete wf;
	delete wf2;
	
	LOG_INFO(logger, "============== END WORKFLOW TEST =============");
	
}
