// test
#include "TestXPathEvaluation.h"
// gwes
#include <gwes/XPathEvaluator.h>
//fhglog
#include <fhglog/fhglog.hpp>
// std
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fstream>
#include <ctime>

using namespace std;
using namespace fhg::log;
using namespace gwes;
using namespace gwdl;
using namespace gwes::tests;

CPPUNIT_TEST_SUITE_REGISTRATION( gwes::tests::XPathEvaluationTest );

#if defined(LIBXML_XPATH_ENABLED) && defined(LIBXML_SAX1_ENABLED)

void XPathEvaluationTest::testXPathEvaluator() {
	logger_t logger(getLogger("gwes"));

	LOG_INFO(logger, "============== BEGIN XPathEvaluation TEST =============");
	
	xmlInitParser();
	LIBXML_TEST_VERSION
    
	XPathEvaluator* xpathP = new XPathEvaluator("<data><min>15</min><max>33</max><step>1</step></data>");
	string str;
	string str2;

	// test eval conditions
	str = "/data/min + 1 = 16";
    CPPUNIT_ASSERT(xpathP->evalCondition(str)==1);
    str = "count(/data/min) = 2";
    CPPUNIT_ASSERT(xpathP->evalCondition(str)==0);
    str = "count(/data/min) = 1";
    CPPUNIT_ASSERT(xpathP->evalCondition(str)==1);
    str = "/data/min = 15";
    CPPUNIT_ASSERT(xpathP->evalCondition(str)==1);
    str = "/data/min > 10";
    CPPUNIT_ASSERT(xpathP->evalCondition(str)==1);
    str = "/data/min > 15";
    CPPUNIT_ASSERT(xpathP->evalCondition(str)==0);
    str = "NOT A VALID XPATH EXPRESSION";
    CPPUNIT_ASSERT(xpathP->evalCondition(str)==-1);
    
    // test eval expressions
    str = "/data/min"; str2 = "15";
    CPPUNIT_ASSERT( xpathP->evalExpression(str).compare(str2) ==0 );
    str = "/data/min + /data/max"; str2 = "48";
    CPPUNIT_ASSERT( xpathP->evalExpression(str).compare(str2) ==0 );
    // ToDo: should be <data><min>...</data> instead?
    str = "/data"; str2 = "15331";
    CPPUNIT_ASSERT( xpathP->evalExpression(str).compare(str2) ==0 );
    delete xpathP;
    
	xpathP = new XPathEvaluator("<data><x><a>5</a></x><bla>wörter</bla></data>");
	str = "/data/x/a = 5";
    CPPUNIT_ASSERT(xpathP->evalCondition(str)==1);
    str = "$x/a = 5";
    CPPUNIT_ASSERT(xpathP->evalCondition(str)==1);
    
    // test eval expression 2 xml

    // node set
	str = "/data/x"; str2 = "<data>\n  <x><a>5</a></x>\n</data>";
    CPPUNIT_ASSERT( xpathP->evalExpression2Xml(str).compare(str2) == 0 );

    // boolean
	str = "/data/x/a = 5";	str2 = "<data><boolean xmlns=\"\">true</boolean></data>";
    CPPUNIT_ASSERT( xpathP->evalExpression2Xml(str).compare(str2) == 0 );
    str = "/data/x/a = 4";	str2 = "<data><boolean xmlns=\"\">false</boolean></data>";
    CPPUNIT_ASSERT( xpathP->evalExpression2Xml(str).compare(str2) == 0 );
    
    // number
	str = "number(/data/x/a)"; 	str2 = "<data><number xmlns=\"\">5</number></data>";
    CPPUNIT_ASSERT( xpathP->evalExpression2Xml(str).compare(str2) == 0 );
	
	// string
	str = "normalize-space(/data/bla)";	str2 = "<data><string xmlns=\"\">wörter</string></data>";
    CPPUNIT_ASSERT( xpathP->evalExpression2Xml(str).compare(str2) == 0 );
    
    delete xpathP;

    // test conditions of pstm test case
    xpathP = new XPathEvaluator("<data><OffsetParameters><min>15</min><max>33</max><step>1</step></OffsetParameters><HasNext>true</HasNext><i>0</i></data>");
    // wrong syntax!
	str = "$HasNext = true";
    CPPUNIT_ASSERT(xpathP->evalCondition(str)==0);
	str = "$HasNext";
    CPPUNIT_ASSERT(xpathP->evalCondition(str)==1);
    str = "not($HasNext)";
    CPPUNIT_ASSERT(xpathP->evalCondition(str)==0);
    // correct syntax
	str = "$HasNext = 'true'";
    CPPUNIT_ASSERT(xpathP->evalCondition(str)==1);
    str = "$HasNext = 'false'";
    CPPUNIT_ASSERT(xpathP->evalCondition(str)==0);

    delete xpathP;

    // test conditions of pstm test case
    xpathP = new XPathEvaluator("<data><OffsetParameters><min>15</min><max>33</max><step>1</step></OffsetParameters><HasNext>false</HasNext><i>18</i></data>");
    // wrong syntax!
	str = "$HasNext = true";
    CPPUNIT_ASSERT(xpathP->evalCondition(str)==0);
	str = "$HasNext";
    CPPUNIT_ASSERT(xpathP->evalCondition(str)==1);
    str = "not($HasNext)";
    CPPUNIT_ASSERT(xpathP->evalCondition(str)==0);
    // correct syntax
    str = "$HasNext = 'true'";
    CPPUNIT_ASSERT(xpathP->evalCondition(str)==0);
    str = "$HasNext = 'false'";
    CPPUNIT_ASSERT(xpathP->evalCondition(str)==1);

    delete xpathP;

    xmlCleanupParser();

	LOG_INFO(logger, "============== END XPathEvaluation TEST =============");
}


