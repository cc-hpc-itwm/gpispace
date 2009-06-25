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
//gwdl
#include <gwdl/Operation.h>
#include <gwdl/Defines.h>
#include <gwdl/XMLUtils.h>

XERCES_CPP_NAMESPACE_USE
using namespace std;

#define X(str) XMLString::transcode((const char*)& str)
#define XS(strg) XMLString::transcode((const char*) strg.c_str())
#define S(str) XMLString::transcode(str)

namespace gwdl
{

Operation::Operation()
{
	operationClass = NULL;
}

Operation::~Operation()
{
	if(operationClass != NULL)
	{
		delete operationClass;
		operationClass = NULL;
	}
}

Operation::Operation(DOMElement* element)
{
	//cout << "in Operation(el)=" << this << " operationClass=" << operationClass << endl;

	// default values
	operationClass = NULL;

	//XMLCh* ns = X(SCHEMA_wf_oc);

	// loop child nodes
	DOMNodeList* le = element->getChildNodes();
	for (XMLSize_t i = 0; i<le->getLength(); i++) {
		DOMNode* node = le->item(i);
		const XMLCh* name = node->getNodeName(); 
		// operationClass
		if (XMLString::equals(name,X("oc:operationClass"))) {
			operationClass = new OperationClass((DOMElement*)node);
		}  				 
	}
}

DOMElement* Operation::toElement(DOMDocument* doc)
{
	DOMElement* el = NULL;
	// Initialize the XML4C2 system.
	XMLUtils::Instance();

	XMLCh* ns = X(SCHEMA_wfSpace);

	try
	{
		el = doc->createElementNS(ns, X("operation"));
		if(operationClass != NULL)
		{    
			el->appendChild(operationClass->toElement(doc));
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

int Operation::getAbstractionLevel()
{	
	if(operationClass != NULL) return operationClass->getAbstractionLevel();
	else return RED;
}

} //end namespace gwdl

ostream& operator<<(ostream &out, gwdl::Operation &operation) 
{	
	DOMNode* node = operation.toElement(gwdl::XMLUtils::Instance()->createEmptyDocument(true));
	gwdl::XMLUtils::Instance()->serialize(out,node,true);
	return out;
}


