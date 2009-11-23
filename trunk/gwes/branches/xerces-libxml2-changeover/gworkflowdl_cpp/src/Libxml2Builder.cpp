/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
//gwdl
#include <gwdl/Libxml2Builder.h>
#include <gwdl/XMLUtils.h>
//std
#include <fstream>
#include <errno.h>

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
	_nsGworkflowdlP = xmlNewNs(NULL, BAD_CAST "http://www.gridworkflow.org/gworkflowdl", BAD_CAST "gwdl");
	_nsOperationclassP = xmlNewNs(NULL, BAD_CAST "http://www.gridworkflow.org/gworkflowdl/operationclass", BAD_CAST "oc");
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
	string contents = XMLUtils::Instance()->serializeLibxml2NodeList(nodeP, false); 
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
	const string ret = XMLUtils::Instance()->serializeLibxml2Node(nodeP, true); 
	xmlFreeNode(nodeP);
	return ret;
}

Token::ptr_t Libxml2Builder::elementToToken(const xmlNodePtr nodeP) const throw (WorkflowFormatException) {
	///////////////////
	// long _id;                -> set by constructor
    // Data::ptr_t _dataP;      -> set from element <data>
    // Properties _properties;  -> set from elements <property>
    // bool _control;           -> set from element <control>
    // Transition::ptr_t _p_lock;     -> ignored.
	///////////////////
	
	Token::ptr_t tokenP;
    xmlNodePtr curNodeP = nodeP;
    
    curNodeP = nextElementNode(curNodeP);

    if (checkElementName(curNodeP, "token")) {            // <token>
        curNodeP = nextElementNode(curNodeP->children);
    	Properties props = elementsToProperties(curNodeP); // <property> ...
    	while (checkElementName(curNodeP, "property")) {
    		// skip all property elements, they have been already processed.
    		curNodeP = nextElementNode(curNodeP->next); 
    	}
    	
    	if (checkElementName(curNodeP, "data")) {               // <data>
        	curNodeP = nextElementNode(curNodeP->children);
        	if (curNodeP == NULL) {                               // empty data
        		Data::ptr_t dataP = Data::ptr_t(new Data(""));
        		tokenP.reset(new Token(props,dataP));
        	} else {                                              // data with content element
        		Data::ptr_t dataP = elementToData(curNodeP);
        		tokenP.reset(new Token(props,dataP));
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
        		tokenP.reset(new Token(props,Token::CONTROL_TRUE));
        	} else if ( xmlStrcmp(curNodeP->content,BAD_CAST "false")==0 ) {
        		tokenP.reset(new Token(props,Token::CONTROL_FALSE));
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
	xmlNodePtr nodeP = xmlNewNode(_nsGworkflowdlP, BAD_CAST "token");
	xmlNodePtr curNodeP;
	
	// properties
	Properties props = token.readProperties();
	if (!props.empty()) {
		curNodeP = propertiesToElements(props);
		if (curNodeP) {
			xmlAddChildList(nodeP, curNodeP); 
		}
	}
	
	// data 
	if (token.isData()) {
		curNodeP = xmlNewChild(nodeP,_nsGworkflowdlP, BAD_CAST "data",NULL);
		xmlAddChildList(curNodeP, dataToElement(* token.getData())); 
	} 
	// control
	else {
		if (token.getControl()) {
			xmlNewChild(nodeP,_nsGworkflowdlP, BAD_CAST "control",BAD_CAST "true");
		} else {
			xmlNewChild(nodeP,_nsGworkflowdlP, BAD_CAST "control",BAD_CAST "false");
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
	const string ret = XMLUtils::Instance()->serializeLibxml2NodeList(nodeP, true); 
	xmlFreeNode(nodeP);
	return ret;
}

Properties Libxml2Builder::elementsToProperties(const xmlNodePtr nodeP) const throw (WorkflowFormatException) {
    xmlNodePtr curNodeP = nodeP;
    xmlNodePtr textNodeP;
    curNodeP = nextElementNode(curNodeP);
    Properties props;
    while(curNodeP) {
    	if (checkElementName(curNodeP, "property")) {
    		string name = string((const char*) xmlGetProp(curNodeP, BAD_CAST "name")); 
    		textNodeP = nextTextNode(curNodeP->children);
    		if (textNodeP == NULL) {
    			props.put( name, string("") );
    		} else {
    			string text = string( (const char*) textNodeP->content);
        		props.put( name, text ); 
    		}
    	} else if (!props.empty()) { // there MUST NOT be any other element between property elements!
    		break;
    	}
    	curNodeP = nextElementNode(curNodeP->next);
    }
    return props;
}

xmlNodePtr Libxml2Builder::propertiesToElements(const Properties& props) const {
	xmlNodePtr firstNodeP;
	xmlNodePtr lastNodeP;
	xmlNodePtr curNodeP;
	xmlNodePtr textNodeP;
	bool first = true;
	for(CITR_Properties it = props.begin(); it != props.end(); ++it) {
		curNodeP = xmlNewNode(_nsGworkflowdlP, BAD_CAST "property");
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
	const string ret = XMLUtils::Instance()->serializeLibxml2Node(nodeP, true); 
	xmlFreeNode(nodeP);
	return ret;
}

Place::ptr_t Libxml2Builder::elementToPlace(const xmlNodePtr nodeP) const throw (WorkflowFormatException) {
	///////////////////
	// 	std::string _id;                   -> from attribute <place ID="">
    //  unsigned int _capacity;            -> from attribute <place capacity="">
    //  std::string _description;          -> from child element <description>
    //  Properties _properties;    -> from child elements <property>
	//  std::string _tokenType;            -> from attribute of child element <tokenClass type="">
	//  std::vector<Token::ptr_t> _tokens; -> from child elements <token>
    //  Token::ptr_t _nextUnlockedTokenP;  -> ignored (calculated on demand)
	///////////////////
	
	Place::ptr_t placeP;
    xmlNodePtr curNodeP = nodeP;
    xmlNodePtr textNodeP;
    
    curNodeP = nextElementNode(curNodeP);
    
    // <place>
    if (checkElementName(curNodeP, "place")) {
    	// <place ID="">
		string id((const char*) xmlGetProp(curNodeP, BAD_CAST "ID"));      
		placeP = Place::ptr_t(new Place(id));
		// <place capacity="">
		xmlChar* xmlCharP = xmlGetProp(curNodeP, BAD_CAST "capacity");     
		placeP->setCapacity((xmlCharP) ? atoi((const char*) xmlCharP) : Place_DEFAULT_CAPACITY);
		curNodeP = nextElementNode(curNodeP->children);
		
		//   <description>
	    if (curNodeP && checkElementName(curNodeP, "description")) {               
	    	textNodeP = nextTextNode(curNodeP->children);
	    	if (textNodeP) {
	    		placeP->setDescription(string( (const char*) textNodeP->content)); 
	    	}
			curNodeP = nextElementNode(curNodeP->next);
	    }
	    
		// <property>
		// extract all property elements
		Properties props = elementsToProperties(curNodeP);         
		if (!props.empty()) placeP->setProperties(props);
		while(curNodeP && checkElementName(curNodeP, "property")) {           
			// do nothing, properties have already been extracted before
			curNodeP = nextElementNode(curNodeP->next);
		}
	    
		//   <tokenClass>
	    if (curNodeP && checkElementName(curNodeP, "tokenClass")) {       
	    	//   <tokenClass type="">
	    	xmlCharP = xmlGetProp(curNodeP, BAD_CAST "type"); 
	    	if (xmlCharP) {
	    		placeP->setTokenType(string((const char*) xmlCharP)); 
	    	}
			curNodeP = nextElementNode(curNodeP->next);
	    }
	    
	    //   <token>
	    while(curNodeP && checkElementName(curNodeP, "token")) { 
	    	Token::ptr_t token = elementToToken(curNodeP); 
	    	placeP->addToken(token);
			curNodeP = nextElementNode(curNodeP->next);
	    }

		if (curNodeP) {
			ostringstream oss;
			oss << "Unknown element <" << curNodeP->name << "> as child of <place ID=\"" << id << "\">";
			LOG_ERROR(_logger, oss.str());
			throw WorkflowFormatException(oss.str()); 
		}
    } else {
    	LOG_ERROR(_logger, "Element <place> not found!");
    	throw WorkflowFormatException("Element <place> not found!"); 
    }
    
	return placeP;
}

xmlNodePtr Libxml2Builder::placeToElement(const Place &place) const {
	xmlNodePtr nodeP = xmlNewNode(_nsGworkflowdlP, BAD_CAST "place");
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
		xmlNewChild(nodeP,_nsGworkflowdlP, BAD_CAST "description",BAD_CAST place.getDescription().c_str());
    }

    //   <property>
	Properties props = place.readProperties();
	if (!props.empty()) {
		curNodeP = propertiesToElements(props);
		if (curNodeP) {
			xmlAddChildList(nodeP, curNodeP); 
		}
	}
	
	//   <tokenClass type="">
	if (place.getTokenType().size() > 0) {
		curNodeP = xmlNewChild(nodeP,_nsGworkflowdlP, BAD_CAST "tokenClass",NULL);
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
// OperationCandidate
//////////////////////////
//					<xsd:element name = "operationCandidate">
//						<xsd:complexType>
//							<xsd:sequence>
//								<xsd:element ref = "owl" minOccurs = "0" maxOccurs = "unbounded"/>
//							</xsd:sequence>
//                          <xsd:attribute name = "type" type = "xsd:string"/>
//							<xsd:attribute name = "operationName" type = "xsd:string"/>
//							<xsd:attribute name = "resourceName" type = "xsd:string"/>
//							<xsd:attribute name = "quality" type = "xsd:float"/>
//                          <xsd:attribute name = "selected" default = "false" type = "xsd:boolean"/>
//						</xsd:complexType>
//					</xsd:element>
//////////////////////////

OperationCandidate::ptr_t Libxml2Builder::deserializeOperationCandidate(const string &xmlstring) const throw (WorkflowFormatException) {
	xmlDocPtr docP = XMLUtils::Instance()->deserializeLibxml2(xmlstring);
	const xmlNodePtr nodeP = xmlDocGetRootElement(docP);
	const OperationCandidate::ptr_t ret = elementToOperationCandidate(nodeP);
	xmlFreeDoc(docP);
	return ret;
}

string Libxml2Builder::serializeOperationCandidate(const OperationCandidate &operationCandidate) const {
	xmlNodePtr nodeP = operationCandidateToElement(operationCandidate);
	const string ret = XMLUtils::Instance()->serializeLibxml2Node(nodeP, true); 
	xmlFreeNode(nodeP);
	return ret;
}

OperationCandidate::ptr_t Libxml2Builder::elementToOperationCandidate(const xmlNodePtr nodeP) const throw (WorkflowFormatException) {
	///////////////////
	// long _id;                       -> generated by constructor
	// bool _selected;                 -> from attribute <operationCandidate selected="">
	// std::string _type;              -> from attribute <operationCandidate type=""> 
	// std::string _operationName;     -> from attribute <operationCandidate operationName="">
	// std::string _resourceName;      -> from attribute <operationCandidate resourceName="">
	// float _quality;                 -> from attribute <operationCandidate quality="">
	///////////////////
	
	OperationCandidate::ptr_t operationCandidateP;
    xmlNodePtr curNodeP = nodeP;
    xmlChar* xmlCharP;
    
    curNodeP = nextElementNode(curNodeP);
    if (checkElementName(curNodeP, "operationCandidate")) {                             // <operationCandidate>
    	operationCandidateP = OperationCandidate::ptr_t(new OperationCandidate());
		xmlCharP = xmlGetProp(curNodeP, BAD_CAST "selected");                           // <operationCandidate selected="">
		if (xmlCharP) {
			if ( xmlStrcmp(xmlCharP,BAD_CAST "true")==0 ) {
				operationCandidateP->setSelected(true);
			} else if ( xmlStrcmp(xmlCharP,BAD_CAST "false")==0 ) {
				operationCandidateP->setSelected(false);
			} else {
				LOG_ERROR(_logger, "Invalid value for attribute <selected>! MUST be \"true\" or \"false\" but is \"" << xmlCharP << "\"");
				throw WorkflowFormatException("Invalid value for attribute <selected>! MUST be \"true\" or \"false\"");
			}
		} else {
			operationCandidateP->setSelected(false);
		}
		xmlCharP = xmlGetProp(curNodeP, BAD_CAST "type");                               // <operationCandidate type="">
		if (xmlCharP) {
			operationCandidateP->setType( string( (const char*)xmlCharP ) );
		}
		xmlCharP = xmlGetProp(curNodeP, BAD_CAST "operationName");                      // <operationCandidate operationName="">
		if (xmlCharP) {
			operationCandidateP->setOperationName( string( (const char*)xmlCharP ) );
		}
		xmlCharP = xmlGetProp(curNodeP, BAD_CAST "resourceName");                       // <operationCandidate resourceName="">
		if (xmlCharP) {
			operationCandidateP->setResourceName( string( (const char*)xmlCharP ) );
		}
		xmlCharP = xmlGetProp(curNodeP, BAD_CAST "quality");                            // <operationCandidate quality="">
		if (xmlCharP) {
			operationCandidateP->setQuality( atof( (const char*)xmlCharP ) );
		} else {
			operationCandidateP->setQuality( -1 );
		}
    } else {
    	LOG_ERROR(_logger, "Element <operationCandidate> not found!");
    	throw WorkflowFormatException("Element <operationCandidate> not found!"); 
    }
    
	return operationCandidateP;
}

xmlNodePtr Libxml2Builder::operationCandidateToElement(const OperationCandidate &operationCandidate) const {
	// <operationCandidate>
	xmlNodePtr nodeP = xmlNewNode(_nsOperationclassP, BAD_CAST "operationCandidate");

	// <operationCandidate selected="">
	if (operationCandidate.isSelected()) {
	    xmlNewProp(nodeP, BAD_CAST "selected", BAD_CAST "true");
	}
	// <operationCandidate type="">
    if (operationCandidate.getType().size()>0) {
    	xmlNewProp(nodeP, BAD_CAST "type", BAD_CAST operationCandidate.getType().c_str());
    }
    // <operationCandidate operationName="">
    if (operationCandidate.getOperationName().size()>0) {
    	xmlNewProp(nodeP, BAD_CAST "operationName", BAD_CAST operationCandidate.getOperationName().c_str());
    }
    // <operationCandidate resourceName="">
    if (operationCandidate.getResourceName().size()>0) {
    	xmlNewProp(nodeP, BAD_CAST "resourceName", BAD_CAST operationCandidate.getResourceName().c_str());
    }
    // <operationCandidate quality="">
    if (operationCandidate.getQuality()>-1.0) {
    	ostringstream oss;
    	oss << operationCandidate.getQuality();
        xmlNewProp(nodeP, BAD_CAST "quality", BAD_CAST oss.str().c_str());
    }
	
	return nodeP;
}

//////////////////////////
// OperationClass
//////////////////////////
//	<xsd:element name = "operationClass">
//		<xsd:complexType>
//			<xsd:sequence>
//				<xsd:element ref = "owl" minOccurs = "0" maxOccurs = "unbounded"/>
//				<xsd:choice minOccurs = "0" maxOccurs = "unbounded">
//              ===> refer to operation candidate <====
//				</xsd:choice>
//			</xsd:sequence>
//			<xsd:attribute name = "name" type = "xsd:string"/>
//		</xsd:complexType>
//	</xsd:element>
//	<xsd:element name = "owl" type = "xsd:string"/>
//////////////////////////

OperationClass::ptr_t Libxml2Builder::deserializeOperationClass(const string &xmlstring) const throw (WorkflowFormatException) {
	xmlDocPtr docP = XMLUtils::Instance()->deserializeLibxml2(xmlstring);
	const xmlNodePtr nodeP = xmlDocGetRootElement(docP);
	const OperationClass::ptr_t ret = elementToOperationClass(nodeP);
	xmlFreeDoc(docP);
	return ret;
}

string Libxml2Builder::serializeOperationClass(const OperationClass &operationClass) const {
	xmlNodePtr nodeP = operationClassToElement(operationClass);
	const string ret = XMLUtils::Instance()->serializeLibxml2Node(nodeP, true); 
	xmlFreeNode(nodeP);
	return ret;
}

OperationClass::ptr_t Libxml2Builder::elementToOperationClass(const xmlNodePtr nodeP) const throw (WorkflowFormatException) {
	///////////////////
	// std::vector<OperationCandidate::ptr_t> _operationCandidates;   -> child elements <operationCandidate>
    // std::string _name;                                             -> attribute <operationClass name="">
	///////////////////
	
	OperationClass::ptr_t operationClassP;
    xmlNodePtr curNodeP = nodeP;
    xmlChar* xmlCharP;
    
    curNodeP = nextElementNode(curNodeP);
    if (checkElementName(curNodeP, "operationClass")) {                             // <operationClass>
    	operationClassP = OperationClass::ptr_t(new OperationClass());
		xmlCharP = xmlGetProp(curNodeP, BAD_CAST "name");                           // <operationClass name="">
		if (xmlCharP) {
			operationClassP->setName( string( (const char*)xmlCharP ) );
		}
		// children
		curNodeP = nextElementNode(curNodeP->children);
		while(curNodeP) {
		    if (checkElementName(curNodeP, "operationCandidate")) {              //   <operationCandidate>
		    	OperationCandidate::ptr_t ocand = elementToOperationCandidate(curNodeP);
	    		operationClassP->addOperationCandidate(ocand);
		    } else {
		    	ostringstream oss;
		    	oss << "Unknown element <" << curNodeP->name << ">. Expected <operationCandidate>";
		    	LOG_ERROR(_logger, oss.str());
		    	throw WorkflowFormatException(oss.str()); 
		    }
		    // next element
			curNodeP = nextElementNode(curNodeP->next);
		}
    } else {
    	LOG_ERROR(_logger, "Element <operationClass> not found!");
    	throw WorkflowFormatException("Element <operationClass> not found!"); 
    }
    
	return operationClassP;
}

xmlNodePtr Libxml2Builder::operationClassToElement(const OperationClass &operationClass) const {
	// <operationClass>
	xmlNodePtr nodeP = xmlNewNode(_nsOperationclassP, BAD_CAST "operationClass");
	
	// <operationClass name="">
    if (operationClass.getName().size()>0) {
    	xmlNewProp(nodeP, BAD_CAST "name", BAD_CAST operationClass.getName().c_str());
    }
    
    //   <operationCandidate>
	std::vector<OperationCandidate::ptr_t> operationCandidates = operationClass.getOperationCandidates();
	for (size_t i = 0; i < operationCandidates.size(); i++) {
		xmlAddChild(nodeP, operationCandidateToElement(*operationCandidates[i]));
	}
	
	return nodeP;
}

//////////////////////////
// Operation
//////////////////////////
//<xs:element name="operation" minOccurs="0">
//    <xs:annotation>
//        <xs:documentation>
//The element "operation" links transitions with (external) operations. As operations may be platform-specific, they are
//specified in a separate XML Schema using different namespace.
//        </xs:documentation>
//    </xs:annotation>
//    <xs:complexType>
//        <xs:sequence>
//            <xs:any namespace="##any" processContents="lax" minOccurs="0"/>
//            ===> refer to <operationClass> <==
//        </xs:sequence>
//    </xs:complexType>
//</xs:element>
//////////////////////////

Operation::ptr_t Libxml2Builder::deserializeOperation(const string &xmlstring) const throw (WorkflowFormatException) {
	xmlDocPtr docP = XMLUtils::Instance()->deserializeLibxml2(xmlstring);
	const xmlNodePtr nodeP = xmlDocGetRootElement(docP);
	const Operation::ptr_t ret = elementToOperation(nodeP);
	xmlFreeDoc(docP);
	return ret;
}

string Libxml2Builder::serializeOperation(const Operation &operation) const {
	xmlNodePtr nodeP = operationToElement(operation);
	const string ret = XMLUtils::Instance()->serializeLibxml2Node(nodeP, true); 
	xmlFreeNode(nodeP);
	return ret;
}

Operation::ptr_t Libxml2Builder::elementToOperation(const xmlNodePtr nodeP) const throw (WorkflowFormatException) {
	///////////////////
	// 	OperationClass::ptr_t _operationClassP;            -> from child element <operationClass>
	///////////////////
	
	Operation::ptr_t operationP;
    xmlNodePtr curNodeP = nodeP;
    
    curNodeP = nextElementNode(curNodeP);
    if (checkElementName(curNodeP, "operation")) {                             // <operation>
    	operationP = Operation::ptr_t(new Operation());
		// children
		curNodeP = nextElementNode(curNodeP->children);
		if (curNodeP) {
			if (checkElementName(curNodeP, "operationClass")) {                    //   <operationClass>
				OperationClass::ptr_t ocP = elementToOperationClass(curNodeP);
				operationP->setOperationClass(ocP);
			} else {
		    	ostringstream oss;
		    	oss << "Unknown element <" << curNodeP->name << ">. Expected <operationClass>";
		    	LOG_ERROR(_logger, oss.str());
		    	throw WorkflowFormatException(oss.str()); 
			}
		}
    } else {
    	LOG_ERROR(_logger, "Element <operation> not found!");
    	throw WorkflowFormatException("Element <operation> not found!"); 
    }
    
	return operationP;
}

xmlNodePtr Libxml2Builder::operationToElement(const Operation &operation) const {
	// <operation>
	xmlNodePtr nodeP = xmlNewNode(_nsGworkflowdlP, BAD_CAST "operation");

    //   <operationClass>
	if (operation.readOperationClass()) {
		xmlAddChild(nodeP, operationClassToElement(*operation.readOperationClass()));
	}
	
	return nodeP;
}

//////////////////////////
// Edge
//////////////////////////
//<xs:complexType name="placeRef">
//    <xs:annotation>
//        <xs:documentation>
//Reference to a place within the same GWorkflowDL document.
//        </xs:documentation>
//    </xs:annotation>
//    <xs:attribute name="placeID" use="required" type="xs:string">
//        <xs:annotation>
//            <xs:documentation>
//The attribute "placeRef" denotes a place ID.
//            </xs:documentation>
//        </xs:annotation>
//    </xs:attribute>
//    <xs:attribute name="edgeExpression" type="xs:string">
//        <xs:annotation>
//            <xs:documentation>
//The attribute edgeExpression is used as value assignement for input and read places and as functional statement for
//output places using the XPath 1.0 standard.
//            </xs:documentation>
//        </xs:annotation>
//    </xs:attribute>
//</xs:complexType>
//////////////////////////

Edge::ptr_t Libxml2Builder::elementToEdge(Workflow::ptr_t wfP, const xmlNodePtr nodeP) const throw (WorkflowFormatException) {
	Edge::scope_t scope;
	if (checkElementName(nodeP, "readPlace")) {
		scope = Edge::SCOPE_READ;
	} else if (checkElementName(nodeP, "inputPlace")) {
		scope = Edge::SCOPE_INPUT;
	} else if (checkElementName(nodeP, "writePlace")) {
		scope = Edge::SCOPE_WRITE;
	} else if (checkElementName(nodeP, "outputPlace")) {
		scope = Edge::SCOPE_OUTPUT;
	} else {
		ostringstream oss;
		oss << "Invalid name of element \"" << nodeP->name << "\""; 
		LOG_ERROR(_logger, oss.str());
		throw WorkflowFormatException(oss.str()); 
	}

	string placeID((const char*) xmlGetProp(nodeP, BAD_CAST "placeID"));
	xmlChar* xmlCharP = xmlGetProp(nodeP, BAD_CAST "edgeExpression");
	string edgeExpression = xmlCharP ? string((const char*) xmlCharP) : string();
	Place::ptr_t placeP;
	try {
		placeP = wfP->getPlace(placeID);
	} catch (NoSuchWorkflowElement e) {
		ostringstream oss;
		oss << "placeID \"" << placeID << "\" defined as edge within a transition is not available in workflow \"" << wfP->getID() << "\"!";
		LOG_ERROR(_logger, oss.str());
		throw WorkflowFormatException(oss.str()); 
	}
	
	Edge::ptr_t edgeP = Edge::ptr_t(new Edge(scope, placeP, edgeExpression));
	return edgeP;
}

xmlNodePtr Libxml2Builder::edgeToElement(const Edge &edge) const {

	// <readPlace>, <inputPlace>, <writePlace>, <outputPlace>
	const char* elementName;
	switch(edge.getScope()) {
	case(Edge::SCOPE_READ):
		elementName = "readPlace";
	break;
	case(Edge::SCOPE_INPUT):
		elementName = "inputPlace";
	break;
	case(Edge::SCOPE_WRITE):
		elementName = "writePlace";
	break;
	case(Edge::SCOPE_OUTPUT):
		elementName = "outputPlace";
	break;
	}
	xmlNodePtr nodeP = xmlNewNode(_nsGworkflowdlP, BAD_CAST elementName);
	
	// <*Place placeID="">
    xmlNewProp(nodeP, BAD_CAST "placeID", BAD_CAST edge.getPlace()->getID().c_str());
    
    // <*Place edgeExpression="">
    if (edge.getExpression().size() > 0) {
        xmlNewProp(nodeP, BAD_CAST "edgeExpression", BAD_CAST edge.getExpression().c_str());
    }
    
    return nodeP;
}

//////////////////////////
// Transition
//////////////////////////
//<xs:element name="transition" minOccurs="0" maxOccurs="unbounded">
//    <xs:annotation>
//        <xs:documentation>
//The ocurrence of a transition alters the state of the Petri net. A transition may be linked
//to an (external) operation. Petri net arcs are directed edges that are represented by read places, input places,
//write places and output places.
//        </xs:documentation>
//    </xs:annotation>
//    <xs:complexType>
//        <xs:sequence>
//            <xs:element ref="description" minOccurs="0"/>
//            <xs:element ref="property" minOccurs="0" maxOccurs="unbounded"/>
//            <xs:element name="readPlace" type="placeRef" minOccurs="0" maxOccurs="unbounded"/>
//            <xs:element name="inputPlace" type="placeRef" minOccurs="0" maxOccurs="unbounded"/>
//            <xs:element name="writePlace" type="placeRef" minOccurs="0" maxOccurs="unbounded"/>
//            <xs:element name="outputPlace" type="placeRef" minOccurs="0" maxOccurs="unbounded"/>
//            <xs:element name="condition" type="xs:string" minOccurs="0" maxOccurs="unbounded">
//                <xs:annotation>
//                    <xs:documentation>
//The optional element "condition" specifies a pre condition expression that must evaluate to "true" for a transition to
//be enabled. The syntax for conditions is XPath 1.0. The context for the evaluation is the contents of the input and
//read tokens which can be referred to by means of a "$" concatenated with the corresponding edgeExpression (e.g., 
//"$input".
//                    </xs:documentation>
//                </xs:annotation>
//            </xs:element>
//            ===> refer to element operation <===
//        </xs:sequence>
//        <xs:attribute name="ID" use="required" type="xs:ID"/>
//    </xs:complexType>
//</xs:element>
//////////////////////////

Transition::ptr_t Libxml2Builder::deserializeTransition(Workflow::ptr_t wfP, const string &xmlstring) const throw (WorkflowFormatException) {
	xmlDocPtr docP = XMLUtils::Instance()->deserializeLibxml2(xmlstring);
	const xmlNodePtr nodeP = xmlDocGetRootElement(docP);
	const Transition::ptr_t ret = elementToTransition(wfP, nodeP);
	xmlFreeDoc(docP);
	return ret;
}

string Libxml2Builder::serializeTransition(const Transition &transition) const {
	xmlNodePtr nodeP = transitionToElement(transition);
	const string ret = XMLUtils::Instance()->serializeLibxml2Node(nodeP, true); 
	xmlFreeNode(nodeP);
	return ret;
}

Transition::ptr_t Libxml2Builder::elementToTransition(Workflow::ptr_t wfP, const xmlNodePtr nodeP) const throw (WorkflowFormatException) {
	///////////////////
	// std::string _id;                      -> from attribute <transition ID="">
	// TransitionStatus _status;             -> ignored.
    // std::string _description;             -> from child element <description>
    // Properties _properties;       -> from child elements <property>
    // std::vector<Edge::ptr_t> _readEdges;  -> from child elements <readPlace>
    // std::vector<Edge::ptr_t> _inEdges;    -> from child elements <inputPlace>
    // std::vector<Edge::ptr_t> _writeEdges; -> from child elements <writePlace>
    // std::vector<Edge::ptr_t> _outEdges;   -> from child elements <outputPlace>
    // std::vector<std::string> _conditions; -> from child elements <condition>
    // Operation::ptr_t _operationP;         -> from child element <operation>
	///////////////////
	
	Transition::ptr_t transitionP;
    xmlNodePtr curNodeP = nodeP;
    xmlNodePtr textNodeP;
    
    curNodeP = nextElementNode(curNodeP);
    // <transition>
    if (checkElementName(curNodeP, "transition")) {                        
    	// <transition ID="">
		string id((const char*) xmlGetProp(curNodeP, BAD_CAST "ID"));      
		transitionP = Transition::ptr_t(Transition::ptr_t(new Transition(id)));
		// children
		curNodeP = nextElementNode(curNodeP->children);
		// <description>
		if (curNodeP && checkElementName(curNodeP, "description")) {               
			textNodeP = nextTextNode(curNodeP->children);
			if (textNodeP) {
				transitionP->setDescription(string( (const char*) textNodeP->content)); 
			}
			curNodeP = nextElementNode(curNodeP->next);
		}
		// <property>
		// extract all property elements
		Properties props = elementsToProperties(curNodeP);         
		if (!props.empty()) transitionP->setProperties(props);
		while(curNodeP && checkElementName(curNodeP, "property")) {           
			// do nothing, properties have already been extracted before
			curNodeP = nextElementNode(curNodeP->next);
		} 
		// <readPlace>
		while(curNodeP && checkElementName(curNodeP, "readPlace")) {
			Edge::ptr_t edge = elementToEdge(wfP, curNodeP);
			transitionP->addEdge(edge); 
			curNodeP = nextElementNode(curNodeP->next);
		}
		// <inputPlace>
		while(curNodeP && checkElementName(curNodeP, "inputPlace")) {
			Edge::ptr_t edge = elementToEdge(wfP, curNodeP);
			transitionP->addEdge(edge); 
			curNodeP = nextElementNode(curNodeP->next);
		}
		// <writePlace>
		while(curNodeP && checkElementName(curNodeP, "writePlace")) {
			Edge::ptr_t edge = elementToEdge(wfP, curNodeP);
			transitionP->addEdge(edge); 
			curNodeP = nextElementNode(curNodeP->next);
		}
		// <outputPlace>
		while(curNodeP && checkElementName(curNodeP, "outputPlace")) {
			Edge::ptr_t edge = elementToEdge(wfP, curNodeP);
			transitionP->addEdge(edge); 
			curNodeP = nextElementNode(curNodeP->next);
		}
		// <condition>
		while(curNodeP && checkElementName(curNodeP, "condition")) {
			textNodeP = nextTextNode(curNodeP->children);
			if (textNodeP) {
				string condition = string( (const char*) textNodeP->content);
				transitionP->addCondition(condition); 
			}
			curNodeP = nextElementNode(curNodeP->next);
		}
		// <operation>
		if(curNodeP && checkElementName(curNodeP, "operation")) {
			Operation::ptr_t oper = elementToOperation(curNodeP);
			transitionP->setOperation(oper);
			curNodeP = nextElementNode(curNodeP->next);
		}
		
		if (curNodeP) {
			ostringstream oss;
			oss << "Unknown element <" << curNodeP->name << "> as child of <transition ID=\"" << id << "\">";
			LOG_ERROR(_logger, oss.str());
			throw WorkflowFormatException(oss.str()); 
		}
    } else {
    	LOG_ERROR(_logger, "Element <transition> not found!");
    	throw WorkflowFormatException("Element <transition> not found!"); 
    }
    
	return transitionP;
}

xmlNodePtr Libxml2Builder::transitionToElement(const Transition &transition) const {
	// <transition>
	xmlNodePtr nodeP = xmlNewNode(_nsGworkflowdlP, BAD_CAST "transition");
	xmlNodePtr curNodeP;
	
	// <transition ID="">
    xmlNewProp(nodeP, BAD_CAST "ID", BAD_CAST transition.getID().c_str());

    //   <description>
    if (transition.getDescription().size()>0) {
		xmlNewChild(nodeP,_nsGworkflowdlP, BAD_CAST "description",BAD_CAST transition.getDescription().c_str());
    }
	
    //   <property>
	Properties props = transition.readProperties();
	if (!props.empty()) {
		curNodeP = propertiesToElements(props);
		if (curNodeP) {
			xmlAddChildList(nodeP, curNodeP); 
		}
	}

	// <readPlace>
	std::vector<Edge::ptr_t> edges = transition.getReadEdges();
	for (size_t i = 0; i < edges.size(); i++) {
		xmlAddChild(nodeP, edgeToElement(*edges[i]));
	}

	// <inputPlace>
	edges = transition.getInEdges();
	for (size_t i = 0; i < edges.size(); i++) {
		xmlAddChild(nodeP, edgeToElement(*edges[i]));
	}

	// <writePlace>
	edges = transition.getWriteEdges();
	for (size_t i = 0; i < edges.size(); i++) {
		xmlAddChild(nodeP, edgeToElement(*edges[i]));
	}

	// <outputPlace>
	edges = transition.getOutEdges();
	for (size_t i = 0; i < edges.size(); i++) {
		xmlAddChild(nodeP, edgeToElement(*edges[i]));
	}
	
	// <condition>
	std::vector<std::string> conditions = transition.getConditions();
	for (size_t i = 0; i < conditions.size(); i++) {
		xmlNewChild(nodeP,_nsGworkflowdlP, BAD_CAST "condition",BAD_CAST conditions[i].c_str());		
	}

	// <operation>
	if (transition.readOperation()) {
		xmlAddChild(nodeP, operationToElement(*transition.readOperation()));
	}
	
	return nodeP;
}

//////////////////////////
// Workflow
//////////////////////////
//<xs:element name="workflow">
//    <xs:annotation>
//        <xs:documentation>
//Representation of a workflow by means of a High-level Petri net, mainly composed of places and transitions. The
//"workflow" element is the root element of each GWorkflowDL document. The arcs are represented as child elements of
//transitions, the tokens are child elements of places.
//        </xs:documentation>
//    </xs:annotation>
//    <xs:complexType>
//        <xs:sequence>
//            <xs:element ref="owl" minOccurs="0" maxOccurs="unbounded"/>
//            <xs:element ref="description" minOccurs="0"/>
//            ===> refer to <property> <===
//            ===> refer to <place> <===
//            ===> refer to <transition> <===
//        </xs:sequence>
//        <xs:attribute name="ID" use="required" type="xs:ID"/>
//    </xs:complexType>
//</xs:element>
//////////////////////////

Workflow::ptr_t Libxml2Builder::deserializeWorkflow(const string &xmlstring) const throw (WorkflowFormatException) {
	xmlDocPtr docP = XMLUtils::Instance()->deserializeLibxml2(xmlstring);
	const xmlNodePtr nodeP = xmlDocGetRootElement(docP);
	const Workflow::ptr_t ret = elementToWorkflow(nodeP);
	xmlFreeDoc(docP);
	return ret;
}

/**
 * Construct workflow from file.
 * @param filename The filename of the GWorkflowDL file including its path. 
 */
Workflow::ptr_t Libxml2Builder::deserializeWorkflowFromFile(const std::string& filename) const throw (WorkflowFormatException) {
	xmlDocPtr docP = XMLUtils::Instance()->deserializeFileLibxml2(filename);
	const xmlNodePtr nodeP = xmlDocGetRootElement(docP);
	const Workflow::ptr_t ret = elementToWorkflow(nodeP);
	xmlFreeDoc(docP);
	return ret;
}

string Libxml2Builder::serializeWorkflow(const Workflow &workflow) const {
	xmlNodePtr nodeP = workflowToElement(workflow);
	const string ret = XMLUtils::Instance()->serializeLibxml2Node(nodeP, true); 
	xmlFreeNode(nodeP);
	return ret;
}

void Libxml2Builder::serializeWorkflowToFile(const Workflow &workflow, const string& filename) const {
	// write file
	ofstream file(filename.c_str());
	if (file.is_open()) {
		file << serializeWorkflow(workflow);
		file.close();
	} else {
		LOG_ERROR(logger_t(getLogger("gwdl")), "Unable to open file " << filename << ": " << strerror(errno));
	}
}

Workflow::ptr_t Libxml2Builder::elementToWorkflow(const xmlNodePtr nodeP) const throw (WorkflowFormatException) {
	///////////////////
	// std::string _id;                                     -> from attribute <workflow ID="">
	// std::string _description;                            -> from child element <description>
	// Properties _properties;                      -> from child elements <property>
	// place_map_t _places;         -> from child elements <place>
	// transition_list_t _transitions;         -> from child elements <transition>
	// transition_list_t _enabledTransitions;  -> calculated on demand.
	///////////////////
	
	Workflow::ptr_t workflowP;
    xmlNodePtr curNodeP = nodeP;
    xmlNodePtr textNodeP;
    
    curNodeP = nextElementNode(curNodeP);
    
    // <workflow>
    if (checkElementName(curNodeP, "workflow")) {
    	// <workflow ID="">
		string id((const char*) xmlGetProp(curNodeP, BAD_CAST "ID"));      
		workflowP = Workflow::ptr_t(new Workflow(id));
		// children
		curNodeP = nextElementNode(curNodeP->children);
		// <description>
		if (curNodeP && checkElementName(curNodeP, "description")) {               
			textNodeP = nextTextNode(curNodeP->children);
			if (textNodeP) {
				workflowP->setDescription(string( (const char*) textNodeP->content)); 
			}
			curNodeP = nextElementNode(curNodeP->next);
		}
		// <property>
		// extract all property elements
		Properties props = elementsToProperties(curNodeP);         
		if (!props.empty()) workflowP->setProperties(props);
		while(curNodeP && checkElementName(curNodeP, "property")) {           
			// do nothing, properties have already been extracted before
			curNodeP = nextElementNode(curNodeP->next);
		} 
		
		// <place>
		while(curNodeP && checkElementName(curNodeP, "place")) {
			Place::ptr_t place = elementToPlace(curNodeP); 
			workflowP->addPlace(place); 
			curNodeP = nextElementNode(curNodeP->next);
		}
		
		// <transition>
		while(curNodeP && checkElementName(curNodeP, "transition")) {
			Transition::ptr_t trans = elementToTransition(workflowP,curNodeP); 
			workflowP->addTransition(trans); 
			curNodeP = nextElementNode(curNodeP->next);
		}
		
		if (curNodeP) {
			ostringstream oss;
			oss << "Unknown element <" << curNodeP->name << "> as child of <workflow ID=\"" << id << "\">";
			LOG_ERROR(_logger, oss.str());
			throw WorkflowFormatException(oss.str()); 
		}
    } else {
    	LOG_ERROR(_logger, "Element <workflow> not found!");
    	throw WorkflowFormatException("Element <workflow> not found!"); 
    }
    
	return workflowP;
}

xmlNodePtr Libxml2Builder::workflowToElement(const Workflow &workflow) const {
	xmlNodePtr nodeP = xmlNewNode(_nsGworkflowdlP, BAD_CAST "workflow");
	xmlNodePtr curNodeP;
	
	// <workflow ID="">
    xmlNewProp(nodeP, BAD_CAST "ID", BAD_CAST workflow.getID().c_str());
    
    //   <description>
    if (workflow.getDescription().size()>0) {
		xmlNewChild(nodeP,_nsGworkflowdlP, BAD_CAST "description",BAD_CAST workflow.getDescription().c_str());
    }

    //   <property>
	Properties props = workflow.readProperties();
	if (!props.empty()) {
		curNodeP = propertiesToElements(props);
		if (curNodeP) {
			xmlAddChildList(nodeP, curNodeP); 
		}
	}
	
	//  <place>
	Workflow::place_map_t places = workflow.readPlaces();
	for(Workflow::place_map_t::iterator it=places.begin(); it!=places.end(); ++it) {
		xmlAddChild(nodeP, placeToElement(*(it->second)));
	}
	
	// <transition>
	Workflow::transition_list_t transitions = workflow.readTransitions();
	for (size_t i = 0; i < transitions.size(); i++) {
		xmlAddChild(nodeP, transitionToElement(*transitions[i]));
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

// OperationCandidate
ostream& operator<<(ostream &out, gwdl::OperationCandidate &ocand) {
	gwdl::Libxml2Builder builder;
	out << builder.serializeOperationCandidate(ocand);
	return out;
}

// OperationClass
ostream& operator<<(ostream &out, gwdl::OperationClass &oclass) {
	gwdl::Libxml2Builder builder;
	out << builder.serializeOperationClass(oclass);
	return out;
}

// Operation
ostream& operator<<(ostream &out, gwdl::Operation &oper) {
	gwdl::Libxml2Builder builder;
	out << builder.serializeOperation(oper);
	return out;
}

// Transition
ostream& operator<<(ostream &out, gwdl::Transition &transition) {
	gwdl::Libxml2Builder builder;
	out << builder.serializeTransition(transition);
	return out;
}

// Workflow
ostream& operator<<(ostream &out, gwdl::Workflow &workflow) {
	gwdl::Libxml2Builder builder;
	out << builder.serializeWorkflow(workflow);
	return out;
}
