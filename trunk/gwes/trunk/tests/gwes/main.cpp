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
// 
#include <iostream>
#include <ostream>

using namespace std;
using namespace gwdl;
using namespace gwes;

#define TEST_ALL true

int main() 
{
	cout << "########################### BEGIN OF ALL GWES TESTS ###########################" << endl;

	gwes::GWES gwes;
	string twd = getTestWorkflowDirectory();
	string wd = getWorkflowDirectory();
	Workflow workflow;
	Place* placeP;
	Token* tokenP;

	// test basic GWES features 
	testGWES(gwes);


	if(TEST_ALL) {
		
		// test XPathEvaluator
		testXPathEvaluator();
		testXPathEvaluatorContextCache();

		// test SDPA2GWES communication
		testSdpa2Gwes();
		testGwes2Sdpa();

		// test various workflows
		
		// simple.gwdl
		workflow = testWorkflow(twd + "/simple.gwdl",gwes);
		assert(gwes.getStatusAsString(workflow)=="COMPLETED");
		assert( workflow.getProperties().get("occurrence.sequence").compare("t") == 0 );

		// split-token.gwdl
		workflow = testWorkflow(twd + "/split-token.gwdl",gwes);
		assert(gwes.getStatusAsString(workflow)=="COMPLETED");
		assert( workflow.getProperties().get("occurrence.sequence").compare("joinSplitTokens") == 0 );

		placeP = workflow.getPlace("value"); 
		assert(placeP->getTokenNumber() == 1);
		tokenP = placeP->getTokens().front();
		assert(tokenP->isData());
		cout << *(tokenP->getData()) << endl;
		// ToDo: improve pretty printing (too much spaces).
		assert(tokenP->getData()->toString()->compare("<data>\n  <value>\n          <x>15</x>\n          <y>23</y>\n        </value>\n  <value>\n          <x>16</x>\n          <y>24</y>\n        </value>\n</data>") == 0);

		placeP = workflow.getPlace("x"); 
		assert(placeP->getTokenNumber() == 1);
		tokenP = placeP->getTokens().front();
		assert(tokenP->isData());
		cout << *(tokenP->getData()) << endl;
		assert(tokenP->getData()->toString()->compare("<data>\n  <x>15</x>\n  <x>16</x>\n</data>") == 0);

	    placeP = workflow.getPlace("y"); 
		assert(placeP->getTokenNumber() == 1);
		tokenP = placeP->getTokens().front();
		assert(tokenP->isData());
		cout << *(tokenP->getData()) << endl;
		assert(tokenP->getData()->toString()->compare("<data>\n  <y>23</y>\n  <y>24</y>\n</data>") == 0);

		// exclusive-choice.gwdl
		workflow = testWorkflow(twd + "/exclusive-choice.gwdl", gwes);
		assert(gwes.getStatusAsString(workflow)=="COMPLETED");
		assert( workflow.getProperties().get("occurrence.sequence").compare("B") == 0 );
		assert(workflow.getPlace("end_A")->getTokenNumber() == 0);
		placeP = workflow.getPlace("end_B"); 
		assert(placeP->getTokenNumber() == 1);
		tokenP = placeP->getTokens().front();
		assert(!tokenP->isData());
		assert(tokenP->getControl());

		// condition-test.gwdl
		workflow = testWorkflow(twd + "/condition-test.gwdl", gwes);
		assert(gwes.getStatusAsString(workflow)=="COMPLETED");
		assert( workflow.getProperties().get("occurrence.sequence").compare("B A A") == 0 );
		
	    placeP = workflow.getPlace("end_A"); 
		assert(placeP->getTokenNumber() == 2);
		tokenP = placeP->getTokens()[0];
		assert(tokenP->isData());
		cout << *(tokenP->getData()) << endl;
		assert(tokenP->getData()->toString()->compare("<data>\n  <x>6</x>\n</data>") == 0);
		tokenP = placeP->getTokens()[1];
		assert(tokenP->isData());
		cout << *(tokenP->getData()) << endl;
		assert(tokenP->getData()->toString()->compare("<data>\n  <x>7</x>\n</data>") == 0);
		
	    placeP = workflow.getPlace("end_B"); 
		assert(placeP->getTokenNumber() == 1);
		tokenP = placeP->getTokens()[0];
		assert(tokenP->isData());
		cout << *(tokenP->getData()) << endl;
		assert(tokenP->getData()->toString()->compare("<data>\n  <x>5</x>\n</data>") == 0);

		// control-loop.gwdl
		workflow = testWorkflow(twd + "/control-loop.gwdl",gwes);
		assert(gwes.getStatusAsString(workflow)=="COMPLETED");
		assert( workflow.getProperties().get("occurrence.sequence").compare("i_plus_plus i_plus_plus i_plus_plus i_plus_plus i_plus_plus i_plus_plus i_plus_plus i_plus_plus i_plus_plus i_plus_plus break") == 0 );
	    placeP = workflow.getPlace("end"); 
		assert(placeP->getTokenNumber() == 1);
		tokenP = placeP->getTokens()[0];
		assert(tokenP->isData());
		cout << *(tokenP->getData()) << endl;
		assert(tokenP->getData()->toString()->compare("<data>\n  <a>10</a>\n</data>") == 0);

		// Will not work on auto build because shell scripts are not installed there.
		//   workflow = testWorkflow(twd + "/concatenateIt.gwdl",gwes);
		//   assert(workflow.getProperties().get("status")=="COMPLETED");
		//   assert(gwes.getStatusAsString(workflow)=="COMPLETED");

		//   workflow = testWorkflow(twd + "/concatenateIt_fail.gwdl",gwes);
		//   assert(gwes.getStatusAsString(workflow)=="TERMINATED");
		//

	}

//	// Test of PSTM use case.
//	workflow = testWorkflow(wd + "/pstm-1_withInputTokens.gwdl",gwes);
//	// workflow = testWorkflow(wd + "/pstm-calculateOffsetVolume_withInputTokens.gwdl",gwes);
//	assert(gwes.getStatusAsString(workflow)=="COMPLETED");
	
	cout << "########################### END OF ALL GWES TESTS ###########################" << endl;

	return 0;
}

