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
	CPPUNIT_ASSERT_MESSAGE("unlocked token", !token->isLocked());
	
	LOG_INFO(logger, "-------------- test constructor Token(true)... --------------");
	token.reset(new Token(Token::CONTROL_TRUE));
	LOG_INFO(logger, "\n" << *token);
	CPPUNIT_ASSERT_MESSAGE("control token1", !token->isData());
	CPPUNIT_ASSERT_MESSAGE("control token1 true", token->getControl());
	CPPUNIT_ASSERT_MESSAGE("unlocked token", !token->isLocked());

	LOG_INFO(logger, "-------------- test constructor Token(false)... --------------");
	token.reset(new Token(Token::CONTROL_FALSE));
	LOG_INFO(logger, "\n" << *token);
	CPPUNIT_ASSERT(!token->getControl());
	CPPUNIT_ASSERT_MESSAGE("unlocked token", !token->isLocked());

	LOG_INFO(logger, "-------------- test constructor Token(properties, control)... --------------");
	Properties::ptr_t propsP(new Properties());
	propsP->put("key1","value1");
	propsP->put("key2","value2");
	token.reset(new Token(propsP, Token::CONTROL_TRUE));
	LOG_INFO(logger, "\n" << *token);
	CPPUNIT_ASSERT_EQUAL_MESSAGE("number properties", (size_t) 2, token->getProperties()->size() );
	CPPUNIT_ASSERT(token->getControl());
	CPPUNIT_ASSERT_MESSAGE("unlocked token", !token->isLocked());
	
	LOG_INFO(logger, "-------------- test constructor Token(data)... --------------");
	Data::ptr_t dataP = Data::ptr_t(new Data("<test>15</test>"));
	token.reset(new Token(dataP));
	LOG_INFO(logger, "\n" << *token);
	CPPUNIT_ASSERT(token->isData());
	CPPUNIT_ASSERT_EQUAL(string("<test>15</test>"), token->getData()->getContent());
	CPPUNIT_ASSERT_MESSAGE("unlocked token", !token->isLocked());
	
	LOG_INFO(logger, "-------------- test constructor Token(properties, data)... --------------");
	token.reset(new Token(propsP,dataP));
	LOG_INFO(logger, "\n" << *token);
	CPPUNIT_ASSERT(token->isData());
	CPPUNIT_ASSERT_EQUAL(string("<test>15</test>"), token->getData()->getContent());
	CPPUNIT_ASSERT_EQUAL_MESSAGE("number properties", (size_t) 2, token->getProperties()->size() );
	CPPUNIT_ASSERT_MESSAGE("unlocked token", !token->isLocked());

	LOG_INFO(logger, "-------------- test deepCopy() --------------");
	Token::ptr_t token2 = token->deepCopy();
	LOG_INFO(logger, "pointer to original token " << token);
	LOG_INFO(logger, "pointer to cloned token " << token2);
	LOG_INFO(logger, "original token:\n" << *token);
	LOG_INFO(logger, "cloned token:\n" << *token2);
	///ToDo: CCPUNIT_ASSERTS!

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
	} catch (const WorkflowFormatException &e) {
		LOG_INFO(logger, e.what());
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
	} catch (const WorkflowFormatException &e) {
		LOG_INFO(logger, "WorkflowFormatException: " << e.what());
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
	} catch (const WorkflowFormatException &e) {
		LOG_INFO(logger, e.what());
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

	LOG_INFO(logger, "============== END TOKEN TEST =============");
}
