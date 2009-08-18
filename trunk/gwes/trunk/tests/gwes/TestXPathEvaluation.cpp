// test
#include "TestXPathEvaluation.h"
// gwes
#include <gwes/XPathEvaluator.h>
// std
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;
using namespace gwes;
 
#if defined(LIBXML_XPATH_ENABLED) && defined(LIBXML_SAX1_ENABLED)

/**
 * example refer to http://xmlsoft.org/examples/xpath1.c
 */
void testXPathEvaluator() {
	cout << "============== BEGIN XpathEvaluation TEST =============" << endl;
	
	xmlInitParser();
	LIBXML_TEST_VERSION
    
	XPathEvaluator* xpathP = new XPathEvaluator("<data><min>15</min><max>33</max><step>1</step></data>");
    assert(!xpathP->evalCondition("count(//data/min) = 2"));
    assert(xpathP->evalCondition("count(//data/min) = 1"));
    assert(xpathP->evalCondition("//data/min = 15"));
    assert(xpathP->evalCondition("//data/min > 10"));
    assert(!xpathP->evalCondition("//data/min > 15"));
    assert(xpathP->evalCondition("NOT A VALID XPATH EXPRESSION")==-1);
    assert(strcmp(xpathP->evalExpression("//data/min"),"15")==0);
    assert(strcmp(xpathP->evalExpression("//data/min + //data/max"),"48")==0);
    
    delete xpathP;
    
    xmlCleanupParser();

	cout << "============== END XpathEvaluation TEST =============" << endl;
}

#else
void testXPathEvaluator() {
    cerr << "XPath support not compiled in" << endl;
    assert(false);
}
#endif
