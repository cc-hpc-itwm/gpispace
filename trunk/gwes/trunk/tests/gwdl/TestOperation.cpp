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
#include "TestOperation.h"

using namespace std;
using namespace gwdl;
 
void testOperation() 
{
   cout << "============== BEGIN OPERATION TEST =============" << endl;
   cout << "test red operation..." << endl;
   Operation* op = new Operation();
   cout << *op << endl;
   assert(op->getAbstractionLevel()==Operation::RED);
   
   cout << "test yellow operation..." << endl;
   OperationClass* opc = new OperationClass();
   opc->setName("calculateEverything");
   op->setOperationClass(opc);
   cout << *op << endl;
   assert(op->getAbstractionLevel()==Operation::YELLOW);
   
   cout << "test blue operation..." << endl;
   OperationCandidate* opcan1 = new OperationCandidate();
   opcan1->setType("phastgrid");
   opcan1->setOperationName("calculate1");
   opcan1->setResourceName("big_machine");
   opcan1->setQuality(0.9);
   op->getOperationClass()->addOperationCandidate(opcan1);
   OperationCandidate* opcan2 = new OperationCandidate();
   opcan1->setType("phastgrid");
   opcan2->setOperationName("calculate2");
   opcan2->setResourceName("big_machine");
   op->getOperationClass()->addOperationCandidate(opcan2);
   cout << "  candidate size=" << op->getOperationClass()->getOperationCandidates().size() << endl;
   assert(op->getOperationClass()->getOperationCandidates().size()==2);
   cout << *op << endl; 
   assert(op->getAbstractionLevel()==Operation::BLUE);
   
   cout << "test green operation..." << endl;
   opcan1->setSelected();
   assert(op->getOperationClass()->getOperationCandidates()[0]->isSelected()==true);
   assert(op->getOperationClass()->getOperationCandidates()[0]->getAbstractionLevel()==Operation::GREEN);
   assert(op->getOperationClass()->getOperationCandidates()[1]->isSelected()==false);
   assert(op->getOperationClass()->getOperationCandidates()[1]->getAbstractionLevel()==Operation::BLUE);
   cout << *op << endl; 
   assert(op->getAbstractionLevel()==Operation::GREEN);
   
   delete op;
		
   cout << "============== END OPERATION TEST =============" << endl;
}
