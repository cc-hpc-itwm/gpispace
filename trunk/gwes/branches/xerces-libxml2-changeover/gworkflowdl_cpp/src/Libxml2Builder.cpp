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
Libxml2Builder::Libxml2Builder() : _logger(fhg::log::getLogger("gwdl")) {
	// init libxml2 parser
	/// done in XMLUtils!
	//	xmlInitParser();
	//	LIBXML_TEST_VERSION
	
	// create gworkflowdl namespace
	// ToDo: namespace should be without prefix.
	_nsP = xmlNewNs(NULL, BAD_CAST "http://www.gridworkflow.org/gworkflowdl", BAD_CAST "gwdl");
}

/**
 * Destructor implementation.
 */
Libxml2Builder::~Libxml2Builder() {
	// cleanup xml parser
	/// done in XMLUtils!
	//	xmlCleanupParser();
}

//////////////////////////
// Data
//////////////////////////
//                <xs:element name="data">
//                    <xs:annotation>
//                        <xs:documentation>
//The data element can hold one single child element with arbitrary XML representing data (e.g., SOAP parameters) or
//references to data (e.g., filenames).
//                        </xs:documentation>
//                    </xs:annotation>
//                    <xs:complexType>
//                        <xs:sequence>
//                            <xs:any namespace="##any" processContents="lax" minOccurs="0"
//                                    maxOccurs="unbounded"/>
//                        </xs:sequence>
//                    </xs:complexType>
//                </xs:element>
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
//<xs:element name="token" minOccurs="0" maxOccurs="unbounded">
//    <xs:annotation>
//        <xs:documentation>
//A token represents state information, such as true/false (control token) or arbitrary XML (data token).
//        </xs:documentation>
//    </xs:annotation>
//    <xs:complexType>
//        <xs:sequence>
//            <xs:element ref="property" minOccurs="0" maxOccurs="unbounded"/>
//            <xs:choice minOccurs="0">
//                ===> refer to data <====
//                <xs:element name="control" type="xs:boolean">
//                    <xs:annotation>
//                        <xs:documentation>
//A control token can either be "true" or "false", reflecting the exit status of the previous transition.
//                        </xs:documentation>
//                    </xs:annotation>
//                </xs:element>
//            </xs:choice>
//        </xs:sequence>
//    </xs:complexType>
//</xs:element>
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
    	Properties::ptr_t propsP = elementsToProperties(curNodeP); // <property> ...
    	while (checkElementName(curNodeP, "property")) {
    		// skip all property elements, they have been already processed.
    		curNodeP = nextElementNode(curNodeP->next); 
    	}
    	
    	if (checkElementName(curNodeP, "data")) {               // <data>
        	curNodeP = nextElementNode(curNodeP->children);
        	if (curNodeP == NULL) {                               // empty data
        		Data::ptr_t dataP = Data::ptr_t(new Data(""));
        		if (propsP) tokenP.reset(new Token(propsP,dataP));
        		else tokenP.reset(new Token(dataP));
        	} else {                                              // data with content element
        		Data::ptr_t dataP = elementToData(curNodeP);
        		if (propsP) tokenP.reset(new Token(propsP,dataP));
        		else tokenP.reset(new Token(dataP));
        		// check for multiple children of <data> (MUST be single child!)
        		curNodeP = nextElementNode(curNodeP->next);
        		if (curNodeP != NULL) {
                	LOG_ERROR(_logger, "Element <data> MUST have exactly ONE child element!");
                	throw WorkflowFormatException("Element <data> MUST have exactly ONE child element!");
        		}
        	}
        } else if (checkElementName(curNodeP, "control")) {     // <control>
        	curNodeP = nextTextNode(curNodeP->children);
        	if ( xmlStrcmp(curNodeP->content,BAD_CAST "true")==0 ) {
        		if (propsP) tokenP.reset(new Token(propsP,Token::CONTROL_TRUE));
        		else tokenP.reset(new Token(Token::CONTROL_TRUE));
        	} else if ( xmlStrcmp(curNodeP->content,BAD_CAST "false")==0 ) {
        		if (propsP) tokenP.reset(new Token(propsP,Token::CONTROL_FALSE));
        		else tokenP.reset(new Token(Token::CONTROL_FALSE));
        	} else {
            	LOG_ERROR(_logger, "Invalid text content of element <control>! MUST be \"true\" or \"false\" but is \"" << curNodeP->content << "\"");
            	throw WorkflowFormatException("Invalid text content of element <control>! MUST be \"true\" or \"false\"");
        	}
        } else {
        	LOG_ERROR(_logger, "Element <data> or <control> not found as child of <token>!");
        	throw WorkflowFormatException("Element <data> or <control> not found as child of <token>!");
        }
    } else {
    	LOG_ERROR(_logger, "Element <token> not found!");
    	throw WorkflowFormatException("Element <token> not found!"); 
    }

	return tokenP;
}

