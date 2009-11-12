/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
//gwdl
#include <gwdl/Workflow.h>
#include <gwdl/XMLUtils.h>
//fhglog
#include <fhglog/fhglog.hpp>
// xerces-c
#include <xercesc/util/OutOfMemoryException.hpp>
//std
#include <fstream>
#include <errno.h>

#include <gwdl/XMLTranscode.hpp>

using namespace fhg::log;
XERCES_CPP_NAMESPACE_USE
using namespace std;

namespace gwdl
{

Workflow::Workflow()
{
	_id = WORKFLOW_DEFAULT_ID;
	description = WORKFLOW_DEFAULT_DESCRIPTION;
}

Workflow::~Workflow()
{
	for(vector<Transition*>::iterator it=transitions.begin(); it!=transitions.end(); ++it) delete *it;
	transitions.clear();
	enabledTransitions.clear();
	for(map<string,Place::ptr_t>::iterator it=places.begin(); it!=places.end(); ++it) (it->second).reset();
	places.clear();
	placeList.clear();
}

Workflow::Workflow(DOMElement* element)
{
	// default values
	description = WORKFLOW_DEFAULT_DESCRIPTION;

	// ID
	_id = string(S(element->getAttribute(X("ID"))));
	
	//XMLCh* ns = X(SCHEMA_wfSpace);

	DOMNodeList* le = element->getChildNodes();
	if (le->getLength() >0) {
		// properties
		///ToDo: Migration to libxml2.
//		properties = Properties(le);
		// other nodes
		for (XMLSize_t i = 0; i<le->getLength(); i++) {
			DOMNode* node = le->item(i);
			const XMLCh* name = node->getNodeName(); 
			if (XMLString::equals(name,X("description"))) {
                          char * cntx (XMLString::transcode(node->getTextContent()));
				description = string(cntx);
                                XMLString::release(&cntx);
			} else if (XMLString::equals(name,X("place"))) {
				LOG_WARN(logger_t(getLogger("gwdl")), "///ToDo: migration to libxml2///");
//				addPlace(new Place((DOMElement*) node));
			} else if (XMLString::equals(name,X("transition"))) {
				addTransition(new Transition(this, (DOMElement*) node));
			}
		}
	}
}

Workflow::Workflow(const string& filename) throw (WorkflowFormatException) 
{
	// read file
	ifstream file(filename.c_str());
	ostringstream workflowS;
	if (file.is_open()) {
		char c;
		while (file.get(c)) {
			workflowS << c;
		}
		file.close();
	} else {
		ostringstream message; 
		message << "Unable to open file " << filename << ": " << strerror(errno);
		LOG_ERROR(logger_t(getLogger("gwdl")), message);
		throw WorkflowFormatException(message.str());
	}

	// convert string to DOMElement
	string gworkflowdl = workflowS.str();
	DOMElement* element = (XMLUtils::Instance()->deserialize(gworkflowdl))->getDocumentElement();

	// create new workflow object
	// ID
	_id = string(S(element->getAttribute(X("ID"))));

	//XMLCh* ns = X(SCHEMA_wfSpace);

	DOMNodeList* le = element->getChildNodes();
	if (le->getLength() >0) {
		// properties
		///ToDo: Migration to libxml2.
//		properties = Properties(le);
		// other nodes
		for (XMLSize_t i = 0; i<le->getLength(); i++) {
			DOMNode* node = le->item(i);
			const XMLCh* name = node->getNodeName(); 
			if (XMLString::equals(name,X("description"))) {
                          char * cntx (XMLString::transcode(node->getTextContent()));
				description = string(cntx);
                                XMLString::release(&cntx);
			} else if (XMLString::equals(name,X("place"))) {
				LOG_WARN(logger_t(getLogger("gwdl")), "///ToDo: migration to libxml2///");
//				addPlace(new Place((DOMElement*) node));
			} else if (XMLString::equals(name,X("transition"))) {
				addTransition(new Transition(this, (DOMElement*) node));
			}
		}
	}
}

DOMDocument* Workflow::toDocument() 
{
	DOMDocument* doc = NULL;
	// Initialize the XML4C2 system.
	XMLUtils::Instance();
	XMLCh* ns = X(SCHEMA_wfSpace);

	try
	{
		doc = XMLUtils::Instance()->createEmptyDocument(true);
		DOMElement* wfe = doc->getDocumentElement();

		wfe->setAttribute(X("xmlns:xsd"), X(SCHEMA_xsd));
		//wfe->setAttribute(X("xmlns:xsi"), X(SCHEMA_xsi)); wird schon durchs folgende Kommando gesetzt 
		wfe->setAttributeNS(X(SCHEMA_xsi), X("xsi:schemaLocation"), X(SCHEMA_location));

		//identity
		wfe->setAttribute(X("ID"), XS(_id));

		// description
		if (description.size()>0)
		{
			DOMElement* el1 = doc->createElementNS(ns, X("description"));
			el1->setTextContent(XS(description));
			wfe->appendChild(el1);
		}

		// properties
		///ToDo: Migration to libxml2.
//		vector<DOMElement*> v = properties.toElements(doc);       
//		for (unsigned int i = 0; i < v.size(); i++)
//		{
//			wfe->appendChild(v[i]);
//		}

		// places
		for (map<string,Place::ptr_t>::iterator it=places.begin(); it!=places.end(); ++it)
		{
			LOG_WARN(logger_t(getLogger("gwdl")), "///ToDo: migration to libxml2///");
//			wfe->appendChild(it->second->toElement(doc));
		}

		// transitions
		for (unsigned int i = 0; i < transitions.size(); i++)
		{
			wfe->appendChild(transitions[i]->toElement(doc)); 
		}             
	}
	catch (const OutOfMemoryException&)
	{
		LOG_FATAL(logger_t(getLogger("gwdl")), "OutOfMemoryException during Workflow.toElement()." );
	}
	catch (const DOMException& e)
	{
		LOG_ERROR(logger_t(getLogger("gwdl")), "DOMException during Workflow.toElement(). code is:  " << e.code );
		LOG_ERROR(logger_t(getLogger("gwdl")), "Message: " << S(e.msg) );
	}
	catch (...)
	{
		LOG_ERROR(logger_t(getLogger("gwdl")), "An error occurred creating the document during Workflow.toElement()." );
	}

	return doc;
}

void Workflow::saveToFile(const string& filename) {
	// write file
	ofstream file(filename.c_str());
	if (file.is_open()) {
		file << *this;
		file.close();
	} else {
		LOG_ERROR(logger_t(getLogger("gwdl")), "Unable to open file " << filename << ": " << strerror(errno));
	}
}

Place::ptr_t Workflow::getPlace(unsigned int i) throw (NoSuchWorkflowElement)
{
	unsigned int j=0;
	for(map<string,Place::ptr_t>::iterator it=places.begin(); it!=places.end(); ++it)
	{
		if(j++ == i) return (it->second);
	}
	// no such workflow element
	ostringstream message; 
	message << "Place with index=\"" << i << "\" is not available!";
	throw NoSuchWorkflowElement(message.str());
}

Place::ptr_t Workflow::getPlace(const string& id) throw (NoSuchWorkflowElement)
{	
	map<string,Place::ptr_t>::iterator iter = places.find(id);
	if (iter!=places.end()) return (iter->second);
	// no such workflow element
	ostringstream message; 
	message << "Place with id=\"" << id << "\" is not available!";
	throw NoSuchWorkflowElement(message.str());
}

unsigned int Workflow::getPlaceIndex(const string& id) throw (NoSuchWorkflowElement)
{
	int j=0;
	for(map<string,Place::ptr_t>::iterator it=places.begin(); it!=places.end(); ++it)
	{
		if(it->first == id) return j;
		++j;
	}
	// no such workflow element
	ostringstream message; 
	message << "Place with id=\"" << id << "\" is not available!";
	throw NoSuchWorkflowElement(message.str());
}

void Workflow::removePlace(unsigned int i) throw (NoSuchWorkflowElement)
{
	if (i>=places.size()) 
	{
		// no such workflow element
		ostringstream message; 
		message << "Place with index=\"" << i << "\" is not available!";
		throw NoSuchWorkflowElement(message.str());
	}
	unsigned int j=0;
	for(map<string,Place::ptr_t>::iterator it=places.begin(); it!=places.end(); ++it)
	{
		if(j++ == i) {
			(it->second).reset();
			places.erase(it);
			break;
		}
	}
}

void Workflow::removeTransition(unsigned int i) throw (NoSuchWorkflowElement)
{	
	if (i>=transitions.size())
	{
		// no such workflow element
		ostringstream message; 
		message << "Transition with index=\"" << i << "\" is not available!";
		throw NoSuchWorkflowElement(message.str());
	}
	Transition* tP = *(transitions.begin()+i);
	delete tP;
	tP=NULL;
	transitions.erase(transitions.begin()+i);
}

Transition* Workflow::getTransition(const string& id) throw (NoSuchWorkflowElement)
{
	for(unsigned int i=0; i<transitions.size(); ++i)
	{
		if(transitions[i]->getID() == id) return transitions[i];
	}
	// no such workflow element
	ostringstream message; 
	message << "Transition with id=\"" << id << "\" is not available!";
	throw NoSuchWorkflowElement(message.str());
}

unsigned int Workflow::getTransitionIndex(const string& id) const throw (NoSuchWorkflowElement)
{
	for(unsigned int i=0; i<transitions.size(); ++i)
	{
		if(transitions[i]->getID() == id) return i;
	}
	// no such workflow element
	ostringstream message; 
	message << "Transition with id=\"" << id << "\" is not available!";
	throw NoSuchWorkflowElement(message.str());
}

Transition* Workflow::getTransition(unsigned int i) throw (NoSuchWorkflowElement)
{	
	if (i>=transitions.size())
	{
		// no such workflow element
		ostringstream message; 
		message << "Transition with index=\"" << i << "\" is not available!";
		throw NoSuchWorkflowElement(message.str());
	}
	return transitions[i];
}

void Workflow::setTransitions(vector<Transition*>& _transitions)
{
	for(vector<Transition*>::iterator it=transitions.begin(); it!=transitions.end(); ++it) delete *it;
	transitions.clear();
	transitions.insert(transitions.end(), _transitions.begin(), _transitions.end());
}


} // end namespace gwdl

ostream& operator<<(ostream &out, gwdl::Workflow &wf) 
{	
	DOMDocument* doc = wf.toDocument();
	gwdl::XMLUtils::Instance()->serialize(out,doc,true);
	return out;
}

