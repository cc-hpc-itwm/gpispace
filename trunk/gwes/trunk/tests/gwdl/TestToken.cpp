/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#include <iostream>
#include <ostream>
#include <assert.h>
//gwdl
#include <gwdl/XMLUtils.h>
#include <gwdl/Transition.h>
//tests
#include "TestToken.h"

#define X(str) XMLString::transcode((const char*)& str)

using namespace std;
using namespace gwdl;
XERCES_CPP_NAMESPACE_USE

void testToken()
{
	cout << "============== BEGIN TOKEN TEST =============" << endl;
	
	cout << "test default control token..." << endl;
	Token * token1 = new Token();
	cout << *token1 << endl;
	assert(token1->getControl());
	delete token1;
	
	cout << "test false control token..." << endl;
	Token * token2 = new Token(false);
	cout << *token2 << endl;
	assert(!token2->getControl());
	delete token2;

	cout << "test control token with properties..." << endl;
	Properties *props = new Properties();
	props->put("key1","value1");
	props->put("key2","value2");
	Token * token3 = new Token(*props,true);
	cout << *props << endl;
	Properties props2 = token3->getProperties();
	assert(props2.get("key2")=="value2");
	cout << *token3 << endl;
	delete token3;
	
	try {
		cout << "test data token with data constructed from element..." << endl;
		DOMDocument* doc = XMLUtils::Instance()->createEmptyDocument(true);
		// <data>
		DOMElement* dataElement = doc->createElement(X("data"));
		// <value1>25</value1>
		DOMElement* e1 = doc->createElement(X("value1"));
		dataElement->appendChild(e1);
		DOMText* e1value = doc->createTextNode(X("25"));
		e1->appendChild(e1value);
		// <value2>15</value1>
		DOMElement* e2 = doc->createElement(X("value2"));
		dataElement->appendChild(e2);
		DOMText* e2value = doc->createTextNode(X("15"));
		e2->appendChild(e2value);
		Data *data4 = new Data(dataElement);
		data4->toElement(doc);
		cout << "  data constructed:" << *data4 << endl;
	    Token* token4 = new Token(data4);
	    cout << "  token4->toElement(doc) ..." << endl;
		DOMElement* token4elem = token4->toElement(doc);
	    string* token4str = XMLUtils::Instance()->serialize(token4elem,true);
	    cout << "  token4str:" << *token4str << endl;
	//    delete data4;
		delete token4;
	} catch (WorkflowFormatException e) {
		cerr << e.message << endl;	
	} catch (DOMException e) {
		char* message = XMLString::transcode(e.msg);
        cerr << "DOMException: " << message << endl;
        XMLString::release(&message);
        assert(false);
	}

    cout << "test data token with data constructed from string..." << endl;
    bool test = false;
	try {
		string* str = new string("<data><x>1</x><y>2</y></data>");
		Data* data5 = new Data(*str);
		cout << " data constructed." << endl;
		Token* token5 = new Token(data5);
		cout << " token constructed." << endl;
		cout << *token5 << endl;
		Data *data5b = token5->getData();
		string* str2 = data5b->toString();
		cout << *str2 << endl;
		assert(*str==*str2);
		assert(token5->isData());
		test = true;
		delete str;
		delete token5;
	} catch (WorkflowFormatException e) {
		cerr << e.message << endl;	
	} catch (DOMException e) {
		char* message = XMLString::transcode(e.msg);
        cerr << "DOMException: " << message << endl;
        XMLString::release(&message);
	}
	assert(test);
	
	cout << "test data token with properties..." << endl;
	string* str6 = new string("<data><x>6</x></data>");
	Data* data6 = new Data(*str6);
	Token* token6 = new Token(*props,data6);
	cout << *token6 << endl;
	assert(*str6==*(token6->getData()->toString()));
	assert(token6->getProperties().get("key2")=="value2");
	delete token6;
	delete str6;
	
	cout << "test token.lock(transition)..." << endl;
   	Token *token7 = new Token();
   	assert(token7->isLocked()==false);
   	Transition *t7 = new Transition("");
   	Transition *t7b = new Transition("");
   	token7->lock(t7);
   	assert(token7->isLocked());
    assert(token7->isLockedBy(t7));
    assert(!token7->isLockedBy(t7b));
   	delete token7;
   	delete t7;
   	delete t7b;

	cout << "test token.deepCopy()..." << endl;
	// control token
   	Token *token8 = new Token(*props,true);
   	Token *token8cp = token8->deepCopy();
   	assert(token8 != token8cp);
   	token8->getProperties().put("key1","valueXXX");
   	assert(token8->getProperties().get("key1").compare("valueXXX")==0);
   	assert(token8cp->getProperties().get("key1").compare("value1")==0);
    cout << *token8cp << endl;
    delete token8cp;
    cout << *token8 << endl;
    assert (token8);
   	delete token8;
   	// data token
	Data* data9 = new Data("<data><x>1</x><y>2</y></data>");
   	Token *token9 = new Token(*props,data9);
   	Token *token9cp = token9->deepCopy();
   	cout << *token9 << endl;
   	assert(token9 != token9cp);
   	assert(token9->getData() != token9cp->getData());
   	// ToDo: Getting 'xercesc_2_7::DOMException' if deleting data9 here...
//   	delete data9;
//   	assert(token9cp);
   	delete token9;
   	assert(token9cp);
   	cout << *token9cp << endl;
   	delete token9cp;
   	
	delete props;
	cout << "============== END TOKEN TEST =============" << endl;
}
