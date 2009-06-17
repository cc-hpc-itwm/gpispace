/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
// xerces-c
#include <xercesc/dom/DOMLocator.hpp>
// std
#include <iostream>
#include <ostream>
#include <sstream>
// gwdl
#include "XMLDOMErrorHandler.h"

using namespace std;
XERCES_CPP_NAMESPACE_USE

namespace gwdl
{

XMLDOMErrorHandler::XMLDOMErrorHandler()
{
	reset();
	error = -1;
}

XMLDOMErrorHandler::~XMLDOMErrorHandler()
{
}

bool XMLDOMErrorHandler::handleError(const DOMError &domError) 
{
	char * errormessage = XMLString::transcode(domError.getMessage());
	DOMLocator *location = domError.getLocation();
	error = domError.getSeverity();
	
	bool ret = true;
	char* type;
	switch(error)
	{
	case DOMError::DOM_SEVERITY_WARNING:
		ret = true;
	    type = "warning";
	break;
	case DOMError::DOM_SEVERITY_ERROR:
		ret = true;
		hasError = true;
		type = "parsing error";
	break;
	case DOMError::DOM_SEVERITY_FATAL_ERROR:
		ret = false;
		hasError = true;
		type = "fatal error";
	break;
	}
	ostringstream oss; 
	oss << "XML " << type << " at line/column (" << location->getLineNumber() << "/"<< location->getColumnNumber() << "): " << errormessage;
	message = oss.str();	
	//return false;
	return ret;
}

void XMLDOMErrorHandler::reset() { hasError = false; }
	
}


