/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
//std
#include <iostream>
#include <sstream>
#include <fstream>
#include <errno.h>
#include <xercesc/util/OutOfMemoryException.hpp>
//gwdl
#include <gwdl/Workflow.h>
#include <gwdl/XMLUtils.h>

#define X(str) XMLString::transcode((const char*)& str)
#define XS(strg) XMLString::transcode((const char*) strg.c_str())
#define S(str) XMLString::transcode(str)

namespace gwdl
{

Workflow::Workflow()
{
	id = WORKFLOW_DEFAULT_ID;
	description = WORKFLOW_DEFAULT_DESCRIPTION;
}

Workflow::~Workflow()
{
	//cout << "~Workflow()=" << this << endl;
	for(vector<Transition*>::iterator it=transitions.begin(); it!=transitions.end(); ++it) delete *it;
	transitions.clear();
	enabledTransitions.clear();
	for(map<string,Place*>::iterator it=places.begin(); it!=places.end(); ++it) delete it->second;
	places.clear();
	placeList.clear();
}

Workflow::Workflow(DOMElement* element)
{
  // ID
  id = string(S(element->getAttribute(X("ID"))));

  //XMLCh* ns = X(SCHEMA_wfSpace);
  
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
		  } else if (XMLString::equals(name,X("place"))) {
			  addPlace(new Place((DOMElement*) node));
		  } else if (XMLString::equals(name,X("transition"))) {
	          addTransition(new Transition(this, (DOMElement*) node));
		  }
				 
	  }
  }
}

Workflow::Workflow(string filename) 
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
    	cerr << "Unable to open file " << filename << ": " << strerror(errno) << endl;
    }
    
    // convert string to DOMElement
    string gworkflowdl = workflowS.str();
    DOMElement* element = (XMLUtils::Instance()->deserialize(gworkflowdl))->getDocumentElement();

    // create new workflow object
    // ID
    id = string(S(element->getAttribute(X("ID"))));

    //XMLCh* ns = X(SCHEMA_wfSpace);
    
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
  		  } else if (XMLString::equals(name,X("place"))) {
  			  addPlace(new Place((DOMElement*) node));
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
        wfe->setAttribute(X("ID"), XS(id));

        // description
        if (description.size()>0)
        {
        	DOMElement* el1 = doc->createElementNS(ns, X("description"));
            el1->setTextContent(XS(description));
            wfe->appendChild(el1);
        }

        // properties
        vector<DOMElement*> v = properties.toElements(doc);       
        for (unsigned int i = 0; i < v.size(); i++)
        {
        	wfe->appendChild(v[i]);
        }

        // places
		for (map<string,Place*>::iterator it=places.begin(); it!=places.end(); ++it)
        {
 			wfe->appendChild(it->second->toElement(doc));
        }
        
        // transitions
        for (unsigned int i = 0; i < transitions.size(); i++)
        {
        	wfe->appendChild(transitions[i]->toElement(doc)); 
        }             
    }
    catch (const OutOfMemoryException&)
    {
    	XERCES_STD_QUALIFIER cerr << "OutOfMemoryException during Workflow.toElement()." << XERCES_STD_QUALIFIER endl;
    }
    catch (const DOMException& e)
    {
    	XERCES_STD_QUALIFIER cerr << "DOMException during Workflow.toElement(). code is:  " << e.code << XERCES_STD_QUALIFIER endl;
		XERCES_STD_QUALIFIER cerr << "Message: " << S(e.msg) << XERCES_STD_QUALIFIER endl;
    }
    catch (...)
    {
    	XERCES_STD_QUALIFIER cerr << "An error occurred creating the document during Workflow.toElement()." << XERCES_STD_QUALIFIER endl;
    }

    return doc;
}

void Workflow::saveToFile(string filename) {
	// write file
    ofstream file(filename.c_str());
    if (file.is_open()) {
    	file << *this;
    	file.close();
    } else {
    	cerr << "Unable to open file " << filename << ": " << strerror(errno) << endl;
    }
}

Place* Workflow::getPlace(unsigned int i) throw (NoSuchWorkflowElement)
{
      unsigned int j=0;
      for(map<string,Place*>::iterator it=places.begin(); it!=places.end(); ++it)
      {
        if(j++ == i) return (it->second);
      }
     // no such workflow element
     ostringstream message; 
	 message << "Place with index=\"" << i << "\" is not available!";
     throw NoSuchWorkflowElement(message.str());
}

Place* Workflow::getPlace(string id) throw (NoSuchWorkflowElement)
{	
	map<string,Place*>::iterator iter = places.find(id);
	if (iter!=places.end()) return (iter->second);
    // no such workflow element
    ostringstream message; 
	message << "Place with id=\"" << id << "\" is not available!";
    throw NoSuchWorkflowElement(message.str());
}

unsigned int Workflow::getPlaceIndex(string id) throw (NoSuchWorkflowElement)
{
      int j=0;
      for(map<string,Place*>::iterator it=places.begin(); it!=places.end(); ++it)
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
    for(map<string,Place*>::iterator it=places.begin(); it!=places.end(); ++it)
    {
    	if(j++ == i) {
    		delete it->second; it->second = NULL;
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

Transition* Workflow::getTransition(string id) throw (NoSuchWorkflowElement)
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

unsigned int Workflow::getTransitionIndex(string id) throw (NoSuchWorkflowElement)
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

void Workflow::setTransitions(vector<Transition*> _transitions)
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

