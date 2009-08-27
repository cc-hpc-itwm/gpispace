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

XPathEvaluator::XPathEvaluator(Transition* transitionP, int step) throw (gwdl::WorkflowFormatException) {
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
		xmlNodePtr rootP = xmlNewDocNode(_xmlContextDocP, NULL, (const xmlChar*)"data", NULL);
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
	cout << XMLUtils::Instance()->serializeLibxml2(_xmlContextDocP,false) << endl;

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

int XPathEvaluator::evalCondition(string& xPathExprStr) {
	cout << "gwes::XPathEvaluator::evalCondition(" << xPathExprStr << ")..." << endl;
	
	// create xpath expression
    const xmlChar* xPathExpressionP = xmlCharStrdup(expandVariables(xPathExprStr).c_str());
    
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
    
	cout << "gwes::XPathEvaluator::evalCondition(" << xPathExprStr << ") = " << (xPathCondition ? "true":"false") << endl;
    return xPathCondition;
}

string XPathEvaluator::evalExpression(string& xPathExprStr) {
	cout << "gwes::XPathEvaluator::evalExpression(" << xPathExprStr << ")..." << endl;
	
	// create xpath expression
    const xmlChar* xPathExpressionP = xmlCharStrdup(expandVariables(xPathExprStr).c_str());
	    
    // evaluate xpath expression
    xmlXPathObjectPtr xpathObjP = xmlXPathEvalExpression(xPathExpressionP, _xmlContextP);
    if(xpathObjP == NULL) {
        cerr << "gwes::XPathEvaluator::evalExpression(): ERROR: unable to evaluate xpath expression \"" << xPathExpressionP << "\"!" << endl;
        return NULL;
    }
    
    string xmlResult = string((const char*) xmlXPathCastToString(xpathObjP));

    // cleanup
    xmlXPathFreeObject(xpathObjP);
    
    //printXmlNodes(xpathObjP->nodesetval);
	cout << "gwes::XPathEvaluator::evalExpression(" << xPathExprStr << ") = " << xmlResult << endl;
    
    return xmlResult;
}

string XPathEvaluator::evalExpression2Xml(string& xPathExprStr) {
	cout << "gwes::XPathEvaluator::evalExpression2Xml(" << xPathExprStr << ")..." << endl;

	// create xpath expression
    const xmlChar* xPathExpressionP = xmlCharStrdup(expandVariables(xPathExprStr).c_str());
    
    // evaluate xpath expression
    xmlXPathObjectPtr xpathObjP = xmlXPathEvalExpression(xPathExpressionP, _xmlContextP);
    if(xpathObjP == NULL) {
        cerr << "gwes::XPathEvaluator::evalExpression2Xml(): ERROR: unable to evaluate xpath expression \"" << xPathExpressionP << "\"!" << endl;
        return NULL;
    }
    
    string xmlResult;
	ostringstream xml;
	// cout << "gwes::XPathEvaluator::evalExpression2Xml(): evaluates to type " << xpathObjP->type << endl;
    //      	XPATH_UNDEFINED = 0
    // XPATH_NODESET = 1
    // XPATH_BOOLEAN = 2
    // XPATH_NUMBER = 3
    // XPATH_STRING = 4
    //    	    XPATH_POINT = 5
    //    	    XPATH_RANGE = 6
    //    	    XPATH_LOCATIONSET = 7
    //    	    XPATH_USERS = 8
    //    	    XPATH_XSLT_TREE = 9 
	
	switch (xpathObjP->type) {
	case(XPATH_UNDEFINED):
		break;
	case(XPATH_NODESET):
	    cout << "gwes::XPathEvaluator::evalExpression2Xml(): nodes->nodeNr=" << xpathObjP->nodesetval->nodeNr << endl;
		xml << "<data>\n";
		for (int i = 0; i < xpathObjP->nodesetval->nodeNr; i++) {
			xml << "  " << XMLUtils::Instance()->serializeLibxml2(xpathObjP->nodesetval->nodeTab[i],false);
		}
		xml << "</data>";
		xmlResult = xml.str();
		break;
	case (XPATH_BOOLEAN): 
		xml << "<data><boolean xmlns=\"\">";
		if (xpathObjP->boolval) xml << "true";
		else xml << "false";
		xml << "</boolean></data>"; 
		xmlResult = xml.str(); 
		break;
	case(XPATH_NUMBER):
		xml << "<data><number xmlns=\"\">" << xpathObjP->floatval << "</number></data>"; 
		xmlResult = xml.str(); 
		break;
	case(XPATH_STRING):
		xml << "<data><string xmlns=\"\">" << xpathObjP->stringval << "</string></data>"; 
		xmlResult = xml.str(); 
		break;
	case(XPATH_POINT):
	case(XPATH_RANGE):
	case(XPATH_LOCATIONSET):
	case(XPATH_USERS):
	case(XPATH_XSLT_TREE):
		cerr << "XPath evaluation result (xmlXPathObjectPtr) of type " << xpathObjP->type << " is not supported!" << endl;
		break;
	}

    // cleanup
    xmlXPathFreeObject(xpathObjP);
    
    //printXmlNodes(xpathObjP->nodesetval);
	cout << "gwes::XPathEvaluator::evalExpression2Xml(" << xPathExprStr << ") = " << xmlResult << endl;
    
    return xmlResult;
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

void XPathEvaluator::addTokenToContext(const string& edgeExpression, Token* tokenP) throw (gwdl::WorkflowFormatException) {
	xmlNodePtr cur;
	
	if (tokenP->isData()) {
		// data token
		cur = xmlNewTextChild(xmlDocGetRootElement(_xmlContextDocP),NULL,(const xmlChar*)edgeExpression.c_str(),NULL);
		// ToDo: remove ugly conversion from Xerces DOMElement to libxml2 xmlDocPtr.
		xmlDocPtr xmldoc = XMLUtils::Instance()->deserializeLibxml2(*tokenP->getData()->toString());
		// copy children from one document to the other.
		// search for <data> element
		xmlNodePtr dataElementP = xmlDocGetRootElement(xmldoc)->children; 
		while (dataElementP != NULL && dataElementP->type != XML_ELEMENT_NODE) dataElementP = dataElementP->next;
		if (dataElementP == NULL) {
			// missing data element
			ostringstream message; 
			message << "Missing <data> element on data token " << tokenP->getID() << "!";
			throw gwdl::WorkflowFormatException(message.str());
		}
		xmlNodePtr children = xmlDocCopyNodeList(_xmlContextDocP, dataElementP->children);
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
 * Replace "$" by "/data/" in string
 */
string XPathEvaluator::expandVariables(string& str) {
	size_t i = str.find('$');
	if (i == str.npos) return str;
	str.replace(i,1,"/data/");
	return expandVariables(str);
}

} // end namespace gwes
