/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#include <iostream>
#include <xercesc/util/OutOfMemoryException.hpp>

#include <gwdl/Defines.h>
#include <gwdl/OperationClass.h>
#include <gwdl/Operation.h>
#include <gwdl/XMLUtils.h>
#include <gwdl/OperationCandidate.h>

#define X(str) XMLString::transcode((const char*)& str)
#define XS(strg) XMLString::transcode((const char*) strg.c_str())
#define S(str) XMLString::transcode(str)


namespace gwdl
{
    
OperationClass::OperationClass(DOMElement* element)
{
  //cout << "OperationClass(el)=" << this << endl;
  name = string(S(element->getAttribute(X("name"))));
  DOMNodeList* le;
  le = element->getChildNodes();
  for(unsigned int i=0; i < le->getLength(); ++i)
  {
	DOMElement* el = (DOMElement*) le->item(i);
	const XMLCh* name = el->getTagName(); 
	
	if(XMLString::equals(name,X("oc:operationCandidate")))
	{
      operationCandidates.push_back(new OperationCandidate(el));	
	}
  }
}
	
OperationClass::~OperationClass()
{
  //cout << "~OperationClass()=" << this << endl;
  removeAllOperationCandidates();
}
	
DOMElement* OperationClass::toElement(DOMDocument* doc)
{
	DOMElement* el = NULL;
    // Initialize the XML4C2 system.
    XMLUtils::Instance();
  
    XMLCh* ns = X(SCHEMA_wf_oc);
    
	try
    {
    	el = doc->createElementNS(ns, X("oc:operationClass")); 
        if (name.size()>0) el->setAttribute(X("name"), XS(name));
        for (unsigned int i = 0; i < operationCandidates.size(); i++)
        {
        	el->appendChild(operationCandidates[i]->toElement(doc));
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

vector<OperationCandidate*>& OperationClass::getOperationCandidates()
{
	return operationCandidates;
}

int OperationClass::getAbstractionLevel()
{
  if(operationCandidates.size() == 0)
  {
  	return Operation::YELLOW;
  }
  else
  {
    int count = 0;
    for (unsigned int i = 0; i < operationCandidates.size(); i++)
    {
      if(operationCandidates[i]->getAbstractionLevel()==Operation::GREEN)
      {
        count++;
      }
    }
    return count == 1 ? Operation::GREEN : Operation::BLUE;
  }
}

void OperationClass::removeOperationCandidate(int i)
{
	delete operationCandidates[i]; operationCandidates.erase(operationCandidates.begin()+i);
}

void OperationClass::removeOperationCandidate(OperationCandidate& oper)
{
	for (unsigned int i=0; i<operationCandidates.size(); i++) {
		if (oper.getID() == operationCandidates[i]->getID()) {
			OperationCandidate* ocP = * (operationCandidates.begin()+i);
			delete ocP;
			ocP=NULL;
			operationCandidates.erase(operationCandidates.begin()+i);
			break;	
		}	
	}
}

void OperationClass::removeAllOperationCandidates()
{
	for (unsigned int i=0; i<operationCandidates.size(); i++) {
		delete operationCandidates[i];
	}
	operationCandidates.clear();
}

}
