/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
// gwes
#include <gwes/XPathEvaluator.h>
#include <gwes/CommandLineActivity.h>
// libxml2
#include <libxml/tree.h>
#include <libxml/parser.h>
#include <libxml/xpathInternals.h>
// std
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <errno.h>

using namespace std;
using namespace gwdl;

namespace gwes
{

/**
 * Reset static context cache
 */
xmlXPathContextPtr XPathEvaluator::_cacheXmlContextP = NULL;
xmlDocPtr XPathEvaluator::_cacheXmlContextDocP = NULL;
const TransitionOccurrence* XPathEvaluator::_cacheTransitionOccurrenceP = NULL;
int XPathEvaluator::_cacheStep = -1;

/**
 * example refer to http://xmlsoft.org/examples/xpath1.c
 */
XPathEvaluator::XPathEvaluator(const char* xmlContextChar) throw(XPathException) : _logger(fhg::log::getLogger("gwes")) 
{
	LOG_DEBUG(_logger, "XPathEvaluator(" << xmlContextChar << ")...");
	// init parser should be done only ONCE before construction of XPathEvaluator!
	XMLUtils::Instance();

    // create context
        xmlChar*xmlContextCharDuped(xmlCharStrdup(xmlContextChar));
    _xmlContextDocP = xmlParseDoc(xmlContextCharDuped);
    xmlFree(xmlContextCharDuped);
    _xmlContextP = xmlXPathNewContext(_xmlContextDocP);
    if(_xmlContextP == NULL) {
        LOG_ERROR(_logger, "Error: unable to create new XPath context");
        xmlFreeDoc(_xmlContextDocP); 
        throw XPathException("Error: unable to create new XPath context");
    }

    // register namespaces
	xmlXPathRegisterNs(_xmlContextP, (const xmlChar*)"gwdl", (const xmlChar*)"http://www.gridworkflow.org/gworkflowdl");
}

///ToDo FIXME: This class seems not to be thread safe for multiple workflows (segfault).
/// Check cache (one cache per workflow?)
/// Cache deactivated!!!
XPathEvaluator::XPathEvaluator(const TransitionOccurrence* toP, int step) throw (gwdl::WorkflowFormatException,XPathException) : _logger(fhg::log::getLogger("gwes")) {
	LOG_DEBUG(_logger, "XPathEvaluator(TransitionOccurrence=" << toP->getID() << ")...");
	LOG_DEBUG(_logger, "step=" << step);
	
//    // look if context is still available in cache
//	if (step == _cacheStep && toP == _cacheTransitionOccurrenceP) {
//		_xmlContextDocP = _cacheXmlContextDocP;
//		_xmlContextP = _cacheXmlContextP;
//		LOG_DEBUG(_logger, "XPathEvaluator(TransitionOccurrence=" << toP->getID() << "): using context from cache.");
//	}
//
//	// create new context from input and read places
//	else {

		// create new context document  
		_xmlContextDocP = xmlNewDoc((const xmlChar*)"1.0");
		xmlNodePtr rootP = xmlNewDocNode(_xmlContextDocP, NULL, (const xmlChar*)"data", NULL);
		xmlDocSetRootElement(_xmlContextDocP, rootP);

		// insert contents of parameter tokens.
		for (parameter_list_t::const_iterator it=toP->tokens.begin(); it!=toP->tokens.end(); ++it) {
			if (it->tokenP != NULL) {
				addTokenToContext(it->edgeP->getExpression(), it->tokenP);
			}
		}

		_xmlContextP = xmlXPathNewContext(_xmlContextDocP);

//		xmlXPathFreeContext(_cacheXmlContextP); 
//	    xmlFreeDoc(_cacheXmlContextDocP);
//	    _cacheXmlContextDocP = _xmlContextDocP;
//	    _cacheXmlContextP = _xmlContextP;
//	    _cacheStep = step;
//	    _cacheTransitionOccurrenceP = toP;
//	}
	
    if(_xmlContextP == NULL) {
        LOG_FATAL(_logger, "Error: unable to create new XPath context");
        xmlFreeDoc(_xmlContextDocP); 
        throw XPathException("Error: unable to create new XPath context");
    }
    
    LOG_DEBUG(_logger, "gwes::XPathEvaluator::XPathEvaluator(TransitionOccurrence=" << toP->getID() << "): context:"
    		<< endl << XMLUtils::Instance()->serializeLibxml2(_xmlContextDocP,false));

    // register namespaces
	xmlXPathRegisterNs(_xmlContextP, (const xmlChar*)"gwdl", (const xmlChar*)"http://www.gridworkflow.org/gworkflowdl");
}

XPathEvaluator::~XPathEvaluator()
{
    // do not free _xmlContextP if it has be moved to the cache.
	if (_cacheXmlContextP != _xmlContextP) {
		LOG_DEBUG(_logger, "Removing context from memory (not in cache)...");
		xmlXPathFreeContext(_xmlContextP); 
	    xmlFreeDoc(_xmlContextDocP);
	}
	// cleanup parser should be done only ONCE after destruction of XPathEvaluator!
    // xmlCleanupParser();
}

int XPathEvaluator::evalCondition(string& xPathExprStr) {
	// create xpath expression
  xmlChar* xPathExpressionP = xmlCharStrdup(expandVariables(xPathExprStr).c_str());
    
    // evaluate xpath expression
    xmlXPathObjectPtr xpathObjP = xmlXPathEvalExpression(xPathExpressionP, _xmlContextP);
    xmlFree(xPathExpressionP);
    if(xpathObjP == NULL) {
        LOG_ERROR(_logger, "ERROR: unable to evaluate xpath expression \"" << xPathExpressionP << "\"!");
        return -1;
    }
    
    // check Predicate (true/false)
    bool xPathCondition = xmlXPathEvalPredicate(_xmlContextP,xpathObjP);

    // cleanup
    xmlXPathFreeObject(xpathObjP);
    
	LOG_DEBUG(_logger, "evalCondition(" << xPathExprStr << ") = " << (xPathCondition ? "true":"false"));
    return xPathCondition;
}

string XPathEvaluator::evalExpression(string& xPathExprStr) {
	// create xpath expression
  xmlChar* xPathExpressionP = xmlCharStrdup(expandVariables(xPathExprStr).c_str());
	    
    // evaluate xpath expression
    xmlXPathObjectPtr xpathObjP = xmlXPathEvalExpression(xPathExpressionP, _xmlContextP);
    xmlFree(xPathExpressionP);
    if(xpathObjP == NULL) {
        LOG_ERROR(_logger, "ERROR: unable to evaluate xpath expression \"" << xPathExpressionP << "\"!");
        return NULL;
    }
    
    string xmlResult = string((const char*) xmlXPathCastToString(xpathObjP));

    // cleanup
    xmlXPathFreeObject(xpathObjP);
    
    //printXmlNodes(xpathObjP->nodesetval);
	LOG_INFO(_logger, "evalExpression(" << xPathExprStr << ") = " << xmlResult);
    
    return xmlResult;
}

string XPathEvaluator::evalExpression2Xml(string& xPathExprStr) {
	// create xpath expression
  xmlChar* xPathExpressionP = xmlCharStrdup(expandVariables(xPathExprStr).c_str());
    
    // evaluate xpath expression
    xmlXPathObjectPtr xpathObjP = xmlXPathEvalExpression(xPathExpressionP, _xmlContextP);
    xmlFree(xPathExpressionP);
    if(xpathObjP == NULL) {
        LOG_ERROR(_logger, "ERROR: unable to evaluate xpath expression \"" << xPathExpressionP << "\"!");
        return NULL;
    }
    
    string xmlResult;
	ostringstream xml;
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
		LOG_ERROR(_logger, "XPath evaluation result (xmlXPathObjectPtr) of type " << xpathObjP->type << " is not supported!");
		break;
	}

