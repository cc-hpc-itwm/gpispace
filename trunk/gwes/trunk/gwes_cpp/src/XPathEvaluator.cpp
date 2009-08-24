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

/**
 * Reset static context cache
 */
xmlXPathContextPtr XPathEvaluator::_cacheXmlContextP = NULL;
xmlDocPtr XPathEvaluator::_cacheXmlContextDocP = NULL;
Transition* XPathEvaluator::_cacheTransitionP = NULL;
int XPathEvaluator::_cacheStep = -1;

/**
 * example refer to http://xmlsoft.org/examples/xpath1.c
 */
XPathEvaluator::XPathEvaluator(const char* xmlContextChar)
{
	cout << "gwes::XPathEvaluator::XPathEvaluator(" << xmlContextChar << ")..." << endl;
	// init parser should be done only ONCE before construction of XPathEvaluator!
	XMLUtils::Instance();

    // create context
    _xmlContextDocP = xmlParseDoc(xmlCharStrdup(xmlContextChar));
    _xmlContextP = xmlXPathNewContext(_xmlContextDocP);
    if(_xmlContextP == NULL) {
        cerr << "Error: unable to create new XPath context" << endl;
        xmlFreeDoc(_xmlContextDocP); 
        assert(false);
    }

    // register namespaces
	xmlXPathRegisterNs(_xmlContextP, (const xmlChar*)"gwdl", (const xmlChar*)"http://www.gridworkflow.org/gworkflowdl");
}

XPathEvaluator::XPathEvaluator(Transition* transitionP, int step) {
	cout << "gwes::XPathEvaluator::XPathEvaluator(Transition=" << transitionP->getID() << ")..." << endl;
	
//	// look if context is still available in cache
//	cout << "step=" << step << " cacheStep=" << _cacheStep << endl;
//	cout << "transition=" << transitionP << " cacheTransition=" << _cacheTransitionP << endl;
	
	if (step == _cacheStep && transitionP == _cacheTransitionP) {
		_xmlContextDocP = _cacheXmlContextDocP;
		_xmlContextP = _cacheXmlContextP;
		cout << "gwes::XPathEvaluator::XPathEvaluator(Transition=" << transitionP->getID() << "): using context from cache." << endl;
	}

	// create new context from input and read places
	else {

		// create new context document  
		_xmlContextDocP = xmlNewDoc((const xmlChar*)"1.0");
		xmlNodePtr rootP = xmlNewDocNode(_xmlContextDocP, NULL, (const xmlChar*)"token", NULL);
		xmlDocSetRootElement(_xmlContextDocP, rootP);

		// insert contents of next unlocked tokens that are connected with edgeExpressions.

		// readPlaces
		vector<Edge*> readEdges = transitionP->getReadEdges();
		for (unsigned int i=0; i<readEdges.size(); i++) {
			string edgeExpression = readEdges[i]->getExpression();
			if (!edgeExpression.empty()) {
//				cout << "gwes::XPathEvaluator::XPathEvaluator(Transition=" << transitionP->getID() << "): adding token to context with edgeExpression=" << edgeExpression << endl;
				unsigned int j = 0;
				Token* tokenP = readEdges[i]->getPlace()->getTokens()[j];
				while (tokenP->isLocked()) {
					tokenP = readEdges[i]->getPlace()->getTokens()[++j];
				}
				addTokenToContext(edgeExpression, tokenP); 
			}
		}

		// inputPlaces
		vector<Edge*> inEdges = transitionP->getInEdges();
		for (unsigned int i=0; i<inEdges.size(); i++) {
			string edgeExpression = inEdges[i]->getExpression();
			if (!edgeExpression.empty()) {
//				cout << "gwes::XPathEvaluator::XPathEvaluator(Transition=" << transitionP->getID() << "): adding token to context with edgeExpression=" << edgeExpression << endl;
				unsigned int j = 0;
				Token* tokenP = inEdges[i]->getPlace()->getTokens()[j];
				while (tokenP->isLocked()) {
					tokenP = inEdges[i]->getPlace()->getTokens()[++j];
				}
				addTokenToContext(edgeExpression, tokenP); 
			}
		}

		_xmlContextP = xmlXPathNewContext(_xmlContextDocP);

//	    cout << "gwes::XPathEvaluator::XPathEvaluator(Transition=" << transitionP->getID() << "): moving context to cache." << endl;
		xmlXPathFreeContext(_cacheXmlContextP); 
	    xmlFreeDoc(_cacheXmlContextDocP);
	    _cacheXmlContextDocP = _xmlContextDocP;
	    _cacheXmlContextP = _xmlContextP;
	    _cacheStep = step;
	    _cacheTransitionP = transitionP;
	}
	
    if(_xmlContextP == NULL) {
        cerr << "Error: unable to create new XPath context" << endl;
        xmlFreeDoc(_xmlContextDocP); 
        assert(false);
    }
    
    cout << "gwes::XPathEvaluator::XPathEvaluator(Transition=" << transitionP->getID() << "): context:" << endl;
	cout << *XMLUtils::Instance()->serializeLibxml2(_xmlContextDocP,false) << endl;

    // register namespaces
	xmlXPathRegisterNs(_xmlContextP, (const xmlChar*)"gwdl", (const xmlChar*)"http://www.gridworkflow.org/gworkflowdl");
}

XPathEvaluator::~XPathEvaluator()
{
	cout << "gwes::XPathEvaluator::~XPathEvaluator()..." << endl;
    // do not free _xmlContextP if it has be moved to the cache.
	if (_cacheXmlContextP != _xmlContextP) {
		cout << "gwes::XPathEvaluator::~XPathEvaluator(): Removing context from memory (not in cache)..." << endl;
		xmlXPathFreeContext(_xmlContextP); 
	    xmlFreeDoc(_xmlContextDocP);
	}
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

void XPathEvaluator::addTokenToContext(const string& edgeExpression, Token* tokenP) {
	xmlNodePtr cur;
	
	if (tokenP->isData()) {
		// data token
		cur = xmlNewTextChild(xmlDocGetRootElement(_xmlContextDocP),NULL,(const xmlChar*)edgeExpression.c_str(),NULL);
		// ToDo: remove ugly conversion from Xerces DOMElement to libxml2 xmlDocPtr.
		xmlDocPtr xmldoc = XMLUtils::Instance()->deserializeLibxml2(*tokenP->getData()->toString());
		// copy children from one document to the other.
		xmlNodePtr children = xmlDocCopyNodeList(_xmlContextDocP, xmlDocGetRootElement(xmldoc)->children);
	    xmlAddChildList(cur,children);
	    xmlFreeDoc(xmldoc);
	} else {
		// control token
		if (tokenP->getControl()) {
			// <control>true</control>
			cur = xmlNewTextChild(xmlDocGetRootElement(_xmlContextDocP),NULL,(const xmlChar*)edgeExpression.c_str(),(const xmlChar*)"true");
		} else {
			// <control>false</control>
			cur = xmlNewTextChild(xmlDocGetRootElement(_xmlContextDocP),NULL,(const xmlChar*)edgeExpression.c_str(),(const xmlChar*)"false");
		}
	}
}

/** 
 * Replace "$" by "/token/" in string
 */
string XPathEvaluator::expandVariables(string& str) {
	size_t i = str.find('$');
	if (i == str.npos) return str;
	str.replace(i,1,"/token/");
	return expandVariables(str);
}

} // end namespace gwes
