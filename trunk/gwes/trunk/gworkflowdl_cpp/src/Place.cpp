/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
//gwdl
#include <gwdl/Place.h>
#include <gwdl/Defines.h>
#include <gwdl/XMLUtils.h>
//fhglog
#include <fhglog/fhglog.hpp>
//xerces-c
#include <xercesc/util/OutOfMemoryException.hpp>

using namespace fhg::log;
XERCES_CPP_NAMESPACE_USE
using namespace std;

#define X(str) XMLString::transcode((const char*)& str)
#define XS(strg) XMLString::transcode((const char*) strg.c_str())
#define S(str) XMLString::transcode(str)

namespace gwdl
{

Place::Place(const string& _id) : capacity(INT_MAX)
{
	if (_id == "") id=generateID();
	else id = _id;
	capacity = Place_DEFAULT_CAPACITY;
	nextUnlockedToken = NULL;
}


Place::Place(DOMElement* element) throw(CapacityException)
{
	//XMLCh* ns = X(SCHEMA_wfSpace);

	// ID
	id = S(element->getAttribute(X("ID")));

	// capacity
	const XMLCh* res = element->getAttribute(X("capacity"));
	capacity = XMLString::stringLen(res) != 0 ? atoi(S(res)) : Place_DEFAULT_CAPACITY;

	// loop child nodes
	DOMNodeList* le = element->getChildNodes();
	if (le->getLength() >0) {
		// properties
		properties = Properties(le);
		// other nodes
		for (XMLSize_t i = 0; i<le->getLength(); i++) {
			DOMNode* node = le->item(i);
			const XMLCh* name = node->getNodeName(); 
			if (XMLString::equals(name,X("description"))) {
				description = string(XMLString::transcode(node->getTextContent()));
			} else if (XMLString::equals(name,X("tokenClass"))) {
				tokenType = S(((DOMElement*) node)->getAttribute(X("type")));
			} else if (XMLString::equals(name,X("token"))) {
				if (tokens.size() >= capacity) {
					throw CapacityException("Trying to put too many tokens on place"); 
				}
				tokens.push_back(new Token((DOMElement*) node));
			}
		}
	}
	
	nextUnlockedToken = ( getTokenNumber() > 0 ) ? tokens[0] : NULL;
}

Place::~Place()
{
	removeAllTokens();
}

DOMElement* Place::toElement(DOMDocument* doc)
{
	DOMElement* el = NULL;
	// Initialize the XML4C2 system.
	XMLUtils::Instance();

	XMLCh* ns = X(SCHEMA_wfSpace);

	try
	{   
		el = doc->createElementNS(ns, X("place"));
		el->setAttribute(X("ID"), XS(id));
		if (capacity!=Place_DEFAULT_CAPACITY)
		{
			ostringstream oss;
			oss << capacity; 
			el->setAttribute(X("capacity"), XS(string(oss.str())));
		}

		// description
		if (description.size()>0)
		{
			DOMElement* el1 = doc->createElementNS(ns, X("description"));
			el1->setTextContent(XS(description));
			el->appendChild(el1);
		}

		// properties
		vector<DOMElement*> v = properties.toElements(doc);       
		for (unsigned int i = 0; i < v.size(); i++)
		{
			el->appendChild(v[i]);
		}

		// tokenClass
		if (tokenType.size()>0)
		{       
			DOMElement* el2 = doc->createElementNS(ns, X("tokenClass"));
			el2->setAttribute(X("type"), XS(tokenType));
			el->appendChild(el2);
		}

		for (unsigned int i = 0; i < tokens.size(); i++)
		{
			el->appendChild(tokens[i]->toElement(doc));
		}                   
	}
	catch (const OutOfMemoryException&)
	{
		LOG_FATAL(logger_t(getLogger("gwdl")), "OutOfMemoryException" );
	}
	catch (const DOMException& e)
	{
		LOG_ERROR(logger_t(getLogger("gwdl")), "DOMException code is:  " << e.code );
		LOG_ERROR(logger_t(getLogger("gwdl")), "Message: " << S(e.msg) );
	}
	catch (...)
	{
		LOG_ERROR(logger_t(getLogger("gwdl")), "An error occurred creating the document" );
	}

	return el;
}

const string& Place::getID() const
{
	return id;
}

void Place::setTokenType(const string& _type)
{
	tokenType = _type;
}

const string& Place::getTokenType() const
{
	return tokenType;	
}

bool Place::isEmpty() const
{
	return (tokens.empty());
}

void Place::addToken(Token* token) throw(CapacityException) 
{
	if (tokens.size() >= capacity) {
		throw CapacityException("Trying to put too many tokens on place"); 
	}
	tokens.push_back(token);
	nextUnlockedToken = NULL;
}

void Place::removeToken(int i)
{
	delete tokens[i];
	tokens.erase(tokens.begin()+i);
	nextUnlockedToken = NULL;
}

const vector<Token*>& Place::getTokens() const 
{
	return tokens;	
}

void Place::removeToken(Token* _token) 
{
	for(vector<Token*>::iterator it=tokens.begin(); it != tokens.end(); ++it)
	{
		if ((*it)->getID() == _token->getID()) {
			delete *it; *it = NULL;
			tokens.erase(it);
			nextUnlockedToken = NULL;
			break;	
		}	
	}	
}

void Place::removeAllTokens() {
	for(vector<Token*>::iterator it=tokens.begin(); it != tokens.end(); ++it) delete *it;
	tokens.clear();	
	nextUnlockedToken = NULL;
}

void Place::setCapacity(unsigned int _capacity) throw(CapacityException) 
{
	if (_capacity < tokens.size()) {
		throw CapacityException("There are too many tokens on the place for setting capacity");
	}
	capacity = _capacity;
}

int Place::getCapacity() const
{
	return capacity;
}

int Place::getTokenNumber() const
{
	return tokens.size();
}

void Place::setDescription(const string& d) 
{
	description = d;
}

const string& Place::getDescription() const 
{
	return description;	
}

void Place::setProperties(Properties& _props) 
{
	properties = _props;
}

Properties& Place::getProperties() 
{
	return properties;	
}

void Place::lockToken(Token* p_token, Transition* p_transition) {
	nextUnlockedToken = NULL;
	p_token->lock(p_transition);
}

void Place::unlockToken(Token* p_token) {
	nextUnlockedToken = NULL;
	p_token->unlock();
}

Token* Place::getNextUnlockedToken() {
	if ( nextUnlockedToken == NULL) {
		for (size_t i=0; i<tokens.size(); i++) {
			if (!tokens[i]->isLocked()) {
				nextUnlockedToken = tokens[i];
				break;
			}
		}
	}
	return nextUnlockedToken;
}

string Place::generateID() const
{
	static long counter = 0; 
	ostringstream oss; 
	oss << "p" << counter++;
	return oss.str();
}

} // end namespace gwdl

ostream& operator<<(ostream &out, gwdl::Place &place) 
{	
	DOMNode* node = place.toElement(gwdl::XMLUtils::Instance()->createEmptyDocument(true));
	gwdl::XMLUtils::Instance()->serialize(out,node,true);
	return out;
}

