/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
//std
#include <iostream>
#include <sstream>
//xerces-c
#include <xercesc/util/OutOfMemoryException.hpp>
// gwdl
#include <gwdl/OperationCandidate.h>
#include <gwdl/XMLUtils.h>
#include <gwdl/Defines.h>
#include <gwdl/Operation.h>

XERCES_CPP_NAMESPACE_USE
using namespace std;

#define X(str) XMLString::transcode((const char*)& str)
#define XS(strg) XMLString::transcode((const char*) strg.c_str())
#define S(str) XMLString::transcode(str)

namespace gwdl
{

OperationCandidate::OperationCandidate()
{
	id = generateID(); selected=false; type = ""; operationName = ""; resourceName = ""; quality = -1.0;
}

OperationCandidate::OperationCandidate(DOMElement* el) 
{
	const XMLCh* s = el->getAttribute(X("type"));
	if(XMLString::stringLen(s) > 0) type = string(S(s));
	s = el->getAttribute(X("operationName"));
	if(XMLString::stringLen(s) > 0) operationName = string(S(s));
	s = el->getAttribute(X("resourceName"));
	if(XMLString::stringLen(s) > 0) resourceName = string(S(s));
	s = el->getAttribute(X("selected"));
	if(XMLString::compareString(s,X("true")) == 0) selected = true;
	else selected = false;
	s = el->getAttribute(X("quality"));
	quality = XMLString::stringLen(s) != 0 ? atof(S(s)) : -1;
}

DOMElement* OperationCandidate::toElement(DOMDocument* doc)
{
	DOMElement* el = NULL;
    // Initialize the XML4C2 system.
    XMLUtils::Instance();
  
    XMLCh* ns = X(SCHEMA_wf_oc);
    
    try
    {   
    	el = doc->createElementNS(ns, X("oc:operationCandidate"));
    	if (type.size()>0) el->setAttribute(X("type"), XS(type));
    	if (operationName.size()>0) el->setAttribute(X("operationName"), XS(operationName));
    	if (resourceName.size()>0) el->setAttribute(X("resourceName"), XS(resourceName));
    	if (selected) el->setAttribute(X("selected"), S("true"));
    	if (quality>-1.0)
    	{
			ostringstream oss;
           	oss << quality; 
    		el->setAttribute(X("quality"), XS(string(oss.str())));
    	}
    }
    catch (const OutOfMemoryException&)
    {
    	XERCES_STD_QUALIFIER cerr << "OutOfMemoryException" << XERCES_STD_QUALIFIER endl;
	}
    catch (const DOMException& e)
    {
    	XERCES_STD_QUALIFIER cerr << "DOMException code is:  " << e.code << XERCES_STD_QUALIFIER endl;
        XERCES_STD_QUALIFIER cerr << "Message: " << S(e.msg) << XERCES_STD_QUALIFIER endl;
	}
    catch (...)
    {
    	XERCES_STD_QUALIFIER cerr << "An error occurred creating the document" << XERCES_STD_QUALIFIER endl;
	}
	
	return el;
}

int OperationCandidate::getAbstractionLevel() 
{
	if (selected) return Operation::GREEN;
	else return Operation::BLUE;		
}

} // end namespace gwdl
