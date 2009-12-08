/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#include <iostream>
#include <ostream>
//gwdl
#include <gwdl/Libxml2Builder.h>
//tests
#include "TestData.h"
//fhglog
#include <fhglog/fhglog.hpp>

using namespace std;
using namespace gwdl;
using namespace fhg::log;
using namespace gwdl::tests;

CPPUNIT_TEST_SUITE_REGISTRATION( gwdl::tests::DataTest );

void DataTest::testData()
{
	logger_t logger(getLogger("gwdl"));
	LOG_INFO(logger, "============== BEGIN DATA TEST =============");

	LOG_INFO(logger, "constructor test: empty data token... ");
	Data emptyData("",Data::TYPE_EMPTY);
	CPPUNIT_ASSERT_MESSAGE("Empty data", emptyData.getContent().empty());
	CPPUNIT_ASSERT_EQUAL_MESSAGE("Empty data type", (int) Data::TYPE_EMPTY, emptyData.getType());

	LOG_INFO(logger, "constructor test: data from string...");
	string str("<values><x>1</x><y>2</y></values>");
	Data data2(str);
	CPPUNIT_ASSERT_EQUAL_MESSAGE("data content", str, data2.getContent());

	LOG_INFO(logger, "testDeserializeSerialize()...");
	Libxml2Builder builder;
	str = string("<values><x>3</x><y>4</y></values>");
	Data::ptr_t dataP = builder.deserializeData(str);
	string str2 = builder.serializeData(*dataP);
	CPPUNIT_ASSERT_EQUAL(str, str2);

	LOG_INFO(logger, "testDeserializeSerialize() loop...");		
	for (unsigned int i=0; i<100; i++) {
		ostringstream oss;
		oss << "<i>" << i << "</i>";
		Data::ptr_t data2P = builder.deserializeData(oss.str());
		str2 = builder.serializeData(*data2P);
	}

	LOG_INFO(logger, "============== END DATA TEST =============");
}
