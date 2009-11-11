/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
// tests
#include "TestToken.h"
// gwdl
#include <gwdl/Libxml2Builder.h>
#include <gwdl/Transition.h>
// fhglog
#include <fhglog/fhglog.hpp>
// std
#include <iostream>
#include <ostream>
#include <time.h>

using namespace std;
using namespace gwdl;
using namespace fhg::log;
using namespace gwdl::tests;

CPPUNIT_TEST_SUITE_REGISTRATION( gwdl::tests::TokenTest );

void TokenTest::testToken()
{
	logger_t logger(getLogger("gwdl"));
	LOG_INFO(logger, "============== BEGIN TOKEN TEST =============");
	Libxml2Builder builder;
	
	LOG_INFO(logger, "-------------- test default constructor Token()... --------------");
	Token::ptr_t token = Token::ptr_t(new Token());
	LOG_INFO(logger, "\n" << *token);
	CPPUNIT_ASSERT_MESSAGE("control token1", !token->isData());
	CPPUNIT_ASSERT_MESSAGE("control token1 true", token->getControl());
	
	LOG_INFO(logger, "-------------- test constructor Token(true)... --------------");
	token.reset(new Token(Token::CONTROL_TRUE));
	LOG_INFO(logger, "\n" << *token);
	CPPUNIT_ASSERT_MESSAGE("control token1", !token->isData());
	CPPUNIT_ASSERT_MESSAGE("control token1 true", token->getControl());

	LOG_INFO(logger, "-------------- test constructor Token(false)... --------------");
	token.reset(new Token(Token::CONTROL_FALSE));
	LOG_INFO(logger, "\n" << *token);
	CPPUNIT_ASSERT(!token->getControl());

	LOG_INFO(logger, "-------------- test constructor Token(properties, control)... --------------");
	Properties::ptr_t propsP(new Properties());
	propsP->put("key1","value1");
	propsP->put("key2","value2");
	token.reset(new Token(propsP, Token::CONTROL_TRUE));
	LOG_INFO(logger, "\n" << *token);
	CPPUNIT_ASSERT_EQUAL_MESSAGE("number properties", (size_t) 2, token->getProperties()->size() );
	CPPUNIT_ASSERT(token->getControl());
	
	LOG_INFO(logger, "-------------- test constructor Token(data)... --------------");
	Data::ptr_t dataP = Data::ptr_t(new Data("<test>15</test>"));
	token.reset(new Token(dataP));
	LOG_INFO(logger, "\n" << *token);
	CPPUNIT_ASSERT(token->isData());
	CPPUNIT_ASSERT_EQUAL(string("<test>15</test>"), token->getData()->getContent());
	
	LOG_INFO(logger, "-------------- test constructor Token(properties, data)... --------------");
	token.reset(new Token(propsP,dataP));
	LOG_INFO(logger, "\n" << *token);
	CPPUNIT_ASSERT(token->isData());
	CPPUNIT_ASSERT_EQUAL(string("<test>15</test>"), token->getData()->getContent());
	CPPUNIT_ASSERT_EQUAL_MESSAGE("number properties", (size_t) 2, token->getProperties()->size() );

	LOG_INFO(logger, "-------------- testDeserializeSerialize() data... --------------");
	string str = string("<token><property name=\"noText\"/><property name=\"key1\">value1</property><data><x>1</x></data></token>");
	LOG_INFO(logger, "\n" << str);
	Token::ptr_t tokenP = builder.deserializeToken(str);
	LOG_INFO(logger, "\n" << *tokenP);
	string str2 = builder.serializeToken(*tokenP);
	CPPUNIT_ASSERT_EQUAL(string("<token xmlns:gwdl=\"http://www.gridworkflow.org/gworkflowdl\">\n  <gwdl:property name=\"key1\">value1</gwdl:property>\n  <gwdl:property name=\"noText\"/>\n  <gwdl:data>\n    <x>1</x>\n  </gwdl:data>\n</token>\n"), str2);

	LOG_INFO(logger, "-------------- testDeserializeSerialize() control... --------------");
	str = string("<token><control>true</control></token>");
	LOG_INFO(logger, "\n" << str);
	tokenP = builder.deserializeToken(str);
	LOG_INFO(logger, "\n" << *tokenP);
	str2 = builder.serializeToken(*tokenP);
	CPPUNIT_ASSERT_EQUAL(string("<token xmlns:gwdl=\"http://www.gridworkflow.org/gworkflowdl\">\n  <gwdl:control>true</gwdl:control>\n</token>\n"), str2);

	LOG_INFO(logger, "-------------- testDeserializeSerialize() control false... --------------");
	str = string("<token><control>false</control></token>");
	LOG_INFO(logger, "\n" << str);
	tokenP = builder.deserializeToken(str);
	LOG_INFO(logger, "\n" << *tokenP);
	str2 = builder.serializeToken(*tokenP);
	CPPUNIT_ASSERT_EQUAL(string("<token xmlns:gwdl=\"http://www.gridworkflow.org/gworkflowdl\">\n  <gwdl:control>false</gwdl:control>\n</token>\n"), str2);

	LOG_INFO(logger, "-------------- testDeserialize with wrong xml: missing data element... --------------");
	str = string("<token></token>");
	LOG_INFO(logger, "\n" << str);
	bool success = false;
	try {
		tokenP = builder.deserializeToken(str);
		LOG_INFO(logger, "\n" << *tokenP);
	} catch (WorkflowFormatException e) {
		LOG_INFO(logger, "WorkflowFormatException: " << e.message);
		LOG_INFO(logger,"This exception was thrown intentionally, everything is OK!");
		success = true;
	}
	CPPUNIT_ASSERT_MESSAGE("Throws WorkflowFormatException", success);
	
	LOG_INFO(logger, "-------------- testDeserialize with wrong xml: too many child elements... --------------");
	str = string("<token><data><x>1</x><y>2</y></data></token>");
	LOG_INFO(logger, "\n" << str);
	success = false;
	try {
		tokenP = builder.deserializeToken(str);
		LOG_INFO(logger, "\n" << *tokenP);
	} catch (WorkflowFormatException e) {
		LOG_INFO(logger, "WorkflowFormatException: " << e.message);
		LOG_INFO(logger,"This exception was thrown intentionally, everything is OK!");
		success = true;
	}
	CPPUNIT_ASSERT_MESSAGE("Throws WorkflowFormatException", success);

	LOG_INFO(logger, "-------------- testDeserialize with wrong xml: wrong control content... --------------");
	str = string("<token><control>XXXtrue</control></token>");
	LOG_INFO(logger, "\n" << str);
	success = false;
	try {
		tokenP = builder.deserializeToken(str);
		LOG_INFO(logger, "\n" << *tokenP);
	} catch (WorkflowFormatException e) {
		LOG_INFO(logger, "WorkflowFormatException: " << e.message);
		LOG_INFO(logger,"This exception was thrown intentionally, everything is OK!");
		success = true;
	}
	CPPUNIT_ASSERT_MESSAGE("Throws WorkflowFormatException", success);

	LOG_INFO(logger, "-------------- testDeserializeSerialize() loop... --------------");
	time_t before = time (NULL);
	for (unsigned int i=0; i<10000; i++) {
		ostringstream oss;
		oss << "<token><data><i>" << i << "</i></data></token>";
		tokenP = builder.deserializeToken(oss.str());
		str2 = builder.serializeToken(*tokenP);
	}
	time_t after = time (NULL);
	LOG_INFO(logger, "testDeserializeSerialize() loop... done in " << after-before << " seconds.");

//	LOG_INFO(logger, "test control token with properties...");
//	Properties *props = new Properties();
//	props->put("key1","value1");
//	props->put("key2","value2");
//	Token * token3 = new Token(*props,true);
//	LOG_INFO(logger, *props);
//	Properties props2 = token3->getProperties();
//	CPPUNIT_ASSERT(props2.get("key2")=="value2");
//	LOG_INFO(logger, *token3);
//	delete token3;
//	
//	try {
//		LOG_INFO(logger, "test data token with data constructed from element...");
//		DOMDocument* doc = XMLUtils::Instance()->createEmptyDocument(true);
//		// <data>
//		DOMElement* dataElement = doc->createElement(X("data"));
//		// <value1>25</value1>
//		DOMElement* e1 = doc->createElement(X("value1"));
//		dataElement->appendChild(e1);
//		DOMText* e1value = doc->createTextNode(X("25"));
//		e1->appendChild(e1value);
//		// <value2>15</value1>
//		DOMElement* e2 = doc->createElement(X("value2"));
//		dataElement->appendChild(e2);
//		DOMText* e2value = doc->createTextNode(X("15"));
//		e2->appendChild(e2value);
//		LOG_WARN(logger, "ToDo: refractoring from xerces-c to libxml2!");
////		Data *data4 = new Data(dataElement);
////		data4->toElement(doc);
////		LOG_INFO(logger, "  data constructed:" << *data4);
////	    Token* token4 = new Token(data4);
////	    LOG_INFO(logger, "  token4->toElement(doc) ...");
////		DOMElement* token4elem = token4->toElement(doc);
////	    string* token4str = XMLUtils::Instance()->serialize(token4elem,true);
////	    LOG_INFO(logger, "  token4str:" << *token4str);
////	//    delete data4;
////		delete token4;
//	} catch (WorkflowFormatException e) {
//		LOG_ERROR(logger, e.message);	
//	} catch (DOMException e) {
//		char* message = XMLString::transcode(e.msg);
//        LOG_FATAL(logger, "DOMException: " << message);
//        XMLString::release(&message);
//        CPPUNIT_ASSERT(false);
//	}
//
//    LOG_INFO(logger, "test data token with data constructed from string...");
//    bool test = false;
//	try {
//		string str = string("<data><x>1</x><y>2</y></data>");
//		Data* data5 = new Data(str);
//		LOG_INFO(logger, " data constructed.");
//		Token* token5 = new Token(data5);
//		LOG_INFO(logger, " token constructed.");
//		LOG_INFO(logger, *token5);
//		Data *data5b = token5->getData();
//		string str2 = data5b->getContent();
//		LOG_INFO(logger, str2);
//		CPPUNIT_ASSERT_EQUAL_MESSAGE("Token string contents", str, str2);
//		CPPUNIT_ASSERT_MESSAGE("isData()", token5->isData());
//		test = true;
//		delete token5;
//	} catch (WorkflowFormatException e) {
//		LOG_ERROR(logger, e.message);	
//	} catch (DOMException e) {
//		char* message = XMLString::transcode(e.msg);
//        LOG_ERROR(logger, "DOMException: " << message);
//        XMLString::release(&message);
//	}
//	CPPUNIT_ASSERT(test);
//	
//	LOG_INFO(logger, "test data token with properties...");
//	string str6 = string("<data><x>6</x></data>");
//	Data* data6 = new Data(str6);
//	Token* token6 = new Token(*props,data6);
//	LOG_INFO(logger, *token6);
//	CPPUNIT_ASSERT(str6==token6->getData()->getContent());
//	CPPUNIT_ASSERT(token6->getProperties().get("key2")=="value2");
//	delete token6;
//	
//	LOG_INFO(logger, "test token.lock(transition)...");
//   	Token *token7 = new Token();
//   	CPPUNIT_ASSERT(token7->isLocked()==false);
//   	Transition *t7 = new Transition("");
//   	Transition *t7b = new Transition("");
//   	token7->lock(t7);
//   	CPPUNIT_ASSERT(token7->isLocked());
//    CPPUNIT_ASSERT(token7->isLockedBy(t7));
//    CPPUNIT_ASSERT(!token7->isLockedBy(t7b));
//   	delete token7;
//   	delete t7;
//   	delete t7b;
//
//	LOG_INFO(logger, "test token.deepCopy()...");
//	// control token
//   	Token *token8 = new Token(*props,true);
//   	Token *token8cp = token8->deepCopy();
//   	CPPUNIT_ASSERT(token8 != token8cp);
//   	token8->getProperties().put("key1","valueXXX");
//   	CPPUNIT_ASSERT(token8->getProperties().get("key1").compare("valueXXX")==0);
//   	CPPUNIT_ASSERT(token8cp->getProperties().get("key1").compare("value1")==0);
//    LOG_INFO(logger, *token8cp);
//    delete token8cp;
//    LOG_INFO(logger, *token8);
//    CPPUNIT_ASSERT (token8);
//   	delete token8;
//   	// data token
//	Data* data9 = new Data("<data><x>1</x><y>2</y></data>");
//   	Token *token9 = new Token(*props,data9);
//   	Token *token9cp = token9->deepCopy();
//   	LOG_INFO(logger, *token9);
//   	CPPUNIT_ASSERT(token9 != token9cp);
//   	CPPUNIT_ASSERT(token9->getData() != token9cp->getData());
//   	// ToDo: Getting 'xercesc_2_7::DOMException' if deleting data9 here...
////   	delete data9;
////   	CPPUNIT_ASSERT(token9cp);
//   	delete token9;
//   	CPPUNIT_ASSERT(token9cp);
//   	LOG_INFO(logger, *token9cp);
//   	delete token9cp;
//   	
//	delete props;
	LOG_INFO(logger, "============== END TOKEN TEST =============");
}
