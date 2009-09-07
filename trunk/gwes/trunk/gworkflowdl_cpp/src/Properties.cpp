/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */

// std
#include <iostream>
// xerces-c
#include <xercesc/util/OutOfMemoryException.hpp>
//fhglog
#include <fhglog/fhglog.hpp>
// gwdl
#include <gwdl/Properties.h>
#include <gwdl/Defines.h>
#include <gwdl/XMLUtils.h>

using namespace fhg::log;
XERCES_CPP_NAMESPACE_USE
using namespace std;

#define X(str) XMLString::transcode((const char*)& str)
#define XS(strg) XMLString::transcode((const char*) strg.c_str())
#define S(str) XMLString::transcode(str)

namespace gwdl
{

Properties::Properties(DOMNodeList* list)
{
  for(unsigned int i=0; i < list->getLength(); ++i)
  {
	  DOMNode* node = (DOMNode*) list->item(i);
	  if (node->getNodeType() == DOMNode::ELEMENT_NODE) {
		  if (XMLString::equals(node->getNodeName(),X("property"))) {
			  DOMElement* el = (DOMElement*) node;
		      insert(pair<string,string>(string(S(el->getAttribute(X("name")))), string(S(el->getTextContent()))));
		  }
	  }
  }
}

vector<DOMElement*> Properties::toElements(DOMDocument* doc)
{
	std::vector<XERCES_CPP_NAMESPACE::DOMElement*> _dom;
	
    // Initialize the XML4C2 system.
    XMLUtils::Instance();
  
    XMLCh* ns = X(SCHEMA_wfSpace);
    
    {
       int errorCode = 0;
           try
           {
               DOMElement* el;      
               for(ITR_Properties it = begin(); it != end(); ++it)
               {
        	     el = doc->createElementNS(ns, X("property"));
                 el->setAttribute(X("name"), XS(it->first));
                 el->setTextContent(XS(it->second));
                 _dom.push_back(el);
               }                 
           }
           catch (const OutOfMemoryException&)
           {
               LOG_WARN(Logger::get("gwdl"), "OutOfMemoryException" );
               errorCode = 5;
           }
           catch (const DOMException& e)
           {
               LOG_WARN(Logger::get("gwdl"), "DOMException code is:  " << e.code );
               LOG_WARN(Logger::get("gwdl"), "Message: " << S(e.msg) );
               errorCode = 2;
           }
           catch (...)
           {
               LOG_WARN(Logger::get("gwdl"), "An error occurred creating the document" );
               errorCode = 3;
           }
   }

   return _dom;
}

}

ostream& operator<<(ostream &out, gwdl::Properties &props) 
{	
	vector<DOMElement*> elements = props.toElements(gwdl::XMLUtils::Instance()->createEmptyDocument(true));
	for (unsigned int i=0; i<elements.size(); i++) 
	{
	    gwdl::XMLUtils::Instance()->serialize(out,elements[i],true);
	}	
	return out;
}
