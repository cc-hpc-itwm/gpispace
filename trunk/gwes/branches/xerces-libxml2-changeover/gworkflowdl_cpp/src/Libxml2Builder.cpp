/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
//gwdl
#include <gwdl/Libxml2Builder.h>

using namespace std;

namespace gwdl
{

/**
 * Constructor implementation.
 */
Libxml2Builder::Libxml2Builder() {}

/**
 * Destructor implementation.
 */
Libxml2Builder::~Libxml2Builder() {}

// Data
const Data::ptr_t Libxml2Builder::deserializeData(const string &xmlstring) const {
	Data::ptr_t data(new Data(xmlstring));
	return data;
}

const string Libxml2Builder::serializeData(const Data::ptr_t &data) const {
	return data->serialize();
}

///dataToElement()
///elementToData()

// Token
const Token::ptr_t Libxml2Builder::deserializeToken(const string &xmlstring) const {
	xmlDocPtr docP = XMLUtils::Instance()->deserialize(xmlstring);
	xmlNodePtr nodeP = xmlDocGetRootElement(docP);
	const Token::ptr_t ret = elementToToken(nodeP);
	xmlFreeDoc(docP);
	return ret;
}

const Token::ptr_t Libxml2Builder::elementToToken(const xmlNodePtr nodeP) const {
//	long id;
//    Data* data;
//    Properties properties;
//    bool control;
//    Transition* p_lock;
	
 	
}

const string serializeToken(const Token::ptr_t &token) const {
	
}

//Token::Token(DOMElement* element) 
//{
//  id = generateID();
//  control = false;
//  p_lock = NULL;
//  data = NULL;
//  
//  // XMLCh* ns = X(SCHEMA_wfSpace);
//
//  // loop child nodes
//   DOMNodeList* le = element->getChildNodes();
//   if (le->getLength() >0) {
// 	  // properties
// 	  properties = Properties(le);
// 	  // other nodes
// 	  for (XMLSize_t i = 0; i<le->getLength(); i++) {
// 		  DOMNode* node = le->item(i);
// 		  const XMLCh* name = node->getNodeName(); 
// 		  if (XMLString::equals(name,X("control"))) {
// 			  if(!XMLString::compareIString(node->getTextContent(), X("true"))) control = true;
// 		  } else if (XMLString::equals(name,X("data"))) {
// 			  LOG_WARN(logger_t(getLogger("gwdl")), "ToDo: Refractoring to libxml2");
//// 			  data = new Data((DOMElement*)node);
// 		  }
// 	  }
//   }
//}
//
//DOMElement* Token::toElement(DOMDocument * doc)
//{
//	DOMElement* el = NULL;
//    // Initialize the XML4C2 system.
//    XMLUtils::Instance();
//  
//    XMLCh* ns = X(SCHEMA_wfSpace);
//    
//    {
//           try
//           {
//               // token
//               el = doc->createElementNS(ns, X("token"));
//               // properties
//               vector<DOMElement*> v = properties.toElements(doc);       
//               for (unsigned int i = 0; i < v.size(); i++)
//               { 
//        	     //el->appendChild(doc->importNode(v[i],true));
//        	     el->appendChild(v[i]);
//               }
//               // control
//               if(!isData())
//               {
//                 DOMElement* el1 = doc->createElementNS(ns, X("control"));
//                 if (control) el1->setTextContent(X("true"));
//                 else  el1->setTextContent(X("false"));
//                 el->appendChild(el1);
//               }
//               // data
//               else 
//               {
//               	if (data!=NULL) {
//               		LOG_WARN(logger_t(getLogger("gwdl")), "ToDo: Refractoring to libxml2");
////               		DOMElement* eld = data->toElement(doc);
////              		if (eld != NULL) {
////          				el->appendChild(eld);
////              		} else {
////                   		LOG_ERROR(logger_t(getLogger("gwdl")), "Data->toElement()==NULL exception");
////              		}
//               	} else {
//               		LOG_ERROR(logger_t(getLogger("gwdl")), "Data->toElement()==NULL exception");
//               	}
//               }                                  
//           }
//           catch (const OutOfMemoryException&)
//           {
//               LOG_FATAL(logger_t(getLogger("gwdl")), "OutOfMemoryException" );
//           }
//           catch (const DOMException& e)
//           {
//               LOG_ERROR(logger_t(getLogger("gwdl")), "DOMException code is:  " << e.code );
//               LOG_ERROR(logger_t(getLogger("gwdl")), "Message: " << S(e.msg) );
//           }
//           catch (...)
//           {
//               LOG_ERROR(logger_t(getLogger("gwdl")), "An error occurred creating the document" );
//           }
//   }
//
//   return el;
//}


//// Place
//Place deserializePlace(const string& xmlstring) {
//	/// ToDo: implement!
//	return Place(xmlstring);
//}
//
//string serializePlace(const Place& place) {
//	/// ToDo: implement!
//	Place myplace = place;
//	return "<place/>";
//}
//

} // end namespace gwdl

