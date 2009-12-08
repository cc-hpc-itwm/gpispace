/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#include <iostream>
#include <sstream>
// gwdl
#include <gwdl/Token.h>
#include <gwdl/Place.h>
#include <gwdl/OperationCandidate.h>
// tests
#include "TestTransition.h"
//fhglog
#include <fhglog/fhglog.hpp>

using namespace std;
using namespace gwdl;
using namespace fhg::log;
using namespace gwdl::tests;

 CPPUNIT_TEST_SUITE_REGISTRATION( gwdl::tests::TransitionTest );

void TransitionTest::testTransition() 
{
	logger_t logger(getLogger("gwdl"));
   LOG_INFO(logger, "============== BEGIN TRANSITION TEST =============");
   
   LOG_INFO(logger, "test empty transition...");
   Transition *t0 = new Transition("");
   LOG_INFO(logger, *t0);
   
   LOG_INFO(logger, "test description...");
   t0->setDescription("This is the description of the transition"); 
   CPPUNIT_ASSERT(t0->getDescription()=="This is the description of the transition");
		
   LOG_INFO(logger, "test transition connected to four places...");
   Place::ptr_t p0 = Place::ptr_t(new Place(""));
   Place::ptr_t p1 = Place::ptr_t(new Place(""));
   Place::ptr_t p2 = Place::ptr_t(new Place(""));
   Place::ptr_t p3 = Place::ptr_t(new Place(""));
   Edge::ptr_t e0 = Edge::ptr_t(new Edge( p0,"input0"));
   Edge::ptr_t e1 = Edge::ptr_t(new Edge( p1,"input1"));
   Edge::ptr_t e2 = Edge::ptr_t(new Edge( p2,"output0"));
   Edge::ptr_t e3 = Edge::ptr_t(new Edge( p3,"output1"));
   t0->addReadEdge(e0);
   t0->addInEdge(e1);
   t0->addWriteEdge(e2);
   t0->addOutEdge(e3);
   
   LOG_INFO(logger, "test properties...");
   t0->getProperties().put("key1","value1");
   
   LOG_INFO(logger, "test condition...");
   t0->addCondition("true");
   
   // this is still a control transition (without operation)
   CPPUNIT_ASSERT(t0->getAbstractionLevel()==AbstractionLevel::BLACK);

   // link this transtion with an operation
   LOG_INFO(logger, "test operation class...");
   Operation::ptr_t op = Operation::ptr_t(new Operation()); 
   OperationClass::ptr_t opc = OperationClass::ptr_t(new OperationClass());
   opc->setName("calculateEverything");
   op->setOperationClass(opc);
   t0->setOperation(op);
   CPPUNIT_ASSERT(t0->getAbstractionLevel()==AbstractionLevel::YELLOW);
     
   OperationCandidate::ptr_t opcand1 = OperationCandidate::ptr_t(new OperationCandidate());
   opcand1->setType("psp");
   opcand1->setOperationName("calculate1");
   opcand1->setResourceName("big_machine");
   opcand1->setQuality(0.9);
   opcand1->setSelected();
   t0->getOperation()->getOperationClass()->addOperationCandidate(opcand1);
   CPPUNIT_ASSERT(t0->getAbstractionLevel()==AbstractionLevel::GREEN);

   // transition is not enabled
   CPPUNIT_ASSERT(t0->isEnabled()==false);	
   
   // add read token
   Token::ptr_t d0 = Token::ptr_t(new Token());
   p0->addToken(d0);
   CPPUNIT_ASSERT(t0->isEnabled()==false);	
   
   // add input token
   Token::ptr_t d1 = Token::ptr_t(new Token());
   p1->addToken(d1);
   CPPUNIT_ASSERT(t0->isEnabled()==false);	
   
   // add write token
   Token::ptr_t d2 = Token::ptr_t(new Token());
   p2->addToken(d2);
   // transition is now enabled
   CPPUNIT_ASSERT(t0->isEnabled()==true);	

   LOG_INFO(logger, *t0);
   		
   delete t0;
   LOG_INFO(logger, "============== END TRANSITION TEST =============");
}
