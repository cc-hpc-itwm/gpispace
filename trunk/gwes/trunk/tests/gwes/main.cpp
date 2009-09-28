/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
// tests
#include "TestXPathEvaluation.h"
#include "TestGWES.h"
#include "TestSdpa2Gwes.h"
#include "TestWorkflow.h"
#include "TestPreStackProWorkflow.h"
// gwes
#include <gwes/Utils.h>
//fhglog
#include <fhglog/fhglog.hpp>
// std
#include <iostream>
#include <ostream>

using namespace std;
using namespace gwdl;
using namespace gwes;
using namespace fhg::log;

#define TEST_ALL true

int main() 
{
	// logger
	logger_t logger(getLogger("gwes"));
	logger.setLevel(LogLevel::INFO);
	logger.addAppender(Appender::ptr_t(new StreamAppender("console")))->setFormat(Formatter::Short());
	LOG_INFO(logger, "########################### BEGIN OF ALL GWES TESTS ###########################");

	gwes::GWES gwes; 
	Workflow workflow;
	Place* placeP;
	Token* tokenP;

	// test basic GWES features 
	testGWES(gwes);


	if(TEST_ALL) {
		
		// test XPathEvaluator
		testXPathEvaluator();
		///ToDo: FixMe Test commented out because cache in XPathEvaluator is currently deactivated
		//testXPathEvaluatorContextCache();

		// test SDPA2GWES communication
		testSdpa2Gwes();
		testGwes2Sdpa();

		// test various workflows
		
		// simple.gwdl
		workflow = testWorkflow(Utils::expandEnv("${GWES_CPP_HOME}/workflows/test/simple.gwdl"),gwes);
		assert(gwes.getStatusAsString(workflow)=="COMPLETED");
		assert( workflow.getProperties().get("occurrence.sequence").compare("t") == 0 );

		// split-token.gwdl
		workflow = testWorkflow(Utils::expandEnv("${GWES_CPP_HOME}/workflows/test/split-token.gwdl"),gwes);
		assert(gwes.getStatusAsString(workflow)=="COMPLETED");
		assert( workflow.getProperties().get("occurrence.sequence").compare("joinSplitTokens") == 0 );

		placeP = workflow.getPlace("value"); 
		assert(placeP->getTokenNumber() == 1);
		tokenP = placeP->getTokens().front();
		assert(tokenP->isData());
		LOG_INFO(logger, *(tokenP->getData()));
		// ToDo: improve pretty printing (too much spaces).
		assert(tokenP->getData()->toString()->compare("<data>\n  <value>\n          <x>15</x>\n          <y>23</y>\n        </value>\n  <value>\n          <x>16</x>\n          <y>24</y>\n        </value>\n</data>") == 0);

		placeP = workflow.getPlace("x"); 
		assert(placeP->getTokenNumber() == 1);
		tokenP = placeP->getTokens().front();
		assert(tokenP->isData());
		LOG_INFO(logger, *(tokenP->getData()));
		assert(tokenP->getData()->toString()->compare("<data>\n  <x>15</x>\n  <x>16</x>\n</data>") == 0);

	    placeP = workflow.getPlace("y"); 
		assert(placeP->getTokenNumber() == 1);
		tokenP = placeP->getTokens().front();
		assert(tokenP->isData());
		LOG_INFO(logger, *(tokenP->getData()));
		assert(tokenP->getData()->toString()->compare("<data>\n  <y>23</y>\n  <y>24</y>\n</data>") == 0);

		// exclusive-choice.gwdl
		workflow = testWorkflow(Utils::expandEnv("${GWES_CPP_HOME}/workflows/test/exclusive-choice.gwdl"), gwes);
		assert(gwes.getStatusAsString(workflow)=="COMPLETED");
		assert( workflow.getProperties().get("occurrence.sequence").compare("B") == 0 );
		assert(workflow.getPlace("end_A")->getTokenNumber() == 0);
		placeP = workflow.getPlace("end_B"); 
		assert(placeP->getTokenNumber() == 1);
		tokenP = placeP->getTokens().front();
		assert(!tokenP->isData());
		assert(tokenP->getControl());

		// condition-test.gwdl
		workflow = testWorkflow(Utils::expandEnv("${GWES_CPP_HOME}/workflows/test/condition-test.gwdl"), gwes);
		assert(gwes.getStatusAsString(workflow)=="COMPLETED");
		assert( workflow.getProperties().get("occurrence.sequence").compare("B A A") == 0 );
		
	    placeP = workflow.getPlace("end_A"); 
		assert(placeP->getTokenNumber() == 2);
		tokenP = placeP->getTokens()[0];
		assert(tokenP->isData());
		LOG_INFO(logger, *(tokenP->getData()));
		assert(tokenP->getData()->toString()->compare("<data>\n  <x>6</x>\n</data>") == 0);
		tokenP = placeP->getTokens()[1];
		assert(tokenP->isData());
		LOG_INFO(logger, *(tokenP->getData()));
		assert(tokenP->getData()->toString()->compare("<data>\n  <x>7</x>\n</data>") == 0);
		
	    placeP = workflow.getPlace("end_B"); 
		assert(placeP->getTokenNumber() == 1);
		tokenP = placeP->getTokens()[0];
		assert(tokenP->isData());
		LOG_INFO(logger, *(tokenP->getData()));
		assert(tokenP->getData()->toString()->compare("<data>\n  <x>5</x>\n</data>") == 0);

		// control-loop.gwdl
		workflow = testWorkflow(Utils::expandEnv("${GWES_CPP_HOME}/workflows/test/control-loop.gwdl"),gwes);
		assert(gwes.getStatusAsString(workflow)=="COMPLETED");
		assert( workflow.getProperties().get("occurrence.sequence").compare("i_plus_plus i_plus_plus i_plus_plus i_plus_plus i_plus_plus i_plus_plus i_plus_plus i_plus_plus i_plus_plus i_plus_plus break") == 0 );
	    placeP = workflow.getPlace("end"); 
		assert(placeP->getTokenNumber() == 1);
		tokenP = placeP->getTokens()[0];
		assert(tokenP->isData());
		LOG_INFO(logger, *(tokenP->getData()));
		assert(tokenP->getData()->toString()->compare("<data>\n  <a>10</a>\n</data>") == 0);

		// Will not work on auto build because shell scripts are not installed there.
		//   workflow = testWorkflow(twd + "/concatenateIt.gwdl",gwes);
		//   assert(workflow.getProperties().get("status")=="COMPLETED");
		//   assert(gwes.getStatusAsString(workflow)=="COMPLETED");

		//   workflow = testWorkflow(twd + "/concatenateIt_fail.gwdl",gwes);
		//   assert(gwes.getStatusAsString(workflow)=="TERMINATED");
		//

	}

	// Test of PSTM use case.
	workflow = testWorkflow(Utils::expandEnv("${GWES_CPP_HOME}/workflows/pstm-0.gwdl"),gwes);
	assert(gwes.getStatusAsString(workflow)=="COMPLETED");
	assert(workflow.getProperties().get("occurrence.sequence").compare("preStackTimeMigration") == 0);
	
	LOG_INFO(logger, "########################### END OF ALL GWES TESTS ###########################");

	return 0;
}