void XPathEvaluationTest::testXPathEvaluatorContextCache() {
	logger_t logger(getLogger("gwes"));
	LOG_INFO(logger, "============== BEGIN XPathEvaluatorContextCache TEST =============");
	
	xmlInitParser();
	LIBXML_TEST_VERSION
    
	// create transition with input places and input tokens
    Transition *t0 = new Transition("");
    Place::ptr_t p0 = Place::ptr_t(new Place(""));
    Place::ptr_t p1 = Place::ptr_t(new Place(""));
    Edge::ptr_t e0 = Edge::ptr_t(new Edge(p0,"input0"));
    Edge::ptr_t e1 = Edge::ptr_t(new Edge(p1,"input1"));
    t0->addReadEdge(e0);
    t0->addInEdge(e1);
    Token::ptr_t d0 = Token::ptr_t(new Token());
    p0->addToken(d0);
    Token::ptr_t d1 = Token::ptr_t(new Token(Token::CONTROL_FALSE));
    p1->addToken(d1);
    
    TransitionOccurrence* toP = new TransitionOccurrence(t0);
    XPathEvaluator* xpathP = new XPathEvaluator(toP,1);
    string str = "count(/data) = 1";
    xpathP->evalCondition(str);
    xmlXPathContextPtr contextP = xpathP->getXmlContext();
    delete xpathP;
    
    xpathP = new XPathEvaluator(toP,1);
    str = "1 = 1";
    xpathP->evalCondition(str);
    CPPUNIT_ASSERT(contextP==xpathP->getXmlContext());
    delete xpathP;
    

	xmlCleanupParser();
	LOG_INFO(logger, "============== END XPathEvaluatorContextCache TEST =============");
}

#else
void XPathEvaluationTest::testXPathEvaluator() {
	logger_t logger(getLogger("gwes"));
    LOG_FATAL(logger, "XPath support not compiled in");
    CPPUNIT_ASSERT(false);
}
#endif
