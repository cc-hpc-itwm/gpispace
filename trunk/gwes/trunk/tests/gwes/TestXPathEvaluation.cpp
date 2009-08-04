// test
#include "TestXPathEvaluation.h"
// libxml
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpath.h>
#include <libxml/xpathInternals.h>
// std
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;
 
#if defined(LIBXML_XPATH_ENABLED) && defined(LIBXML_SAX1_ENABLED)

/**
 * example refer to http://xmlsoft.org/examples/xpath1.c
 */
void testXPathEvaluation() {
	cout << "============== BEGIN XpathEvaluation TEST =============" << endl;
    cout << "initialization of xml parser..." << endl;
    xmlInitParser();
    LIBXML_TEST_VERSION
    
    assert(!xPathCondition("count(//data/min) = 2", "<data><min>15</min><max>33</max><step>1</step></data>"));
    assert(xPathCondition("count(//data/min) = 1", "<data><min>15</min><max>33</max><step>1</step></data>"));
    assert(xPathCondition("//data/min = 15", "<data><min>15</min><max>33</max><step>1</step></data>"));
    assert(xPathCondition("//data/min > 10", "<data><min>15</min><max>33</max><step>1</step></data>"));
    assert(!xPathCondition("//data/min > 15", "<data><min>15</min><max>33</max><step>1</step></data>"));
//
//    // result output
//    printXmlNodes(xpathObjP->nodesetval);

//    xmlXPathFreeObject(xpathObjP);
//    xmlXPathFreeContext(xpathCtxP); 
//    xmlFreeDoc(xmlDocP); 

    // cleanup
    cout << "cleanup parser..." << endl;
    xmlCleanupParser();
	cout << "============== END XpathEvaluation TEST =============" << endl;
}

bool xPathCondition(char* xPathExprChar, char* xmlContextChar) {
    // create xpath expression
	const xmlChar* xPathExpressionP = xmlCharStrdup(xPathExprChar);
   
    // create context
    const xmlChar* xmlCharP = xmlCharStrdup(xmlContextChar);
    xmlDocPtr xmlDocP = xmlParseDoc(xmlCharP);
    xmlXPathContextPtr xpathCtxP = xmlXPathNewContext(xmlDocP);
    if(xpathCtxP == NULL) {
        cerr << "Error: unable to create new XPath context" << endl;
        xmlFreeDoc(xmlDocP); 
        assert(false);
    }
    
    // ToDo: register namespaces
    
    // evaluate xpath expression
    xmlXPathObjectPtr xpathObjP = xmlXPathEvalExpression(xPathExpressionP, xpathCtxP);
    if(xpathObjP == NULL) {
        cerr << "Error: unable to evaluate xpath expression " << xPathExpressionP << endl;
        xmlXPathFreeContext(xpathCtxP); 
        xmlFreeDoc(xmlDocP); 
        assert(false);
    }
    
    // check Predicate (true/false)
    bool xPathCondition = xmlXPathEvalPredicate(xpathCtxP,xpathObjP);
    cout << "expression \"" << xPathExpressionP << "\" is " << (xPathCondition ? "true":"false") << endl;

    // cleanup
    xmlXPathFreeObject(xpathObjP);
    xmlXPathFreeContext(xpathCtxP); 
    xmlFreeDoc(xmlDocP); 
    
    return xPathCondition;
}

void printXmlNodes(xmlNodeSetPtr nodes) {
	
	if(xmlXPathNodeSetIsEmpty(nodes)){
		cout << "No result" << endl;
		return;
	}
	
    xmlNodePtr cur;
    int size;
    int i;
    
    size = (nodes) ? nodes->nodeNr : 0;
    
    cout << "Result (" << size << "):" << endl;
    for(i = 0; i < size; ++i) {
	assert(nodes->nodeTab[i]);
	
	if(nodes->nodeTab[i]->type == XML_NAMESPACE_DECL) {
	    xmlNsPtr ns;
	    
	    ns = (xmlNsPtr)nodes->nodeTab[i];
	    cur = (xmlNodePtr)ns->next;
	    if(cur->ns) { 
	        cout << "= namespace \"" << ns->prefix << "\"=\"" << ns->href << "\" for node " << cur->ns->href << ":" << cur->name << endl;
	    } else {
	        cout << "= namespace \"" << ns->prefix << "\"=\"" << ns->href << "\" for node " << cur->name << endl;
	    }
	} else if(nodes->nodeTab[i]->type == XML_ELEMENT_NODE) {
	    cur = nodes->nodeTab[i];   	    
	    if(cur->ns) { 
    	        cout << "<" << cur->ns->href << ":" << cur->name << "/>" << endl; 
	    } else {
    	        cout << "<" << cur->name << "/>" << endl; 
	    }
	} else {
	    cur = nodes->nodeTab[i];    
	    cout << "= node \"" << cur->name << "\": type " << cur->type << endl;
	}
    }
}

#else
void testXPathEvaluation() {
    cerr << "XPath support not compiled in" << endl;
    assert(false);
}
#endif
