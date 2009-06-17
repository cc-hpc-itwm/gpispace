/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#include <iostream>
#include <sstream>

#include <xercesc/dom/DOM.hpp>
#include <xercesc/util/XMLString.hpp>
#include <xercesc/util/PlatformUtils.hpp>

#include <assert.h>
#include "../gworkflowdl_cpp/src/OperationCandidate.h"
#include "../gworkflowdl_cpp/src/Workflow.h"
#include "../gworkflowdl_cpp/src/XMLUtils.h"

using namespace std;
using namespace gwdl;
XERCES_CPP_NAMESPACE_USE

void testParser()
{
	cout << "============== test PARSER =============" << endl;
	Workflow *wf = new Workflow();
	wf->setID("test_workflow");

	// description
	cout << "  description..." << endl;
	wf->setDescription("This is the description of the workflow");
	assert(wf->getDescription()=="This is the description of the workflow") ;

	// properties
	cout << "  properties..." << endl,
	wf->getProperties().put("b_name1","value1");	
	wf->getProperties().put("a_name2","value2");	
	assert(wf->getProperties().get("b_name1")=="value1");
	
	// places
	cout << "  places..." << endl;
	Place* p0 = new Place("p0");
	Place* p1 = new Place("p1");
	wf->addPlace(p0);
	wf->addPlace(p1);
	assert(wf->placeCount()==2);
	
	// transition
	cout << "  transition..." << endl;
	Transition* t0 = new Transition("t0");
	t0->getProperties().put("t0-key","t0-value");
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
	Token* d1 = new Token(true);
	wf->getPlace("p0")->addToken(d1);

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
	OperationCandidate* opcand = new OperationCandidate();
	opcand->setType("psp");
	opcand->setOperationName("alg-mean-value");
	opcand->setResourceName("phastgrid");
	opcand->setSelected(true);
	wf->getTransition("t0")->getOperation()->getOperationClass()->addOperationCandidate(opcand);
	
	DOMDocument* doc = wf->toDocument();
	string* s = XMLUtils::Instance()->serialize(doc,true);
	
	// print workflow to stdout
	cout << "workflow in:" << endl;
    cout << *s << endl;
    
    cout << "workflow in parsing" << endl;
    DOMElement* n = (XMLUtils::Instance()->deserialize(*s))->getDocumentElement();
    //cout << "wf element: \n" << *(XMLUtils::Instance()->serialize((DOMNode*)n, true)) << endl;
    Workflow* wf1 = new Workflow(n);
    
    cout << "workflow out to Document" << endl;
    DOMDocument* doc2 = wf1->toDocument();
    string* s2 = XMLUtils::Instance()->serialize(doc2,true);
    
    cout << "workflow out:" << endl;
    cout << *s2 << endl;
    assert(*s == *s2);
    
	delete wf;
	delete wf1;
	
	cout << "============== END test PARSER =============" << endl;
	
}

