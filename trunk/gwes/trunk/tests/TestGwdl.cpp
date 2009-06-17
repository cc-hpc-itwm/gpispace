/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#include <iostream>
#include <ostream>
//gwdl
#include "../gworkflowdl_cpp/src/XMLUtils.h"
// test
#include "TestData.h"
#include "TestToken.h"
#include "TestProperties.h"
#include "TestPlace.h"
#include "TestOperation.h"
#include "TestTransition.h"
#include "TestWorkflow.h"
#include "TestGwdl.h"

using namespace std;
using namespace gwdl;
XERCES_CPP_NAMESPACE_USE

#define TEST_ALL true

void testParser();
int testBuilder();
 
int main() 
{
	XMLUtils* xmlutils = XMLUtils::Instance();

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
