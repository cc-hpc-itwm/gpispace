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
using namespace gwdl;
 
#if defined(LIBXML_XPATH_ENABLED) && defined(LIBXML_SAX1_ENABLED)

void testXPathEvaluator() {
	cout << "============== BEGIN XPathEvaluation TEST =============" << endl;
	
	xmlInitParser();
	LIBXML_TEST_VERSION
    
	XPathEvaluator* xpathP = new XPathEvaluator("<data><min>15</min><max>33</max><step>1</step></data>");
	string str1 = "$x/a";
	string str2 = xpathP->expandVariables(str1);
	cout << "XPathEvaluator::expandVariables(\"$x/a\")=" << str2 << endl;
	assert (str2.compare("/tokens/x/a")==0);
	
	assert(!xpathP->evalCondition("count(/data/min) = 2"));
    assert(xpathP->evalCondition("count(/data/min) = 1"));
    assert(xpathP->evalCondition("/data/min = 15"));
    assert(xpathP->evalCondition("/data/min > 10"));
    assert(!xpathP->evalCondition("/data/min > 15"));
    assert(xpathP->evalCondition("NOT A VALID XPATH EXPRESSION")==-1);
    assert(strcmp(xpathP->evalExpression("/data/min"),"15")==0);
    assert(strcmp(xpathP->evalExpression("/data/min + /data/max"),"48")==0);
    delete xpathP;
    
	xpathP = new XPathEvaluator("<tokens><x><a>5</a></x></tokens>");
    assert(xpathP->evalCondition("/tokens/x/a = 5"));
    delete xpathP;

    xmlCleanupParser();

	cout << "============== END XPathEvaluation TEST =============" << endl;
}


void testXPathEvaluatorContextCache() {
	cout << "============== BEGIN XPathEvaluatorContextCache TEST =============" << endl;
	
	xmlInitParser();
	LIBXML_TEST_VERSION
    
	// create transition with input places and input tokens
    Transition *t0 = new Transition("");
    Place *p0 = new Place("");
    Place *p1 = new Place("");
    Edge *e0 = new Edge(p0,"input0");
    Edge *e1 = new Edge(p1,"input1");
    t0->addReadEdge(e0);
    t0->addInEdge(e1);
    Token* d0 = new Token(true);
    p0->addToken(d0);
    Token* d1 = new Token(false);
    p1->addToken(d1);
    
    XPathEvaluator* xpathP = new XPathEvaluator(t0,1);
    xpathP->evalCondition("count(/tokens) = 1");
    xmlXPathContextPtr contextP = xpathP->getXmlContext();
    delete xpathP;
    
    xpathP = new XPathEvaluator(t0,1);
    xpathP->evalCondition("count(/tokens) = 1");
    assert(contextP==xpathP->getXmlContext());
    delete xpathP;
    

	xmlCleanupParser();
	cout << "============== END XPathEvaluatorContextCache TEST =============" << endl;
}

#else
void testXPathEvaluator() {
    cerr << "XPath support not compiled in" << endl;
    assert(false);
}
#endif
