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
#include <assert.h>
// gwdl
#include <gwdl/Token.h>
#include <gwdl/Transition.h>
// tests
#include "TestPlace.h"

using namespace std;
using namespace gwdl;
 
void testPlace() 
{
   cout << "============== BEGIN PLACE TEST =============" << endl;
	
   // place set id
   string idstr("0815");
   Place *place = new Place(idstr);	
   cout << "place->getID()=" << place->getID() << endl;
   assert(place->getID() == "0815");
   
   // place set token type
   string str("data");
   cout << "place->setTokenType(" << str << ")" << endl;
   place->setTokenType(str);
   cout << "place->getTokenType()=" << place->getTokenType() << endl;
   assert(place->getTokenType() == "data");

   // place isEmpty
   cout << "place->isEmpty()=" << place->isEmpty() << endl;
   assert(place->isEmpty());

   // place addToken
   gwdl::Token* token0 = new Token(false);
   cout << "place->addToken.id_" << token0->getID() << endl;
   place->addToken(token0);
   cout << "place->isEmpty()=" << place->isEmpty() << endl;
   assert(!place->isEmpty());
   printTokens(*place);
   
   // place addToken
   Token* token1 = new Token(true);
   cout << "place->addToken.id_" << token1->getID() << endl;
   cout << "Original token pointer=" << token1 << endl;
   place->addToken(token1);
   cout << "Place token pointer=" << place->getTokens()[1] << endl;
   assert(token1 == place->getTokens()[1]);
   printTokens(*place);

   // place removeToken(int)
   cout << "place->removeToken(1)" << endl;
   place->removeToken(1);
   printTokens(*place);
   assert(place->getTokens().size() == 1);
   
   // place removeToken(token)
   Token* token2 = new Token(true);
   cout << "place->addToken.id_" << token2->getID() << endl;
   place->addToken(token2);
   printTokens(*place);
   cout << "place->removeToken.id_" << token2->getID() << endl;
   place->removeToken(token2);
   printTokens(*place);
   vector<Token*> tokens = place->getTokens();
   assert(tokens.size() == 1);
   Token* t = tokens[0];
   assert(t->getControl() == false);

   // place removeAllTokens()
   cout << "place.removeAllTokens()" << endl;
   place->removeAllTokens();
   assert(place->isEmpty());
    
   // place set capacity
   cout << "place->setCapacity(1)" << endl;
   place->setCapacity(1);
   int cap = place->getCapacity();
   cout << "place->getCapacity()=" << cap << endl;
   assert(cap == 1);
   
   // place CapacityException
   cout << "check capacity exception" << endl;
   bool test = false;
   Token* token3 = new Token(false);
   cout << "place->addToken.id_" << token3->getID() << endl;
   place->addToken(token3);
   try {
   	gwdl::Token* token4 = new Token(true);
   	place->addToken(token4);
   } catch(gwdl::CapacityException e) {
   	cout << "CapacityException:" << e.message << endl;
   	test = true;
   }
   assert(test);
	
   test = false;
   try {
	place->setCapacity(0);
   } catch(gwdl::CapacityException e) {
   	cout << "CapacityException:" << e.message << endl;
   	test = true;
   }
   assert(test);

   // place getTokenNumber()
   cout << "place->getTokenNumber()="<< place->getTokenNumber() << endl;
   assert(place->getTokenNumber()==1);
   
   // place set description
   str = "bla";
   cout << "place.setDescription(" << str << ")" << endl;
   place->setDescription(str);
   cout << "place->getDescription()=" << place->getDescription() << endl;
   assert(place->getDescription() == "bla");
   delete place;
   
   cout << "test place with data tokens and properties..." << endl;
   try 
   {
   Place *place1 = new Place("");
   cout << "  constructed place with id " << place1->getID() << endl;
   assert(place1->getID()=="p0");
   Properties *props1 = new Properties();
   props1->put("k1","v1");
   props1->put("k2","v2");
   place1->setProperties(*props1);
   Properties &props1b = place1->getProperties();
   props1->put("k3", "v3");  // should be ignored!
   props1b.put("k3b","v3b"); // should change the properties!
   assert(place1->getProperties().size()==3);
   Data *data1 = new Data("<data><x>245.4</x></data>");
   Token *token1 = new Token(data1);
   place1->addToken(token1);
   Data *data1b = new Data("<data><y>445</y></data>");
   Token *token1b = new Token(data1b);
   place1->addToken(token1b);

   assert(place1->getTokenNumber()==2);
   vector<Token*> tokens = place1->getTokens();
   for (unsigned int i=0; i<tokens.size(); i++) {
   		
   		cout << "tokens[" << i << "].getData(): " << tokens[i]->getData()->toString() << endl;
   }
   assert(*(tokens[0]->getData()->toString())=="<data><x>245.4</x></data>");   
   assert(*(tokens[1]->getData()->toString())=="<data><y>445</y></data>");   
   place1->removeToken(token1);
   assert(place1->getTokenNumber()==1);
   cout << *place1 << endl;
   
   delete props1;
   delete place1;
   }
   catch (WorkflowFormatException e) 
   {
   cout << "WorkflowFormatException: " << e.message << endl;
   }
   
   // lock token
   Place *place2 = new Place("");
   assert(place2->getNextUnlockedToken() == NULL);
   Token* token2a = new Token(true);
   Token* token2b = new Token(false);
   place2->addToken(token2a);
   place2->addToken(token2b);
   cout << *place2 << endl;
   assert(place2->getNextUnlockedToken() == token2a);
   Transition* tr2 = new Transition("");
   place2->lockToken(token2a,tr2);
   assert(place2->getNextUnlockedToken() == token2b);
   place2->lockToken(token2b,tr2);
   assert(place2->getNextUnlockedToken() == NULL);
   
   delete place2;
   delete tr2;

   cout << "============== END PLACE TEST =============" << endl;
   
}

void printTokens(gwdl::Place &place) 
{
	vector<gwdl::Token*> tokens = place.getTokens();
	for (unsigned int i=0; i<tokens.size(); i++) {
		gwdl::Token* token = tokens[i];
		cout << "Token[" << i << "].id_" << token->getID() << "=" << *token << endl;					
	}	
	
}

