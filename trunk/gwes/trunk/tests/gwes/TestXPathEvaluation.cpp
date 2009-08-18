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
#include <ctime>

using namespace std;
using namespace gwes;
 
#if defined(LIBXML_XPATH_ENABLED) && defined(LIBXML_SAX1_ENABLED)

void testXPathEvaluator() {
	cout << "============== BEGIN XPathEvaluation TEST =============" << endl;
	
	xmlInitParser();
	LIBXML_TEST_VERSION
    
	XPathEvaluator* xpathP = new XPathEvaluator("<data><min>15</min><max>33</max><step>1</step></data>");
	assert(!xpathP->evalCondition("count(/data/min) = 2"));
    assert(xpathP->evalCondition("count(/data/min) = 1"));
    assert(xpathP->evalCondition("/data/min = 15"));
    assert(xpathP->evalCondition("/data/min > 10"));
    assert(!xpathP->evalCondition("/data/min > 15"));
    assert(xpathP->evalCondition("NOT A VALID XPATH EXPRESSION")==-1);
    assert(strcmp(xpathP->evalExpression("/data/min"),"15")==0);
    assert(strcmp(xpathP->evalExpression("/data/min + /data/max"),"48")==0);
    delete xpathP;
    
	xpathP = new XPathEvaluator("<token><x><a>5</a></x></token>");
    assert(xpathP->evalCondition("/token/x/a = 5"));
    delete xpathP;

    xmlCleanupParser();

	cout << "============== END XPathEvaluation TEST =============" << endl;
}

#else
void testXPathEvaluator() {
    cerr << "XPath support not compiled in" << endl;
    assert(false);
}
#endif
