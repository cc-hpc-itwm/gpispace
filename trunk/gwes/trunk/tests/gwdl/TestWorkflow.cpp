/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#include <iostream>
#include <sstream>
#include <assert.h>
// tests
#include "TestWorkflow.h"

using namespace std;
using namespace gwdl;

void testWorkflow()
{
	cout << "============== BEGIN WORKFLOW TEST =============" << endl;
	// create empty workflow
	Workflow *wf = new Workflow();
	wf->setID("test-workflow");

	// add description
	cout << "  description..." << endl;
	wf->setDescription("This is the description of the workflow");
	assert(wf->getDescription()=="This is the description of the workflow") ;

	// add properties
	cout << "  properties..." << endl,
	wf->getProperties().put("b_name1","value1");	
	wf->getProperties().put("a_name2","value2");	
	assert(wf->getProperties().get("b_name1")=="value1");
	
	// add places
	cout << "  places..." << endl;
	Place* p0 = new Place("p0");
	Place* p1 = new Place("p1");
	wf->addPlace(p0);
	wf->addPlace(p1);
	assert(wf->placeCount()==2);
	
	// create transition
	cout << "  transition..." << endl;
	Transition* t0 = new Transition("t0");
	// input edge from p0 to t0
	cout << "  input edge..." << endl;
	Edge* arc0 = new Edge(wf->getPlace("p0"));
	t0->addInEdge(arc0);
	// output edge from t0 to p1
	cout << "  output edge..." << endl;
	Edge* arc1 = new Edge(wf->getPlace("p1"));
	t0->addOutEdge(arc1);
	
	// add  transition	
	wf->addTransition(t0);

	// transition is not enabled
	assert(wf->getTransition("t0")->isEnabled()==false);	
			
	// add token
	cout << "  token..." << endl;
	Token* d0 = new Token(true);
	wf->getPlace("p0")->addToken(d0);

	// transition is now enabled
	assert(wf->getTransition("t0")->isEnabled()==true);
	
	// add operation to transition
	cout << "  operation..." << endl;
	cout << "  set operation..." << endl;
	Operation* op = new Operation();
	wf->getTransition("t0")->setOperation(op);	
	cout << "  set operation class..." << endl;
	OperationClass* opc = new OperationClass();
	opc->setName("mean-value");
	wf->getTransition("t0")->getOperation()->setOperationClass(opc);
	cout << "  add operation candidate..." << endl;
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
	cout << *wf << endl;
	
	// test no such workflow element getTransition
	bool test = false;
	try 
	{
		wf->getTransition("t1");
	}
	catch (NoSuchWorkflowElement e)
	{	
		cout << "NoSuchWorkflowElement: " << e.message << endl;
		test = true;
	}
	assert(test);
		
	// test no such workflow element getPlace
	test = false;
	try 
	{
		wf->getPlace("p5");
	}
	catch (NoSuchWorkflowElement e)
	{	
		cout << "NoSuchWorkflowElement: " << e.message << endl;
		test = true;
	}
	assert(test);

	// test no such workflow element removePlace
	test = false;
	try 
	{
		wf->removePlace(2);
	}
	catch (NoSuchWorkflowElement e)
	{	
		cout << "NoSuchWorkflowElement: " << e.message << endl;
		test = true;
	}
	assert(test);

	// test no such workflow element removeTransition
	test = false;
	try 
	{
		wf->removeTransition(1);
	}
	catch (NoSuchWorkflowElement e)
	{	
		cout << "NoSuchWorkflowElement: " << e.message << endl;
		test = true;
	}
	assert(test);
	
	cout << "safe to file ..." << endl;
	wf->saveToFile("/tmp/wf.xml");
	
	cout << "load from file ..." << endl;
	Workflow* wf2 = new Workflow("/tmp/wf.xml");
	cout << *wf2 << endl;

	delete wf;
	delete wf2;
	
	cout << "============== END WORKFLOW TEST =============" << endl;
	
}
