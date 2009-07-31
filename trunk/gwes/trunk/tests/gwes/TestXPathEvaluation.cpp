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

    // create xpath expression
    cout << "create xpath expression..." << endl;
    const xmlChar* xPathExpressionP = xmlCharStrdup("//data/min");
    cout << "xpath expression = " << xPathExpressionP << endl;
    
    // create xpath context
    cout << "create xpath context..." << endl;
    const xmlChar* xmlCharP = xmlCharStrdup("<data><min>15</min><max>33</max><step>1</step></data>");
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

    // result output
    printXmlNodes(xpathObjP->nodesetval);

    // cleanup
    cout << "cleanup objects..." << endl;
    xmlXPathFreeObject(xpathObjP);
    xmlXPathFreeContext(xpathCtxP); 
    xmlFreeDoc(xmlDocP); 
    xmlCleanupParser();
	cout << "============== END XpathEvaluation TEST =============" << endl;
}

void printXmlNodes(xmlNodeSetPtr nodes) {
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
    	        cout << "= element node \"" << cur->ns->href << ":" << cur->name << "\"" << endl; 
	    } else {
    	        cout << "= element node \"" << cur->name << "\"" << endl; 
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
