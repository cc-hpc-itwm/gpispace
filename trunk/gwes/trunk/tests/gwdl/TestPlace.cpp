/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
// gwdl
#include <gwdl/Token.h>
#include <gwdl/Transition.h>
// tests
#include "TestPlace.h"
//fhglog
#include <fhglog/fhglog.hpp>

using namespace std;
using namespace gwdl;
using namespace fhg::log;
using namespace gwdl::tests;

CPPUNIT_TEST_SUITE_REGISTRATION( gwdl::tests::PlaceTest );

void PlaceTest::testPlace() 
{
	logger_t logger(getLogger("gwdl"));

   LOG_INFO(logger, "============== BEGIN PLACE TEST =============");
	
   // place set id
   string idstr("0815");
   Place *place = new Place(idstr);	
   LOG_INFO(logger, "place->getID()=" << place->getID());
   CPPUNIT_ASSERT(place->getID() == "0815");
   
   // place set token type
   string str("data");
   LOG_INFO(logger, "place->setTokenType(" << str << ")");
   place->setTokenType(str);
   LOG_INFO(logger, "place->getTokenType()=" << place->getTokenType());
   CPPUNIT_ASSERT(place->getTokenType() == "data");

   // place isEmpty
   LOG_INFO(logger, "place->isEmpty()=" << place->isEmpty());
   CPPUNIT_ASSERT(place->isEmpty());

   // place addToken
   gwdl::Token* token0 = new Token(false);
   LOG_INFO(logger, "place->addToken.id_" << token0->getID());
   place->addToken(token0);
   LOG_INFO(logger, "place->isEmpty()=" << place->isEmpty());
   CPPUNIT_ASSERT(!place->isEmpty());
   printTokens(logger,*place);
   
   // place addToken
   Token* token1 = new Token(true);
   LOG_INFO(logger, "place->addToken.id_" << token1->getID());
   LOG_INFO(logger, "Original token pointer=" << token1);
   place->addToken(token1);
   LOG_INFO(logger, "Place token pointer=" << place->getTokens()[1]);
   CPPUNIT_ASSERT(token1 == place->getTokens()[1]);
   printTokens(logger,*place);

   // place removeToken(int)
   LOG_INFO(logger, "place->removeToken(1)");
   place->removeToken(1);
   printTokens(logger,*place);
   CPPUNIT_ASSERT(place->getTokens().size() == 1);
   
   // place removeToken(token)
   Token* token2 = new Token(true);
   LOG_INFO(logger, "place->addToken.id_" << token2->getID());
   place->addToken(token2);
   printTokens(logger,*place);
   LOG_INFO(logger, "place->removeToken.id_" << token2->getID());
   place->removeToken(token2);
   printTokens(logger,*place);
   vector<Token*> tokens = place->getTokens();
   CPPUNIT_ASSERT(tokens.size() == 1);
   Token* t = tokens[0];
   CPPUNIT_ASSERT(t->getControl() == false);

   // place removeAllTokens()
   LOG_INFO(logger, "place.removeAllTokens()");
   place->removeAllTokens();
   CPPUNIT_ASSERT(place->isEmpty());
    
   // place set capacity
   LOG_INFO(logger, "place->setCapacity(1)");
   place->setCapacity(1);
   int cap = place->getCapacity();
   LOG_INFO(logger, "place->getCapacity()=" << cap);
   CPPUNIT_ASSERT(cap == 1);
   
   // place CapacityException
   LOG_INFO(logger, "check capacity exception");
   bool test = false;
   Token* token3 = new Token(false);
   LOG_INFO(logger, "place->addToken.id_" << token3->getID());
   place->addToken(token3);
   try {
   	gwdl::Token* token4 = new Token(true);
   	place->addToken(token4);
   } catch(gwdl::CapacityException e) {
   	LOG_INFO(logger, "CapacityException:" << e.message);
   	test = true;
   }
   CPPUNIT_ASSERT(test);
	
   test = false;
   try {
	place->setCapacity(0);
   } catch(gwdl::CapacityException e) {
   	LOG_INFO(logger, "CapacityException:" << e.message);
   	test = true;
   }
   CPPUNIT_ASSERT(test);

   // place getTokenNumber()
   LOG_INFO(logger, "place->getTokenNumber()="<< place->getTokenNumber());
   CPPUNIT_ASSERT(place->getTokenNumber()==1);
   
   // place set description
   str = "bla";
   LOG_INFO(logger, "place.setDescription(" << str << ")");
   place->setDescription(str);
   LOG_INFO(logger, "place->getDescription()=" << place->getDescription());
   CPPUNIT_ASSERT(place->getDescription() == "bla");
   delete place;
   
   LOG_INFO(logger, "test place with data tokens and properties...");
   try 
   {
   Place *place1 = new Place("");
   LOG_INFO(logger, "  constructed place with id " << place1->getID());
   CPPUNIT_ASSERT(place1->getID()=="p0");
   Properties *props1 = new Properties();
   props1->put("k1","v1");
   props1->put("k2","v2");
   place1->setProperties(*props1);
   Properties &props1b = place1->getProperties();
   props1->put("k3", "v3");  // should be ignored!
   props1b.put("k3b","v3b"); // should change the properties!
   CPPUNIT_ASSERT(place1->getProperties().size()==3);
   Data *data1 = new Data("<data><x>245.4</x></data>");
   Token *token1 = new Token(data1);
   place1->addToken(token1);
   Data *data1b = new Data("<data><y>445</y></data>");
   Token *token1b = new Token(data1b);
   place1->addToken(token1b);

   CPPUNIT_ASSERT(place1->getTokenNumber()==2);
   vector<Token*> tokens = place1->getTokens();
   for (unsigned int i=0; i<tokens.size(); i++) {
   		
   		LOG_INFO(logger, "tokens[" << i << "].getData(): " << tokens[i]->getData()->toString());
   }
   CPPUNIT_ASSERT(*(tokens[0]->getData()->toString())=="<data><x>245.4</x></data>");   
   CPPUNIT_ASSERT(*(tokens[1]->getData()->toString())=="<data><y>445</y></data>");   
   place1->removeToken(token1);
   CPPUNIT_ASSERT(place1->getTokenNumber()==1);
   LOG_INFO(logger, *place1);
   
   delete props1;
   delete place1;
   }
   catch (WorkflowFormatException e) 
   {
   LOG_INFO(logger, "WorkflowFormatException: " << e.message);
   }
   
   // lock token
   Place *place2 = new Place("");
   CPPUNIT_ASSERT(place2->getNextUnlockedToken() == NULL);
   Token* token2a = new Token(true);
   Token* token2b = new Token(false);
   place2->addToken(token2a);
   place2->addToken(token2b);
   LOG_INFO(logger, *place2);
   CPPUNIT_ASSERT(place2->getNextUnlockedToken() == token2a);
   Transition* tr2 = new Transition("");
   place2->lockToken(token2a,tr2);
   CPPUNIT_ASSERT(place2->getNextUnlockedToken() == token2b);
   place2->lockToken(token2b,tr2);
   CPPUNIT_ASSERT(place2->getNextUnlockedToken() == NULL);
   
   delete place2;
   delete tr2;

   LOG_INFO(logger, "============== END PLACE TEST =============");
   
}

void PlaceTest::printTokens(logger_t logger, gwdl::Place &place) 
{
	vector<gwdl::Token*> tokens = place.getTokens();
	for (unsigned int i=0; i<tokens.size(); i++) {
		gwdl::Token* token = tokens[i];
		LOG_INFO(logger, "Token[" << i << "].id_" << token->getID() << "=" << *token);					
	}	
	
}

