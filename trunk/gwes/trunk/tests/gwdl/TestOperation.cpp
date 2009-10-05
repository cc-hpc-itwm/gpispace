/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#include <iostream>
#include <sstream>
// tests
#include "TestOperation.h"
//fhglog
#include <fhglog/fhglog.hpp>

using namespace std;
using namespace gwdl;
using namespace fhg::log;
using namespace gwdl::tests;

CPPUNIT_TEST_SUITE_REGISTRATION( gwdl::tests::OperationTest );

void OperationTest::testOperation() 
{
	logger_t logger(getLogger("gwdl"));

   LOG_INFO(logger, "============== BEGIN OPERATION TEST =============");
   LOG_INFO(logger, "test red operation...");
   Operation* op = new Operation();
   LOG_INFO(logger, *op);
   CPPUNIT_ASSERT(op->getAbstractionLevel()==AbstractionLevel::RED);
   
   LOG_INFO(logger, "test yellow operation...");
   OperationClass* opc = new OperationClass();
   opc->setName("calculateEverything");
   op->setOperationClass(opc);
   LOG_INFO(logger, *op);
   CPPUNIT_ASSERT(op->getAbstractionLevel()==AbstractionLevel::YELLOW);
   
   LOG_INFO(logger, "test blue operation...");
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
   LOG_INFO(logger, "  candidate size=" << op->getOperationClass()->getOperationCandidates().size());
   CPPUNIT_ASSERT(op->getOperationClass()->getOperationCandidates().size()==2);
   LOG_INFO(logger, *op); 
   CPPUNIT_ASSERT(op->getAbstractionLevel()==AbstractionLevel::BLUE);
   
   LOG_INFO(logger, "test green operation...");
   opcan1->setSelected();
   CPPUNIT_ASSERT(op->getOperationClass()->getOperationCandidates()[0]->isSelected()==true);
   CPPUNIT_ASSERT(op->getOperationClass()->getOperationCandidates()[0]->getAbstractionLevel()==AbstractionLevel::GREEN);
   CPPUNIT_ASSERT(op->getOperationClass()->getOperationCandidates()[1]->isSelected()==false);
   CPPUNIT_ASSERT(op->getOperationClass()->getOperationCandidates()[1]->getAbstractionLevel()==AbstractionLevel::BLUE);
   LOG_INFO(logger, *op); 
   CPPUNIT_ASSERT(op->getAbstractionLevel()==AbstractionLevel::GREEN);
   
   delete op;
		
   LOG_INFO(logger, "============== END OPERATION TEST =============");
}