xmlNodePtr Libxml2Builder::tokenToElement(const Token &token) const {
	xmlNodePtr nodeP = xmlNewNode(_nsP, BAD_CAST "token");
	xmlNodePtr curNodeP;
	
	// properties
	Properties::ptr_t propsP = token.readProperties();
	if (propsP != NULL) {
		curNodeP = propertiesToElements(*propsP);
		if (curNodeP != NULL) {
			xmlAddChildList(nodeP, curNodeP); 
		}
	}
	
	// data 
	if (token.isData()) {
		curNodeP = xmlNewChild(nodeP,_nsP, BAD_CAST "data",NULL);
		xmlAddChildList(curNodeP, dataToElement(* token.getData())); 
	} 
	// control
	else {
		if (token.getControl()) {
			xmlNewChild(nodeP,_nsP, BAD_CAST "control",BAD_CAST "true");
		} else {
			xmlNewChild(nodeP,_nsP, BAD_CAST "control",BAD_CAST "false");
		}
	}
	return nodeP;
}

//////////////////////////
// Properties
//////////////////////////
//<xs:element name="property">
//    <xs:annotation>
//        <xs:documentation>
//Generic property. The attribute "name" specifies the property name,
//the element contents its value.
//        </xs:documentation>
//    </xs:annotation>
//    <xs:complexType>
//        <xs:simpleContent>
//            <xs:extension base="xs:string">
//                <xs:attribute name="name" use="required" type="xs:string"/>
//            </xs:extension>
//        </xs:simpleContent>
//    </xs:complexType>
//</xs:element>
//////////////////////////

string Libxml2Builder::serializeProperties(const Properties& props) const {
	xmlNodePtr nodeP = propertiesToElements(props);
	const string ret = XMLUtils::Instance()->serializeLibxml2(nodeP, true); 
	xmlFreeNode(nodeP);
	return ret;
}

/**
 * Returns NULL if there are no properties.
 */
Properties::ptr_t Libxml2Builder::elementsToProperties(const xmlNodePtr nodeP) const throw (WorkflowFormatException) {
    xmlNodePtr curNodeP = nodeP;
    xmlNodePtr textNodeP;
    curNodeP = nextElementNode(curNodeP);
    Properties::ptr_t propP;
    while(curNodeP) {
    	if (checkElementName(curNodeP, "property")) {
    		if (propP == NULL) propP = Properties::ptr_t(new Properties());
    		string name = string((const char*) xmlGetProp(curNodeP, BAD_CAST "name")); 
    		textNodeP = nextTextNode(curNodeP->children);
    		if (textNodeP == NULL) {
    			LOG_INFO(_logger, "generating empty property with name \"" << name << "\"");
    			propP->put( name, string("") );
    		} else {
    			string text = string( (const char*) textNodeP->content);
    			LOG_INFO(_logger, "generating property with name \"" << name << "\" and value \"" << text << "\"");
        		propP->put( name, text ); 
    		}
    	} else if (propP) { // there MUST NOT be any other element between property elements!
    		break;
    	}
    	curNodeP = nextElementNode(curNodeP->next);
    }
    return propP;
}

xmlNodePtr Libxml2Builder::propertiesToElements(const Properties& props) const {
	xmlNodePtr firstNodeP;
	xmlNodePtr lastNodeP;
	xmlNodePtr curNodeP;
	xmlNodePtr textNodeP;
	bool first = true;
	for(CITR_Properties it = props.begin(); it != props.end(); ++it) {
		curNodeP = xmlNewNode(_nsP, BAD_CAST "property");
	    xmlNewProp(curNodeP, BAD_CAST "name", BAD_CAST it->first.c_str());
	    if (!it->second.empty()) {
	    	textNodeP = xmlNewText(BAD_CAST it->second.c_str());
	    	xmlAddChild(curNodeP, textNodeP);
	    }
		if (first) {
			firstNodeP = curNodeP,
			first = false;
		} else {
			xmlAddNextSibling(lastNodeP,curNodeP);
		}
		lastNodeP = curNodeP;
	}
	
	return firstNodeP;
}

