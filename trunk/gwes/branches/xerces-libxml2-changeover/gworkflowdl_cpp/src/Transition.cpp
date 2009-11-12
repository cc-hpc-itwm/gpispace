/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
// xerces-c
#include <xercesc/util/OutOfMemoryException.hpp>
//fhglog
#include <fhglog/fhglog.hpp>
// gwdl
#include <gwdl/Transition.h>
#include <gwdl/Defines.h>
#include <gwdl/Workflow.h>
#include <gwdl/XMLUtils.h>

#include <gwdl/XMLTranscode.hpp>

using namespace fhg::log;
XERCES_CPP_NAMESPACE_USE
using namespace std;

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
	for(ITR_Edges it=readEdges.begin(); it!=readEdges.end(); ++it) (*it).reset();
	readEdges.clear();
	for(ITR_Edges it=inEdges.begin(); it!=inEdges.end(); ++it) (*it).reset();
	inEdges.clear();
	for(ITR_Edges it=writeEdges.begin(); it!=writeEdges.end(); ++it) (*it).reset();
	writeEdges.clear();
	for(ITR_Edges it=outEdges.begin(); it!=outEdges.end(); ++it) (*it).reset();
	outEdges.clear();
}

Transition::Transition(Workflow* wf, DOMElement* element)
{
	LOG_DEBUG(logger_t(getLogger("gwdl")), "Transition() of workflow "<< wf->getID());
	
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
		///ToDo: Migration to libxml2.
//		properties = Properties(le);
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
			
			LOG_WARN(logger_t(getLogger("gwdl")), "///ToDo: Migration to libxml2 and shared_ptr///");
//			// readPlace
//			else if (XMLString::equals(name,X("readPlace"))) {
//				DOMElement* el = (DOMElement*)node;
//				addReadEdge(
//						new Edge(wf->getPlace(string(S(el->getAttribute(X("placeID"))))),
//								string(S(el->getAttribute(X("edgeExpression")))))
//				); 
//			}
//			// inputPlace
//			else if (XMLString::equals(name,X("inputPlace"))) {
//				DOMElement* el = (DOMElement*)node;
//				addInEdge(
//						new Edge(wf->getPlace(string(S(el->getAttribute(X("placeID"))))),
//								string(S(el->getAttribute(X("edgeExpression")))))
//				); 
//			}
//			// writePlace
//			else if (XMLString::equals(name,X("writePlace"))) {
//				DOMElement* el = (DOMElement*)node;
//				addWriteEdge(
//						new Edge(wf->getPlace(string(S(el->getAttribute(X("placeID"))))),
//								string(S(el->getAttribute(X("edgeExpression")))))
//				); 
//			}
//			// outputPlace
//			else if (XMLString::equals(name,X("outputPlace"))) {
//				DOMElement* el = (DOMElement*)node;
//				addOutEdge(
//						new Edge(wf->getPlace(string(S(el->getAttribute(X("placeID"))))),
//								string(S(el->getAttribute(X("edgeExpression")))))
//				); 
//			}
//			// operation
//			else if (XMLString::equals(name,X("operation"))) {
//				operationP = new Operation((DOMElement*)node);
//			}
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
		///ToDo: Migration to libxml2.
//		vector<DOMElement*> v = properties.toElements(doc);       
//		for (unsigned int i = 0; i < v.size(); i++)
//		{
//			el->appendChild(v[i]);
//		}

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
		LOG_FATAL(logger_t(getLogger("gwdl")), "OutOfMemoryException during Transition.toElement()." );
	}
	catch (const DOMException& e)
	{
		LOG_ERROR(logger_t(getLogger("gwdl")), "DOMException during Transition.toElement(). code is:  " << e.code );
		LOG_ERROR(logger_t(getLogger("gwdl")), "Message: " << S(e.msg) );
	}
	catch (...)
	{
		LOG_ERROR(logger_t(getLogger("gwdl")), "An error occurred creating the document during Transition.toElement()." );
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
		LOG_WARN(logger_t(getLogger("gwdl")), "///ToDo: migration to libxml2///");
//		vector<Token::ptr_t> v = (*it)->getPlace()->getTokens();
//		if(v.size() == 0) return false;
//		else
//		{
//			vector<Token::ptr_t>::size_type nTokens = v.size(), nLockedTokens = 0;
//			for(vector<Token::ptr_t>::size_type i=0; i<nTokens; ++i)
//			{	
//				if(v[i]->isLocked()) ++nLockedTokens;  
//			}
//			if(nLockedTokens >= nTokens) return false;
//		}
	}
	// check write edges
	for(ITR_Edges it=writeEdges.begin(); it!=writeEdges.end(); ++it)
	{
		if((*it)->getPlace()->isEmpty()) return false;
	}
	// check output edges
	for(ITR_Edges it=outEdges.begin(); it!=outEdges.end(); ++it)
	{
		Place::ptr_t p = (*it)->getPlace();
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
	if (operationP != NULL) return operationP->getAbstractionLevel();
	else return AbstractionLevel::BLACK;
}

} // end namespace gwdl

ostream& operator<<(ostream &out, gwdl::Transition &trans) 
{	
	DOMNode* node = trans.toElement(gwdl::XMLUtils::Instance()->createEmptyDocument(true));
	gwdl::XMLUtils::Instance()->serialize(out,node,true);
	return out;
}

