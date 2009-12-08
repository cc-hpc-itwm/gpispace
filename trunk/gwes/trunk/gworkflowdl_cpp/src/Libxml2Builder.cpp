/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
//gwdl
#include <gwdl/Libxml2Builder.h>
//libxml2
#include <libxml/tree.h>

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
Data::ptr_t Libxml2Builder::deserializeData(const string &xmlstring) const throw (WorkflowFormatException) {
	Data::ptr_t data(new Data(xmlstring));
	return data;
}

string Libxml2Builder::serializeData(const Data& data) const {
	return data.serialize();
}

Data::ptr_t Libxml2Builder::elementToData(const xmlNodePtr nodeP) const throw (WorkflowFormatException) {
	///////////////////
	// int _type;               -> ignored/set by constructor
    // content_t _content;      -> set from element contents
	///////////////////
	string contents = XMLUtils::Instance()->serializeLibxml2(nodeP, false); 
	Data::ptr_t dataP(new Data(contents));
	return dataP;
}

xmlNodePtr Libxml2Builder::dataToElement(const Data &data) const {
	xmlDocPtr docP = XMLUtils::Instance()->deserializeLibxml2(data.getContent());
	const xmlNodePtr nodeP = xmlDocGetRootElement(docP);
	xmlUnlinkNode(nodeP);
	xmlFreeDoc(docP);
	return nodeP;
}

//////////////////////////
// Token
//////////////////////////
Token::ptr_t Libxml2Builder::deserializeToken(const string &xmlstring) const throw (WorkflowFormatException) {
	xmlDocPtr docP = XMLUtils::Instance()->deserializeLibxml2(xmlstring);
	const xmlNodePtr nodeP = xmlDocGetRootElement(docP);
	const Token::ptr_t ret = elementToToken(nodeP);
	xmlFreeDoc(docP);
	return ret;
}

string Libxml2Builder::serializeToken(const Token &token) const {
	xmlNodePtr nodeP = tokenToElement(token);
	const string ret = XMLUtils::Instance()->serializeLibxml2(nodeP, true); 
	xmlFreeNode(nodeP);
	return ret;
}

Token::ptr_t Libxml2Builder::elementToToken(const xmlNodePtr nodeP) const throw (WorkflowFormatException) {
	///////////////////
	// long _id;                -> set by constructor
    // Data::ptr_t _dataP;      -> set from element <data>
    // Properties _properties;  -> set from elements <property>
    // bool _control;           -> set from element <control>
    // Transition* _p_lock;     -> ignored.
	///////////////////
	
	Token::ptr_t tokenP;
    xmlNodePtr curNodeP = nodeP;
    
    curNodeP = nextElementNode(curNodeP);

    if (checkElementName(curNodeP, "token")) {            // <token>
        curNodeP = nextElementNode(curNodeP->children);
    	///ToDo: properties
        LOG_WARN(_logger, "///ToDo: refractoring from xerces-c to libxml2! ///properties");
        if (checkElementName(curNodeP, "data")) {           // <data>
        	curNodeP = nextElementNode(curNodeP->children);
        	if (curNodeP == NULL) {                           // empty data
        		Data::ptr_t dataP = Data::ptr_t(new Data(""));
        		tokenP.reset(new Token(dataP));
        	} else {                                          // data with content element
        		Data::ptr_t dataP = elementToData(curNodeP);
        		tokenP.reset(new Token(dataP));
        		// check for multiple children of <data> (MUST be single child!)
        		curNodeP = nextElementNode(curNodeP->next);
        		if (curNodeP != NULL) {
                	LOG_ERROR(_logger, "Element <data> MUST have exactly ONE child element!");
                	throw WorkflowFormatException("Element <data> MUST have exactly ONE child element!");
        		}
        	}
        } else if (checkElementName(curNodeP, "control")) { // <control>
        	curNodeP = nextTextNode(curNodeP->children);
        	if ( xmlStrcmp(curNodeP->content,(xmlChar*)"true")==0 ) {
            	tokenP.reset(new Token());
        	} else if ( xmlStrcmp(curNodeP->content,(xmlChar*)"false")==0 ) {
            	tokenP.reset(new Token(Token::CONTROL_FALSE));
        	} else {
            	LOG_ERROR(_logger, "Invalid text content of element <control>! MUST be \"true\" or \"false\" but is \"" << curNodeP->content << "\"");
            	throw WorkflowFormatException("Invalid text content of element <control>! MUST be \"true\" or \"false\"");
        	}
        } else {
        	LOG_ERROR(_logger, "Element <data> or <control> not found as child of <token>!");
        	throw WorkflowFormatException("Element <data> or <control> not found as child of <token>!");
        }
    } else {
    	LOG_WARN(_logger, "Element <token> not found!");
    	throw WorkflowFormatException("Element <token> not found!"); 
    }

	return tokenP;
}

xmlNodePtr Libxml2Builder::nextElementNode(xmlNodePtr nodeP) {
    while (nodeP != NULL && nodeP->type != XML_ELEMENT_NODE) nodeP = nodeP->next;
    return nodeP;
}

xmlNodePtr Libxml2Builder::nextTextNode(xmlNodePtr nodeP) {
    while (nodeP != NULL && nodeP->type != XML_TEXT_NODE) nodeP = nodeP->next;
    return nodeP;
}

bool Libxml2Builder::checkElementName(const xmlNodePtr nodeP,const char* name) {
	if (nodeP == NULL) {
    	LOG_WARN(logger_t(getLogger("gwdl")), "Node pointer is NULL; element <" << name << "> not found!");
		return false;
	}
	return xmlStrcmp(nodeP->name,(xmlChar*)name)==0;
}

xmlNodePtr Libxml2Builder::tokenToElement(const Token &token) const {
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