//////////////////////////
// Place
//////////////////////////
//<xs:element name="place" minOccurs="0" maxOccurs="unbounded">
//    <xs:annotation>
//        <xs:documentation>
//A place is a placeholder for tokens that represent the data and the state of the workflow.
//        </xs:documentation>
//    </xs:annotation>
//    <xs:complexType>
//        <xs:sequence>
//            <xs:element ref="description" minOccurs="0"/>
//            <xs:element ref="property" minOccurs="0" maxOccurs="unbounded"/>
//            <xs:element name="tokenClass" minOccurs="0">
//                <xs:annotation>
//                    <xs:documentation>
//Description of the class of tokens hold by a place.
//                    </xs:documentation>
//                </xs:annotation>
//                <xs:complexType>
//                    <xs:sequence>
//                        <xs:element ref="owl" minOccurs="0" maxOccurs="unbounded"/>
//                    </xs:sequence>
//                    <xs:attribute name="type" type="xs:string"/>
//                </xs:complexType>
//            </xs:element>
//            ====> refer to token <====
//        </xs:sequence>
//        <xs:attribute name="ID" use="required" type="xs:ID"/>
//        <xs:attribute name="capacity" type="xs:integer"/>
//    </xs:complexType>
//////////////////////////

Place::ptr_t Libxml2Builder::deserializePlace(const string &xmlstring) const throw (WorkflowFormatException) {
	xmlDocPtr docP = XMLUtils::Instance()->deserializeLibxml2(xmlstring);
	const xmlNodePtr nodeP = xmlDocGetRootElement(docP);
	const Place::ptr_t ret = elementToPlace(nodeP);
	xmlFreeDoc(docP);
	return ret;
}

string Libxml2Builder::serializePlace(const Place &place) const {
	xmlNodePtr nodeP = placeToElement(place);
	const string ret = XMLUtils::Instance()->serializeLibxml2(nodeP, true); 
	xmlFreeNode(nodeP);
	return ret;
}

Place::ptr_t Libxml2Builder::elementToPlace(const xmlNodePtr nodeP) const throw (WorkflowFormatException) {
	///////////////////
	// 	std::string _id;                   -> from attribute <place ID="">
    //  unsigned int _capacity;            -> from attribute <place capacity="">
    //  std::string _description;          -> from child element <description>
    //  Properties::ptr_t _propertiesP;    -> from child elements <property>
	//  std::string _tokenType;            -> from attribute of child element <tokenClass type="">
	//  std::vector<Token::ptr_t> _tokens; -> from child elements <token>
    //  Token::ptr_t _nextUnlockedTokenP;  -> ignored (calculated on demand)
	///////////////////
	
	Place::ptr_t placeP;
    xmlNodePtr curNodeP = nodeP;
    xmlNodePtr textNodeP;
    
    curNodeP = nextElementNode(curNodeP);

    if (checkElementName(curNodeP, "place")) {                             // <place>
		string id((const char*) xmlGetProp(curNodeP, BAD_CAST "ID"));      // <place ID="">
		placeP = Place::ptr_t(new Place(id));
		xmlChar* xmlCharP = xmlGetProp(curNodeP, BAD_CAST "capacity");     // <place capacity="">
		placeP->setCapacity((xmlCharP) ? atoi((const char*) xmlCharP) : Place_DEFAULT_CAPACITY);
		curNodeP = nextElementNode(curNodeP->children);
		// extract all property elements
		Properties::ptr_t propsP = elementsToProperties(curNodeP);         //   <property>
		if (propsP) placeP->setProperties(propsP);
		while(curNodeP) {
		    if (checkElementName(curNodeP, "description")) {               //   <description>
		    	textNodeP = nextTextNode(curNodeP->children);
		    	if (textNodeP) {
		    		placeP->setDescription(string( (const char*) textNodeP->content)); 
		    	}
		    } else if (checkElementName(curNodeP, "property")) {           //   (<property>)
		    	// do nothing, properties have already been extracted before
		    } else if (checkElementName(curNodeP, "tokenClass")) {         //   <tokenClass>
				xmlCharP = xmlGetProp(curNodeP, BAD_CAST "type"); //   <tokenClass type="">
		    	if (xmlCharP) {
		    		placeP->setTokenType(string((const char*) xmlCharP)); 
		    	}
		    } else if (checkElementName(curNodeP, "token")) {              //   <token>
	    		placeP->addToken(elementToToken(curNodeP));
		    } else {
		    	ostringstream oss;
		    	oss << "Unknown element <" << curNodeP->name << ">";
		    	LOG_ERROR(_logger, oss.str());
		    	throw WorkflowFormatException(oss.str()); 
		    }
		    // next element
			curNodeP = nextElementNode(curNodeP->next);
		}
    } else {
    	LOG_ERROR(_logger, "Element <place> not found!");
    	throw WorkflowFormatException("Element <place> not found!"); 
    }
    
	return placeP;
}

