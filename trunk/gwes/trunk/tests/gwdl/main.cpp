/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#include <iostream>
#include <ostream>
// test
#include "TestData.h"
#include "TestToken.h"
#include "TestProperties.h"
#include "TestPlace.h"
#include "TestOperation.h"
#include "TestTransition.h"
#include "TestWorkflow.h"
//gwdl
#include <gwdl/XMLUtils.h>
//fhglog
#include <fhglog/fhglog.hpp>

using namespace std;
using namespace gwdl;
using namespace fhg::log;
XERCES_CPP_NAMESPACE_USE

#define TEST_ALL true

void testParser();
int testBuilder();

int main() 
{
	// logger
	logger_t logger(getLogger("gwdl"));
	logger.setLevel(LogLevel::INFO);
	logger.addAppender(Appender::ptr_t(new StreamAppender("console")))->setFormat(Formatter::Short());

	XMLUtils* xmlutils = XMLUtils::Instance();
	LOG_INFO(logger, "xmlutils singleton instantiated: " << xmlutils);;

	if(TEST_ALL)
	{		
		// data
		testData();

		// properties
		testProperties();

		// token
		testToken();

		// place
		testPlace();

		// operation
		testOperation();

		// transition
		testTransition();

		// workflow
		testWorkflow();
	}
	testParser();
	return 0;
}
