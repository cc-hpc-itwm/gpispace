/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#include <iostream>
#include <sstream>
#include <assert.h>
#include "../gworkflowdl_cpp/src/OperationCandidate.h"
#include "TestTransition.h"

using namespace std;
using namespace gwdl;
 
void testTransition() 
{
   cout << "============== BEGIN TRANSITION TEST =============" << endl;
   
   cout << "test empty transition..." << endl;
   Transition *t0 = new Transition("");
   cout << *t0 << endl;
   
   cout << "test description..." << endl;
   t0->setDescription("This is the description of the transition"); 
   assert(t0->getDescription()=="This is the description of the transition");
		
   cout << "test transition connected to four places..." << endl;
   Place *p0 = new Place("");
   Place *p1 = new Place("");
   Place *p2 = new Place("");
   Place *p3 = new Place("");
   Edge *e0 = new Edge(p0,"input0");
   Edge *e1 = new Edge(p1,"input1");
   Edge *e2 = new Edge(p2,"output0");
   Edge *e3 = new Edge(p3,"output1");
   t0->addReadEdge(e0);
   t0->addInEdge(e1);
   t0->addWriteEdge(e2);
   t0->addOutEdge(e3);
   
   cout << "test properties..." << endl;
   t0->getProperties().put("key1","value1");
   
   cout << "test condition..." << endl;
   t0->addCondition("true");
   
   // this is still a control transition (without operation)
   assert(t0->getAbstractionLevel()==Operation::BLACK);

   // link this transtion with an operation
   cout << "test operation class..." << endl;
   Operation* op = new Operation(); 
   OperationClass* opc = new OperationClass();
   opc->setName("calculateEverything");
   op->setOperationClass(opc);
   t0->setOperation(op);
   assert(t0->getAbstractionLevel()==Operation::YELLOW);
     
   OperationCandidate* opcand1 = new OperationCandidate();
   opcand1->setType("psp");
   opcand1->setOperationName("calculate1");
   opcand1->setResourceName("big_machine");
   opcand1->setQuality(0.9);
   opcand1->setSelected();
   t0->getOperation()->getOperationClass()->addOperationCandidate(opcand1);
   assert(t0->getAbstractionLevel()==Operation::GREEN);

   // transition is not enabled
   assert(t0->isEnabled()==false);	
   
   // add read token
   Token* d0 = new Token(true);
   p0->addToken(d0);
   assert(t0->isEnabled()==false);	
   
   // add input token
   Token* d1 = new Token(true);
   p1->addToken(d1);
   assert(t0->isEnabled()==false);	
   
   // add write token
   Token* d2 = new Token(true);
   p2->addToken(d2);
   // transition is now enabled
   assert(t0->isEnabled()==true);	

   cout << *t0 << endl;
   		
   delete t0;
   cout << "============== END TRANSITION TEST =============" << endl;
}
