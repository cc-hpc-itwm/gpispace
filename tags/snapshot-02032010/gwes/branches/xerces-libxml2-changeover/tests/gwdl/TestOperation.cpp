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
// gwdl
#include <gwdl/Libxml2Builder.h>
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
	LOG_INFO(logger, "-------------- test red operation... --------------");
	Operation::ptr_t op = Operation::ptr_t(new Operation());
	LOG_INFO(logger, "\n" << *op);
	CPPUNIT_ASSERT(op->getAbstractionLevel()==AbstractionLevel::RED);

	LOG_INFO(logger, "-------------- test yellow operation... --------------");
	OperationClass::ptr_t opc = OperationClass::ptr_t(new OperationClass());
	opc->setName("calculateEverything");
	op->setOperationClass(opc);
	LOG_INFO(logger, "\n" << *op);
	CPPUNIT_ASSERT(op->getAbstractionLevel()==AbstractionLevel::YELLOW);

	LOG_INFO(logger, "-------------- test blue operation... --------------");
	OperationCandidate::ptr_t opcan1 = OperationCandidate::ptr_t(new OperationCandidate());
	opcan1->setType("phastgrid");
	opcan1->setOperationName("calculate1");
	opcan1->setResourceName("big_machine");
	opcan1->setQuality(0.9);
	op->getOperationClass()->addOperationCandidate(opcan1);
	OperationCandidate::ptr_t opcan2 = OperationCandidate::ptr_t(new OperationCandidate());
	opcan1->setType("phastgrid");
	opcan2->setOperationName("calculate2");
	opcan2->setResourceName("big_machine");
	op->getOperationClass()->addOperationCandidate(opcan2);
	LOG_INFO(logger, "  candidate size=" << op->getOperationClass()->getOperationCandidates().size());
	CPPUNIT_ASSERT(op->getOperationClass()->getOperationCandidates().size()==2);
	LOG_INFO(logger, "\n" << *op); 
	CPPUNIT_ASSERT(op->getAbstractionLevel()==AbstractionLevel::BLUE);

	LOG_INFO(logger, "-------------- test green operation... --------------");
	opcan1->setSelected();
	CPPUNIT_ASSERT(op->getOperationClass()->getOperationCandidates()[0]->isSelected()==true);
	CPPUNIT_ASSERT(op->getOperationClass()->getOperationCandidates()[0]->getAbstractionLevel()==AbstractionLevel::GREEN);
	CPPUNIT_ASSERT(op->getOperationClass()->getOperationCandidates()[1]->isSelected()==false);
	CPPUNIT_ASSERT(op->getOperationClass()->getOperationCandidates()[1]->getAbstractionLevel()==AbstractionLevel::BLUE);
	LOG_INFO(logger, "\n" << *op); 
	CPPUNIT_ASSERT(op->getAbstractionLevel()==AbstractionLevel::GREEN);

	LOG_INFO(logger, "-------------- test serialize/deserialize... --------------");
	Libxml2Builder builder;
	string str = builder.serializeOperation(*op);
	Operation::ptr_t op2 = builder.deserializeOperation(str);
	LOG_INFO(logger, "\n" << *op2);
	string str2 = builder.serializeOperation(*op2);
	CPPUNIT_ASSERT_EQUAL(str, str2);

	op.reset();

	LOG_INFO(logger, "============== END OPERATION TEST =============");
}
