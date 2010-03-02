/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#include <iostream>
#include <ostream>
//tests
#include "TestXMLUtils.h"
//fhglog
#include <fhglog/fhglog.hpp>
using namespace std;
using namespace gwdl;
using namespace fhg::log;
using namespace gwdl::tests;

CPPUNIT_TEST_SUITE_REGISTRATION( gwdl::tests::TestXMLUtils );

void TestXMLUtils::testStartsWith() {
	CPPUNIT_ASSERT(XMLUtils::startsWith(string("<data>bla</data>"),"<data")); 
	CPPUNIT_ASSERT(!XMLUtils::startsWith(string("XXX<data>bla</data>"),"<data")); 
}

void TestXMLUtils::testEndsWith() {
	CPPUNIT_ASSERT(XMLUtils::endsWith(string("<data>bla</data>"),"</data>")); 
	CPPUNIT_ASSERT(!XMLUtils::endsWith(string("<data>bla</data>XXX"),"</data>")); 
}

void TestXMLUtils::testGetText() {
	logger_t logger(getLogger("gwdl"));
	LOG_INFO(logger, "============== BEGIN XMLUTILS::GETTEXT TEST =============");
	string str = string("<data><bla>test<i> 23</i></bla></data>");
	LOG_INFO(logger, "xml: "<< str);
	string str2 = XMLUtils::Instance()->getText(str);
	LOG_INFO(logger, "getText(xml): " << str2);
	CPPUNIT_ASSERT_EQUAL_MESSAGE("getText", string("test 23"), str2);
	LOG_INFO(logger, "============== END XMLUTILS::GETTEXT TEST =============");
}
