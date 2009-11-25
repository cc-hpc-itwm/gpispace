/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#include <iostream>
#include <ostream>
// cppunit includes
#include <cppunit/extensions/TestFactoryRegistry.h>
#include <cppunit/CompilerOutputter.h>
#include <cppunit/XmlOutputter.h>
#include <cppunit/TestResult.h>
#include <cppunit/TestResultCollector.h>
#include <cppunit/TextTestRunner.h>
#include <cppunit/ui/text/TestRunner.h>
#include <cppunit/TextTestProgressListener.h>
#include <cppunit/BriefTestProgressListener.h>
#include <cppunit/TestFailure.h>
#include <cppunit/Test.h>

#include <cppunit/XmlOutputterHook.h>
#include <cppunit/tools/XmlElement.h>
#include <cppunit/tools/StringTools.h>
#include <cppunit/tools/XmlDocument.h>
#include <cppunit/TestFailure.h>
#include <cppunit/SourceLine.h>
#include <cppunit/Exception.h>
#include <cppunit/Message.h>
//gwdl
#include <gwdl/XMLUtils.h>
//fhglog
#include <fhglog/fhglog.hpp>

using namespace std;
using namespace gwdl;
using namespace fhg::log;

#define TEST_ALL true

void testParser();
int testBuilder();

int main() 
{
	// logger
    fhg::log::Configurator::configure();
	logger_t logger(getLogger("gwdl"));
//	logger.setLevel(LogLevel::DEBUG);
	//logger.addAppender(Appender::ptr_t(new StreamAppender("console")))->setFormat(Formatter::Short());
	
	LOG_INFO(logger, "########################### BEGIN OF ALL GWDL TESTS ###########################");
	
	XMLUtils* xmlutils = XMLUtils::Instance();
	LOG_INFO(logger, "xmlutils singleton instantiated: " << xmlutils);;

	// Informiert Test-Listener ueber Testresultate
	CPPUNIT_NS::TestResult testresult;
	CPPUNIT_NS::TestResultCollector collectedresults;
	testresult.addListener (&collectedresults);

	// Test-Suite ueber die Registry im Test-Runner einfuegen
	CPPUNIT_NS :: TestRunner runner;
	runner.addTest (CPPUNIT_NS::TestFactoryRegistry :: getRegistry ().makeTest ());

	//CPPUNIT_NS::CompilerOutputter *outputter = new CPPUNIT_NS::CompilerOutputter(&runner.result(), std::cout);
	//outputter->setLocationFormat("%p(%l) : ");
	//outputter->setNoWrap();
	//runner.setOutputter(outputter);

	std::cout << "running testsuite" << std::endl;

	runner.run (testresult);

	// print and save results
	std::ofstream outStream("gwdl_out.xml");
	//outStream= new std::ofstream("out.xml", std::ios::app );
	CPPUNIT_NS::XmlOutputter xmloutputter (&collectedresults, outStream);
	//CPPUNIT_NS::XmlOutputter xmloutputter (&collectedresults, std::cout);
	xmloutputter.write ();

	// text mode
	//CPPUNIT_NS::CompilerOutputter compileroutputter (&collectedresults, std::cout);
	//compileroutputter.write ();
	
	LOG_INFO(logger, "deleting xmlutils singleton: " << xmlutils);;
	delete xmlutils;

	LOG_INFO(logger, "########################### END OF ALL GWDL TESTS ###########################");

	return collectedresults.wasSuccessful () ? 0 : 1;
}