    // cleanup
    xmlXPathFreeObject(xpathObjP);
  
    //printXmlNodes(xpathObjP->nodesetval);
	LOG_DEBUG(_logger, "evalExpression2Xml(" << xPathExprStr << ") = " << xmlResult);
    
    return xmlResult;
}

void XPathEvaluator::printXmlNodes(xmlNodeSetPtr nodes) {
	
	if(xmlXPathNodeSetIsEmpty(nodes)){
		LOG_INFO(_logger, "No result");
		return;
	}
	
    xmlNodePtr cur;
    int size;
    int i;
    
    size = (nodes) ? nodes->nodeNr : 0;
    
    LOG_INFO(_logger, "Result (" << size << "):");
    for(i = 0; i < size; ++i) {
	
	if(nodes->nodeTab[i]->type == XML_NAMESPACE_DECL) {
	    xmlNsPtr ns;
	    
	    ns = (xmlNsPtr)nodes->nodeTab[i];
	    cur = (xmlNodePtr)ns->next;
	    if(cur->ns) { 
	        LOG_INFO(_logger, "= namespace \"" << ns->prefix << "\"=\"" << ns->href << "\" for node " << cur->ns->href << ":" << cur->name);
	    } else {
	        LOG_INFO(_logger, "= namespace \"" << ns->prefix << "\"=\"" << ns->href << "\" for node " << cur->name);
	    }
	} else if(nodes->nodeTab[i]->type == XML_ELEMENT_NODE) {
	    cur = nodes->nodeTab[i];   	    
	    if(cur->ns) { 
    	        LOG_INFO(_logger, "<" << cur->ns->href << ":" << cur->name << "/>"); 
	    } else {
    	        LOG_INFO(_logger, "<" << cur->name << "/>"); 
	    }
	} else {
	    cur = nodes->nodeTab[i];    
	    LOG_INFO(_logger, "= node \"" << cur->name << "\": type " << cur->type);
	}
    }
}

void XPathEvaluator::addTokenToContext(const string& edgeExpression, Token* tokenP) throw (gwdl::WorkflowFormatException) {
	xmlNodePtr cur;
	
	// data token
	if (tokenP->isData()) {

		string* textP = tokenP->getData()->getText();
		
		cur = xmlNewTextChild(xmlDocGetRootElement(_xmlContextDocP),NULL,(const xmlChar*)edgeExpression.c_str(),NULL);
		xmlDocPtr xmldoc = NULL;
		
		// token contains reference to data in file
		if ( textP->substr(0,7).compare("file://") == 0 ) {
			// if token text begins with "file://" insert content of file
			string fn = CommandLineActivity::convertUrlToLocalPath(*textP);
			LOG_DEBUG(_logger, "Adding file '" << fn << "' to context ...");
			xmldoc = XMLUtils::Instance()->deserializeFileLibxml2(fn);
			if (xmldoc == NULL) {      // file not in XML format
				LOG_INFO(_logger, "file '" << fn << "' skipped because it is not in XML format.");
			} 
		}

		// token contains the data itself
		if (xmldoc == NULL) {
			// ToDo: remove ugly conversion from Xerces DOMElement to libxml2 xmlDocPtr.
                  std::string* str = tokenP->getData()->toString();
			xmldoc = XMLUtils::Instance()->deserializeLibxml2(*str);
                        delete str;
		}
		
		if (xmldoc != NULL) {
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
		}
	} 
	
	// control token
	else {
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
