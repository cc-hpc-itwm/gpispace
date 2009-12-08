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
#include <gwdl/Libxml2Builder.h>
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

	LOG_INFO(logger, "-------------- place constructor... --------------");
	string idstr("0815");
	Place::ptr_t placeP = Place::ptr_t(new Place(idstr));	
	LOG_INFO(logger, "\n" << *placeP);
	CPPUNIT_ASSERT_EQUAL_MESSAGE("place ID", idstr, placeP->getID());
	CPPUNIT_ASSERT_MESSAGE("place is empty", placeP->isEmpty());

	LOG_INFO(logger, "-------------- place set capacity... --------------");
	placeP->setCapacity(5);
	LOG_INFO(logger, "\n" << *placeP);
	CPPUNIT_ASSERT_EQUAL_MESSAGE("place capacity", 5, placeP->getCapacity());

	LOG_INFO(logger, "-------------- place set token type... --------------");
	string str("data");
	placeP->setTokenType(str);
	LOG_INFO(logger, "\n" << *placeP);
	CPPUNIT_ASSERT_EQUAL_MESSAGE("place token type", string("data"), placeP->getTokenType());

	LOG_INFO(logger, "-------------- place addToken... --------------");
	Token::ptr_t token0 = Token::ptr_t(new Token(Token::CONTROL_FALSE));
	placeP->addToken(token0);
	CPPUNIT_ASSERT_MESSAGE("!place->isEmpty()", !placeP->isEmpty());
	printTokens(logger,*placeP);
	Token::ptr_t token1 = Token::ptr_t(new Token(Token::CONTROL_TRUE));
	LOG_INFO(logger, "place->addToken.id_" << token1->getID());
	LOG_INFO(logger, "Original token pointer=" << token1);
	placeP->addToken(token1);
	CPPUNIT_ASSERT_EQUAL_MESSAGE("token pointer", token1, placeP->getTokens()[1]);
	printTokens(logger,*placeP);
	
	// place removeToken(int)
	LOG_INFO(logger, "-------------- place->removeToken(1) --------------");
	placeP->removeToken(1);
	printTokens(logger,*placeP);
	CPPUNIT_ASSERT(placeP->getTokens().size() == 1);

	// place removeToken(token)
	Token::ptr_t token2 = Token::ptr_t(new Token(Token::CONTROL_TRUE));
	LOG_INFO(logger, "-------------- place->addToken.id_" << token2->getID() << "--------------");
	placeP->addToken(token2);
	printTokens(logger,*placeP);
	LOG_INFO(logger, "place->removeToken.id_" << token2->getID());
	placeP->removeToken(token2);
	printTokens(logger,*placeP);
	vector<Token::ptr_t> tokens = placeP->getTokens();
	CPPUNIT_ASSERT(tokens.size() == 1);
	Token::ptr_t t = tokens[0];
	CPPUNIT_ASSERT(t->getControl() == false);

	// place removeAllTokens()
	LOG_INFO(logger, "-------------- place.removeAllTokens() --------------");
	placeP->removeAllTokens();
	CPPUNIT_ASSERT(placeP->isEmpty());

	// place set capacity
	LOG_INFO(logger, "-------------- place->setCapacity(1) --------------");
	placeP->setCapacity(1);
	int cap = placeP->getCapacity();
	LOG_INFO(logger, "place->getCapacity()=" << cap);
	CPPUNIT_ASSERT(cap == 1);

	LOG_INFO(logger, "-------------- check capacity exception --------------");
	bool test = false;
	Token::ptr_t token3 = Token::ptr_t(new Token(Token::CONTROL_FALSE));
	LOG_INFO(logger, "place->addToken.id_" << token3->getID());
	placeP->addToken(token3);
	try {
		gwdl::Token::ptr_t token4 = Token::ptr_t(new Token(Token::CONTROL_TRUE));
		placeP->addToken(token4);
	} catch(gwdl::CapacityException e) {
		LOG_INFO(logger, "CapacityException:" << e.message);
		test = true;
	}
	CPPUNIT_ASSERT(test);

	test = false;
	try {
		placeP->setCapacity(0);
	} catch(gwdl::CapacityException e) {
		LOG_INFO(logger, "CapacityException:" << e.message);
		test = true;
	}
	CPPUNIT_ASSERT(test);