xmlNodePtr Libxml2Builder::placeToElement(const Place &place) const {
	xmlNodePtr nodeP = xmlNewNode(_nsP, BAD_CAST "place");
	xmlNodePtr curNodeP;
	
	// <place ID="">
    xmlNewProp(nodeP, BAD_CAST "ID", BAD_CAST place.getID().c_str());
    
    // <place capacity="">
    if (place.getCapacity() != Place_DEFAULT_CAPACITY) {
    	ostringstream oss;
    	oss << place.getCapacity(); 
        xmlNewProp(nodeP, BAD_CAST "capacity", BAD_CAST oss.str().c_str());
    }
    
    //   <description>
    if (place.getDescription().size()>0) {
		xmlNewChild(nodeP,_nsP, BAD_CAST "description",BAD_CAST place.getDescription().c_str());
    }

    //   <property>
	Properties::ptr_t propsP = place.readProperties();
	if (propsP) {
		curNodeP = propertiesToElements(*propsP);
		if (curNodeP) {
			xmlAddChildList(nodeP, curNodeP); 
		}
	}
	
	//   <tokenClass type="">
	if (place.getTokenType().size() > 0) {
		curNodeP = xmlNewChild(nodeP,_nsP, BAD_CAST "tokenClass",NULL);
		xmlNewProp(curNodeP, BAD_CAST "type", BAD_CAST place.getTokenType().c_str());
	}
	
	//   <token>
	std::vector<Token::ptr_t> tokens = place.getTokens();
	for (size_t i = 0; i < tokens.size(); i++) {
		xmlAddChild(nodeP, tokenToElement(*tokens[i]));
	}
	
	return nodeP;
}

//////////////////////////
// private helper methods
//////////////////////////

xmlNodePtr Libxml2Builder::nextElementNode(const xmlNodePtr nodeP) {
	xmlNodePtr curNodeP = nodeP;
    while (curNodeP != NULL && curNodeP->type != XML_ELEMENT_NODE) curNodeP = curNodeP->next;
    return curNodeP;
}

xmlNodePtr Libxml2Builder::nextTextNode(const xmlNodePtr nodeP) {
	xmlNodePtr curNodeP = nodeP;
    while (curNodeP != NULL && curNodeP->type != XML_TEXT_NODE) curNodeP = curNodeP->next;
    return curNodeP;
}

bool Libxml2Builder::checkElementName(const xmlNodePtr nodeP,const char* name) {
	if (nodeP == NULL) {
    	LOG_WARN(logger_t(getLogger("gwdl")), "Node pointer is NULL; element <" << name << "> not found!");
		return false;
	}
	return xmlStrcmp(nodeP->name,BAD_CAST name)==0;
}

} // end namespace gwdl

//////////////////////////
// << operators
//////////////////////////

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

// Properties
ostream& operator<<(ostream &out, gwdl::Properties &props) {	
	gwdl::Libxml2Builder builder;
	out << builder.serializeProperties(props);
	return out;
}

// Place
ostream& operator<<(ostream &out, gwdl::Place &place) {
	gwdl::Libxml2Builder builder;
	out << builder.serializePlace(place);
	return out;
}

