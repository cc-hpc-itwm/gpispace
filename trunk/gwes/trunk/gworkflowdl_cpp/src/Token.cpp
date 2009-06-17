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
// gwdl
#include "Defines.h"
#include "XMLUtils.h"
#include "Token.h"

XERCES_CPP_NAMESPACE_USE
using namespace std;

#define X(str) XMLString::transcode((const char*)& str)
#define XS(strg) XMLString::transcode((const char*) strg.c_str())
#define S(str) XMLString::transcode(str)

namespace gwdl
{
	
Token::Token(Data* _data) 
{
	id = generateID(); 
	data = _data; 
	p_lock = NULL;
}

Token::Token(DOMElement* element) 
{
  id = generateID();
  control = false;
  p_lock = NULL;
  
  // XMLCh* ns = X(SCHEMA_wfSpace);

  // loop child nodes
   DOMNodeList* le = element->getChildNodes();
   if (le->getLength() >0) {
 	  // properties
 	  properties = Properties(le);
 	  // other nodes
 	  for (XMLSize_t i = 0; i<le->getLength(); i++) {
 		  DOMNode* node = le->item(i);
 		  const XMLCh* name = node->getNodeName(); 
 		  if (XMLString::equals(name,X("control"))) {
 			  if(!XMLString::compareIString(node->getTextContent(), X("true"))) control = true;
 		  } else if (XMLString::equals(name,X("data"))) {
 			  data = new Data((DOMElement*)node);
 		  }
 	  }
   }
}

DOMElement* Token::toElement(DOMDocument * doc)
{
	DOMElement* el = NULL;
    // Initialize the XML4C2 system.
    XMLUtils::Instance();
  
    XMLCh* ns = X(SCHEMA_wfSpace);
    
    {
           try
           {
               // token
               el = doc->createElementNS(ns, X("token"));
               // properties
               vector<DOMElement*> v = properties.toElements(doc);       
               for (unsigned int i = 0; i < v.size(); i++)
               { 
        	     //el->appendChild(doc->importNode(v[i],true));
        	     el->appendChild(v[i]);
               }
               // control
               if(!isData())
               {
                 DOMElement* el1 = doc->createElementNS(ns, X("control"));
                 if (control) el1->setTextContent(X("true"));
                 else  el1->setTextContent(X("false"));
                 el->appendChild(el1);
               }
               // data
               else 
               {
               	if (data!=NULL) {
               		DOMElement* eld = data->toElement(doc);
              		if (eld != NULL) {
          				el->appendChild(eld);
              		} else {
                   		cerr << "Data->toElement()==NULL exception" << endl;
              		}
               	} else {
               		cerr << "Data==NULL exception" << endl;
               	}
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
   }

   return el;
}

}

ostream& operator<<(ostream &out, gwdl::Token &token) 
{	
	DOMNode* node = token.toElement(gwdl::XMLUtils::Instance()->createEmptyDocument(true));
	gwdl::XMLUtils::Instance()->serialize(out,node,true);
	return out;
}