//
//	// place getTokenNumber()
//	LOG_INFO(logger, "placeP->getTokenNumber()="<< placeP->getTokenNumber());
//	CPPUNIT_ASSERT(placeP->getTokenNumber()==1);
//
//	// place set description
//	str = "bla";
//	LOG_INFO(logger, "placeP.setDescription(" << str << ")");
//	placeP->setDescription(str);
//	LOG_INFO(logger, "placeP->getDescription()=" << placeP->getDescription());
//	CPPUNIT_ASSERT(placeP->getDescription() == "bla");
//	delete place;
//
//	LOG_INFO(logger, "test place with data tokens and properties...");
//	try 
//	{
//		Place *place1 = new Place("");
//		LOG_INFO(logger, "  constructed place with id " << place1->getID());
//		// should generate place ID "pX".
//		CPPUNIT_ASSERT(place1->getID().size() > 0);
//		CPPUNIT_ASSERT(place1->getID().substr(0,1) == "p");
//		Properties *props1 = new Properties();
//		props1->put("k1","v1");
//		props1->put("k2","v2");
//		place1->setProperties(*props1);
//		Properties &props1b = place1->getProperties();
//		props1->put("k3", "v3");  // should be ignored!
//		props1b.put("k3b","v3b"); // should change the properties!
//		CPPUNIT_ASSERT(place1->getProperties().size()==3);
//		Data::ptr_t data5 = Data::ptr_t(new Data("<data><x>245.4</x></data>"));
//		Token *token5 = Token::ptr_t(new Token(data5));
//		place1->addToken(token5);
//		Data::ptr_t data5b = Data::ptr_t(new Data("<data><y>445</y></data>"));
//		Token *token5b = Token::ptr_t(new Token(data5b));
//		place1->addToken(token5b);
//
//		CPPUNIT_ASSERT(place1->getTokenNumber()==2);
//		vector<Token::ptr_t> tokens5 = place1->getTokens();
//		for (unsigned int i=0; i<tokens5.size(); i++) {
//			LOG_INFO(logger, "tokens5[" << i << "].getData(): " << tokens5[i]->getData()->getContent());
//		}
//		CPPUNIT_ASSERT_EQUAL(string("<data><x>245.4</x></data>"), tokens5[0]->getData()->getContent());   
//		CPPUNIT_ASSERT_EQUAL(string("<data><y>445</y></data>"), tokens5[1]->getData()->getContent());   
//		place1->removeToken(token5);
//		CPPUNIT_ASSERT(place1->getTokenNumber()==1);
//		LOG_INFO(logger, *place1);
//
//		delete props1;
//		delete place1;
//	}
//	catch (WorkflowFormatException e) 
//	{
//		LOG_INFO(logger, "WorkflowFormatException: " << e.message);
//	}
//
//	// lock token
//	Place *place2 = new Place("");
//	CPPUNIT_ASSERT(place2->getNextUnlockedToken() == NULL);
//	Token::ptr_t token2a = Token::ptr_t(new Token(Token::CONTROL_TRUE));
//	Token::ptr_t token2b = Token::ptr_t(new Token(Token::CONTROL_FALSE));
//	place2->addToken(token2a);
//	place2->addToken(token2b);
//	LOG_INFO(logger, *place2);
//	CPPUNIT_ASSERT(place2->getNextUnlockedToken() == token2a);
//	Transition* tr2 = new Transition("");
//	place2->lockToken(token2a,tr2);
//	CPPUNIT_ASSERT(place2->getNextUnlockedToken() == token2b);
//	place2->lockToken(token2b,tr2);
//	CPPUNIT_ASSERT(place2->getNextUnlockedToken() == NULL);
//
//	delete place2;
//	delete tr2;

	LOG_INFO(logger, "============== END PLACE TEST =============");

}

void PlaceTest::printTokens(logger_t logger, gwdl::Place &place) 
{
	vector<gwdl::Token::ptr_t> tokens = place.getTokens();
	for (unsigned int i=0; i<tokens.size(); i++) {
		gwdl::Token::ptr_t token = tokens[i];
//		LOG_INFO(logger, "Token[" << i << "].id_" << token->getID() << "=" << *token);					
		LOG_INFO(logger, "Token[" << i << "].id_" << token->getID());					
	}	

}

