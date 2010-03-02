/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#include <iostream>
#include <ostream>
//tests
#include "TestProperties.h"
//gwdl
#include <gwdl/Libxml2Builder.h>
//fhglog
#include <fhglog/fhglog.hpp>

using namespace std;
using namespace gwdl;
using namespace fhg::log;
using namespace gwdl::tests;

CPPUNIT_TEST_SUITE_REGISTRATION( gwdl::tests::PropertiesTest );

void PropertiesTest::testProperties()
{
	logger_t logger(logger_t(getLogger("gwdl")));
	LOG_INFO(logger, "============== BEGIN PROPERTIES TEST =============");
	Libxml2Builder builder;
	
	LOG_INFO(logger, "-------------- test put properties --------------");
	Properties::ptr_t propsP(new Properties());
	propsP->put("key1","value1");
	propsP->put("key2","value2");
	propsP->put("key3","value3");
	propsP->put("key4","");
	LOG_INFO(logger, "\n" << *propsP);
	CPPUNIT_ASSERT(propsP->size()==4);
	CPPUNIT_ASSERT_EQUAL_MESSAGE("prop1", string("value1"), propsP->get("key1"));
	CPPUNIT_ASSERT_EQUAL_MESSAGE("prop2", string("value2"), propsP->get("key2"));
	CPPUNIT_ASSERT_EQUAL_MESSAGE("prop3", string("value3"), propsP->get("key3"));
	CPPUNIT_ASSERT_EQUAL_MESSAGE("prop4", string(""), propsP->get("key4"));
	string str1("<property name=\"key1\">value1</property>\n<property name=\"key2\">value2</property>\n<property name=\"key3\">value3</property>\n<property name=\"key4\"/>\n");
	string str2 = builder.serializeProperties(*propsP);
	CPPUNIT_ASSERT_EQUAL_MESSAGE("serialized property", str1, str2);
	
	LOG_INFO(logger, "-------------- test remove properties... --------------");
	propsP->remove("key1");
	LOG_INFO(logger, "\n" << *propsP);
    CPPUNIT_ASSERT_EQUAL_MESSAGE("number of properties", (size_t) 3, propsP->size());
	
	LOG_INFO(logger, "-------------- test overwriting of properties ... --------------");
	propsP->put("key3","value3b");
	LOG_INFO(logger, "\n" << *propsP);
	CPPUNIT_ASSERT_EQUAL_MESSAGE("overwritten property", string("value3b"), propsP->get("key3"));

	LOG_INFO(logger, "============== END PROPERTIES TEST =============");
}
