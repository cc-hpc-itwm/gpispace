/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
// gwes
#include <gwes/XPathEvaluator.h>
// libxml2
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpathInternals.h>
// std
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

using namespace std;
using namespace gwdl;

namespace gwes
{

XPathEvaluator::XPathEvaluator(const char* xmlContextChar)
{
	cout << "gwes::XPathEvaluator::XPathEvaluator(" << xmlContextChar << ")..." << endl;
	// init parser should be done only ONCE before construction of XPathEvaluator!
    // xmlInitParser();
    // LIBXML_TEST_VERSION

    // create context
    _xmlContextDocP = xmlParseDoc(xmlCharStrdup(xmlContextChar));
    _xmlContextP = xmlXPathNewContext(_xmlContextDocP);
    if(_xmlContextP == NULL) {
        cerr << "Error: unable to create new XPath context" << endl;
        xmlFreeDoc(_xmlContextDocP); 
        assert(false);
    }
    // ToDo: register namespaces
}

XPathEvaluator::XPathEvaluator(Transition* transitionP) {
	cout << "gwes::XPathEvaluator::XPathEvaluator(Transition=" << transitionP->getID() << ")..." << endl;
	
	// create new context document  
	_xmlContextDocP = xmlNewDoc((const xmlChar*)"1.0");
	xmlNodePtr rootP = xmlNewDocNode(_xmlContextDocP, NULL, (const xmlChar*)"token", NULL);
	xmlDocSetRootElement(_xmlContextDocP, rootP);

	xmlNodePtr cur;
	
	// insert contents of unlocked tokens that are connected with edgeExpressions.
	vector<Edge*> inEdges = transitionP->getInEdges();
	for (unsigned int i=0; i<inEdges.size(); i++) {
		string edgeExpression = inEdges[i]->getExpression();
		if (!edgeExpression.empty()) {
			cout << "gwes::XPathEvaluator::XPathEvaluator(Transition=" << transitionP->getID() << "): adding token to context with edgeExpression=" << edgeExpression << endl;
			unsigned int j = 0;
			Token* tokenP = inEdges[i]->getPlace()->getTokens()[j];
			while (tokenP->isLocked()) {
				tokenP = inEdges[i]->getPlace()->getTokens()[++j];
			}
			cur = xmlNewChild(rootP,NULL,(const xmlChar*)edgeExpression.c_str(),NULL);
			// ToDo: implement!
		}
	}

	_xmlContextP = xmlXPathNewContext(_xmlContextDocP);
    if(_xmlContextP == NULL) {
        cerr << "Error: unable to create new XPath context" << endl;
        xmlFreeDoc(_xmlContextDocP); 
        assert(false);
    }

    // ToDo: register namespaces
}

XPathEvaluator::~XPathEvaluator()
{
	cout << "gwes::XPathEvaluator::~XPathEvaluator()..." << endl;
    xmlXPathFreeContext(_xmlContextP); 
    xmlFreeDoc(_xmlContextDocP);
	// cleanup parser should be done only ONCE after destruction of XPathEvaluator!
    // xmlCleanupParser();
}

int XPathEvaluator::evalCondition(const char* xPathExprChar) {
	cout << "gwes::XPathEvaluator::evalCondition(" << xPathExprChar << ")..." << endl;

	// create xpath expression
	const xmlChar* xPathExpressionP = xmlCharStrdup(xPathExprChar);
    
    // evaluate xpath expression
    xmlXPathObjectPtr xpathObjP = xmlXPathEvalExpression(xPathExpressionP, _xmlContextP);
    if(xpathObjP == NULL) {
        cerr << "gwes::XPathEvaluator::evalCondition(): ERROR: unable to evaluate xpath expression \"" << xPathExpressionP << "\"!" << endl;
        return -1;
    }
    
    // check Predicate (true/false)
    bool xPathCondition = xmlXPathEvalPredicate(_xmlContextP,xpathObjP);

    // cleanup
    xmlXPathFreeObject(xpathObjP);
    
	cout << "gwes::XPathEvaluator::evalCondition(" << xPathExprChar << ") = " << (xPathCondition ? "true":"false") << endl;
    return xPathCondition;
}

const char* XPathEvaluator::evalExpression(const char* xPathExprChar) {
	cout << "gwes::XPathEvaluator::evalExpression(" << xPathExprChar << ")..." << endl;
	// create xpath expression
	const xmlChar* xPathExpressionP = xmlCharStrdup(xPathExprChar);
    
    // evaluate xpath expression
    xmlXPathObjectPtr xpathObjP = xmlXPathEvalExpression(xPathExpressionP, _xmlContextP);
    if(xpathObjP == NULL) {
        cerr << "gwes::XPathEvaluator::evalCondition(): ERROR: unable to evaluate xpath expression \"" << xPathExpressionP << "\"!" << endl;
        return NULL;
    }
    
    const xmlChar* xmlResult = xmlXPathCastToString(xpathObjP);

    // cleanup
    xmlXPathFreeObject(xpathObjP);
    
    //printXmlNodes(xpathObjP->nodesetval);
	cout << "gwes::XPathEvaluator::evalExpression(" << xPathExprChar << ") = " << xmlResult << endl;
    
    return (char*) xmlResult;
}

void XPathEvaluator::printXmlNodes(xmlNodeSetPtr nodes) {
	
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

} // end namespace gwes
