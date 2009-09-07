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
	// get logger
	LoggerApi logger(Logger::get("gwdl"));
	logger.setLevel(LogLevel::INFO);
	Appender::ptr_t appender = Appender::ptr_t(new StreamAppender());
	Formatter::ptr_t formatter = Formatter::ptr_t(Formatter::ShortFormatter());
	appender->setFormat(formatter);
	logger.addAppender(appender);

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
