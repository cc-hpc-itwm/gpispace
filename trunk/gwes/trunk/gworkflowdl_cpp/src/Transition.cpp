/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
// xerces-c
#include <xercesc/util/OutOfMemoryException.hpp>
// gwdl
#include <gwdl/Transition.h>
#include <gwdl/Defines.h>
#include <gwdl/Workflow.h>
#include <gwdl/XMLUtils.h>
// std
#include <iostream>
#include <sstream>

XERCES_CPP_NAMESPACE_USE
using namespace std;

#define X(str) XMLString::transcode((const char*)& str)
#define XS(strg) XMLString::transcode((const char*) strg.c_str())
#define S(str) XMLString::transcode(str)

namespace gwdl
{

Transition::Transition(const string& _id)
{
	if (_id == "") id=generateID();
	else id = _id;
	description = "";
	operationP = NULL; 
	status = Transition::STATUS_NONE;
}

Transition::~Transition()
{
	if(operationP != NULL)
	{
		delete operationP;
		operationP = NULL;
	}
	for(ITR_Edges it=readEdges.begin(); it!=readEdges.end(); ++it) delete *it;
	readEdges.clear();
	for(ITR_Edges it=inEdges.begin(); it!=inEdges.end(); ++it) delete *it;
	inEdges.clear();
	for(ITR_Edges it=writeEdges.begin(); it!=writeEdges.end(); ++it) delete *it;
	writeEdges.clear();
	for(ITR_Edges it=outEdges.begin(); it!=outEdges.end(); ++it) delete *it;
	outEdges.clear();
}

Transition::Transition(Workflow* wf, DOMElement* element)
{
	// default values
	description = "";
	operationP = NULL; 
	status = Transition::STATUS_NONE;

	//XMLCh* ns = X(SCHEMA_wfSpace);

	// ID
	id = S(element->getAttribute(X("ID")));

	// loop child nodes
	DOMNodeList* le = element->getChildNodes();
	if (le->getLength() >0) {
		// properties
		properties = Properties(le);
		// other nodes
		for (XMLSize_t i = 0; i<le->getLength(); i++) {
			DOMNode* node = le->item(i);
			const XMLCh* name = node->getNodeName();
			// description
			if (XMLString::equals(name,X("description"))) {
				description = string(XMLString::transcode(node->getTextContent()));
			} 
			// condition
			else if (XMLString::equals(name,X("condition"))) {
				conditions.push_back(string(S((node)->getTextContent())));
			} 
			// readPlace
			else if (XMLString::equals(name,X("readPlace"))) {
				DOMElement* el = (DOMElement*)node;
				addReadEdge(
						new Edge(wf->getPlace(string(S(el->getAttribute(X("placeID"))))),
								string(S(el->getAttribute(X("edgeExpression")))))
				); 
			}
			// inputPlace
			else if (XMLString::equals(name,X("inputPlace"))) {
				DOMElement* el = (DOMElement*)node;
				addInEdge(
						new Edge(wf->getPlace(string(S(el->getAttribute(X("placeID"))))),
								string(S(el->getAttribute(X("edgeExpression")))))
				); 
			}
			// writePlace
			else if (XMLString::equals(name,X("writePlace"))) {
				DOMElement* el = (DOMElement*)node;
				addWriteEdge(
						new Edge(wf->getPlace(string(S(el->getAttribute(X("placeID"))))),
								string(S(el->getAttribute(X("edgeExpression")))))
				); 
			}
			// outputPlace
			else if (XMLString::equals(name,X("outputPlace"))) {
				DOMElement* el = (DOMElement*)node;
				addOutEdge(
						new Edge(wf->getPlace(string(S(el->getAttribute(X("placeID"))))),
								string(S(el->getAttribute(X("edgeExpression")))))
				); 
			}
			// operation
			else if (XMLString::equals(name,X("operation"))) {
				operationP = new Operation((DOMElement*)node);
			}
		}
	}
}

DOMElement* Transition::toElement(DOMDocument* doc) 
{
	DOMElement* el = NULL;
	// Initialize the XML4C2 system.
	XMLUtils::Instance();

	XMLCh* ns = X(SCHEMA_wfSpace);

	try
	{
		el = doc->createElementNS(ns, X("transition"));
		el->setAttribute(X("ID"), XS(id));
		DOMElement* el1;               

		// description
		if (description.size()>0)
		{
			el1 = doc->createElementNS(ns, X("description"));
			el1->setTextContent(XS(description));
			el->appendChild(el1);
		}

		// properties
		vector<DOMElement*> v = properties.toElements(doc);       
		for (unsigned int i = 0; i < v.size(); i++)
		{
			el->appendChild(v[i]);
		}

		// read edges
		for(ITR_Edges it=readEdges.begin(); it!=readEdges.end(); ++it)
		{
			el1 = doc->createElementNS(ns, X("readPlace"));
			el1->setAttribute(X("placeID"), XS((*it)->getPlace()->getID()));
			if((*it)->getExpression().size() > 0) el1->setAttribute(X("edgeExpression"), XS((*it)->getExpression()));
			el->appendChild(el1);       	      
		}
		//  in edges
		for(ITR_Edges it=inEdges.begin(); it!=inEdges.end(); ++it)
		{
			el1 = doc->createElementNS(ns, X("inputPlace"));
			el1->setAttribute(X("placeID"), XS((*it)->getPlace()->getID()));
			if((*it)->getExpression().size() > 0) el1->setAttribute(X("edgeExpression"), XS((*it)->getExpression()));
			el->appendChild(el1);        	      
		}
		// write edges
		for(ITR_Edges it=writeEdges.begin(); it!=writeEdges.end(); ++it)
		{
			el1 = doc->createElementNS(ns, X("writePlace"));
			el1->setAttribute(X("placeID"), XS((*it)->getPlace()->getID()));
			if((*it)->getExpression().size() > 0) el1->setAttribute(X("edgeExpression"), XS((*it)->getExpression()));
			el->appendChild(el1);       	      
		}
		// out edges
		for(ITR_Edges it=outEdges.begin(); it!=outEdges.end(); ++it)
		{
			el1 = doc->createElementNS(ns, X("outputPlace"));
			el1->setAttribute(X("placeID"), XS((*it)->getPlace()->getID()));
			if((*it)->getExpression().size() > 0) el1->setAttribute(X("edgeExpression"), XS((*it)->getExpression()));
			el->appendChild(el1);        	      
		}
		// conditions  
		for(unsigned int i=0; i < conditions.size(); ++i)
		{
			el1 = doc->createElementNS(ns, X("condition"));
			el1->setTextContent(XS(conditions[i]));
			el->appendChild(el1);
		}
		// operation
		if (operationP != NULL) el->appendChild(operationP->toElement(doc));

	}
	catch (const OutOfMemoryException&)
	{
		XERCES_STD_QUALIFIER cerr << "OutOfMemoryException during Transition.toElement()." << XERCES_STD_QUALIFIER endl;
	}
	catch (const DOMException& e)
	{
		XERCES_STD_QUALIFIER cerr << "DOMException during Transition.toElement(). code is:  " << e.code << XERCES_STD_QUALIFIER endl;
		XERCES_STD_QUALIFIER cerr << "Message: " << S(e.msg) << XERCES_STD_QUALIFIER endl;
	}
	catch (...)
	{
		XERCES_STD_QUALIFIER cerr << "An error occurred creating the document during Transition.toElement()." << XERCES_STD_QUALIFIER endl;
	}

	return el;
}

bool Transition::isEnabled()
{
	// check read edges
	for(ITR_Edges it=readEdges.begin(); it!=readEdges.end(); ++it)
	{
		if((*it)->getPlace()->isEmpty()) return false;
	}
	// check input edges
	for(ITR_Edges it=inEdges.begin(); it!=inEdges.end(); ++it)
	{
		vector<Token*> v = (*it)->getPlace()->getTokens();
		if(v.size() == 0) return false;
		else
		{
			vector<Token*>::size_type nTokens = v.size(), nLockedTokens = 0;
			for(vector<Token*>::size_type i=0; i<nTokens; ++i)
			{	
				if(v[i]->isLocked()) ++nLockedTokens;  
			}
			if(nLockedTokens >= nTokens) return false;
		}
	}
	// check write edges
	for(ITR_Edges it=writeEdges.begin(); it!=writeEdges.end(); ++it)
	{
		if((*it)->getPlace()->isEmpty()) return false;
	}
	// check output edges
	for(ITR_Edges it=outEdges.begin(); it!=outEdges.end(); ++it)
	{
		Place* p = (*it)->getPlace();
		if(p->getTokenNumber() == p->getCapacity()) return false;
	}
	// else      
	return true;
}

string Transition::generateID() const
{
	static long counter = 0; 
	ostringstream oss; 
	oss << "t" << counter++;
	return oss.str();
}

int Transition::getAbstractionLevel() const 
{	
//	cout << "gwdl::Transition[" << getID() << "]::getAbstractionLevel()" << endl;
	if (operationP != NULL) return operationP->getAbstractionLevel();
	else return Operation::BLACK;
}


} // end namespace gwdl

ostream& operator<<(ostream &out, gwdl::Transition &trans) 
{	
	DOMNode* node = trans.toElement(gwdl::XMLUtils::Instance()->createEmptyDocument(true));
	gwdl::XMLUtils::Instance()->serialize(out,node,true);
	return out;
}

