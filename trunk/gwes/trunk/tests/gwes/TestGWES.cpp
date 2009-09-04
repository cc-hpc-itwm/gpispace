// tests
#include "TestGWES.h"
// gwes
#include <gwes/Utils.h>
// std
#include <iostream>
#include <sstream>
#include <assert.h>

using namespace std;
using namespace gwdl;
using namespace gwes;
 
void testGWES(GWES &gwes) 
{
	cout << "============== BEGIN GWES TEST =============" << endl;
	cout << "create workflow ..." << endl;
   
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

	cout << "initiate workflow ..." << endl;
	string id = gwes.initiate(*wf, "test");
	
	// print workflow to stdout	
	cout << *wf << endl;
	assert(gwes.getStatusAsString(id)=="INITIATED");
	
	WorkflowHandlerTable& wfht = gwes.getWorkflowHandlerTable();
	assert(wfht.get(id)->getID()==id);
	
	cout << "execute workflow ..." << endl;
	gwes.execute(*wf);
	// print workflow to stdout	
	cout << *wf << endl;
	assert(gwes.getStatusAsString(id)=="COMPLETED");
	Place* placeP = wf->getPlace("p2"); 
	assert(placeP->getTokenNumber() == 1);
	Token* tokenP = placeP->getTokens()[0];
	assert(!tokenP->isData());
	assert(tokenP->getControl());

    cout << "============== END GWES TEST =============" << endl;
   
}
