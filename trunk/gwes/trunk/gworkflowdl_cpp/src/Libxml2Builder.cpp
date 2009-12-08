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
Libxml2Builder::Libxml2Builder() : _logger(fhg::log::getLogger("gwdl")) {}

/**
 * Destructor implementation.
 */
Libxml2Builder::~Libxml2Builder() {}

//////////////////////////
// Data
//////////////////////////
const Data::ptr_t Libxml2Builder::deserializeData(const string &xmlstring) const {
	Data::ptr_t data(new Data(xmlstring));
	return data;
}

const string Libxml2Builder::serializeData(const Data::ptr_t &data) const {
	return data->serialize();
}

const xmlNodePtr Libxml2Builder::dataToElement(const Data &data) const {
	xmlDocPtr docP = XMLUtils::Instance()->deserializeLibxml2(data.getContent());
	const xmlNodePtr nodeP = xmlDocGetRootElement(docP);
	xmlUnlinkNode(nodeP);
	xmlFreeDoc(docP);
	return nodeP;
}

//////////////////////////
// Token
//////////////////////////
const Token::ptr_t Libxml2Builder::deserializeToken(const string &xmlstring) const {
	xmlDocPtr docP = XMLUtils::Instance()->deserializeLibxml2(xmlstring);
	const xmlNodePtr nodeP = xmlDocGetRootElement(docP);
	const Token::ptr_t ret = elementToToken(nodeP);
	xmlFreeDoc(docP);
	return ret;
}

const string Libxml2Builder::serializeToken(const Token::ptr_t &token) const {
	return serializeToken(*token);
}

const Token::ptr_t Libxml2Builder::elementToToken(const xmlNodePtr nodeP) const {
	///////////////////
	// long _id;                -> set by constructor
    // Data::ptr_t _dataP;      -> set from element <data>
    // Properties _properties;  -> set from elements <property>
    // bool _control;           -> set from element <control>
    // Transition* _p_lock;     -> ignored.
	///////////////////
	
    xmlNodePtr curNodeP = NULL;

    for (curNodeP = nodeP; curNodeP; curNodeP = curNodeP->next) {
        if (curNodeP->type == XML_ELEMENT_NODE) {
            LOG_INFO(_logger, "node type: Element, name: " << curNodeP->name);
        }
    }
 	
	Token::ptr_t token(new Token());
	return token;
}

const string Libxml2Builder::serializeToken(const Token &token) const {
	xmlNodePtr nodeP = tokenToElement(token);
	const string ret = XMLUtils::Instance()->serializeLibxml2(nodeP, true); 
	xmlFreeNode(nodeP);
	return ret;
}

const xmlNodePtr Libxml2Builder::tokenToElement(const Token &token) const {
	// ToDo: namespace should be without prefix.
	xmlNsPtr nsP = xmlNewNs(NULL, (xmlChar*)"http://www.gridworkflow.org/gworkflowdl", (xmlChar*)"gwdl");
	xmlNodePtr nodeP = xmlNewNode(nsP, (xmlChar*)"token");
	xmlNodePtr curNodeP;
	
	///ToDo: properties
	//               // properties
	//               vector<DOMElement*> v = properties.toElements(doc);       
	//               for (unsigned int i = 0; i < v.size(); i++)
	//               { 
	//        	     el->appendChild(v[i]);
	//               }
	
	// data 
	if (token.isData()) {
		curNodeP = xmlNewChild(nodeP,nsP, (xmlChar*)"data",NULL);
		xmlAddChildList(curNodeP, dataToElement(* token.getData())); 
	} 
	// control
	else {
		if (token.getControl()) {
			xmlNewChild(nodeP,nsP, (xmlChar*)"control",(xmlChar*)"true");
		} else {
			xmlNewChild(nodeP,nsP, (xmlChar*)"control",(xmlChar*)"false");
		}
	}
	return nodeP;
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

} // end namespace gwdl

// Data
ostream& operator<<(ostream &out, gwdl::Data &data) {
	out << data.getContent();
	return out;
}

// Token
ostream& operator<<(ostream &out, gwdl::Token &token) {
	gwdl::Libxml2Builder builder;
	out << builder.serializeToken(token);
	return out;
}
