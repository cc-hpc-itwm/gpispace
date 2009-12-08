/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#ifndef GWDL_LIBXML2BUILDER_H_
#define GWDL_LIBXML2BUILDER_H_

// gwdl
#include <gwdl/IBuilder.h>
//fhglog
#include <fhglog/fhglog.hpp>
// libxml2
#include <libxml/xpathInternals.h>

namespace gwdl 
{

/**
 * Concrete implementation of interface IBuilder that builds workflow objects from xml strings
 * and vice versa using libxml2.
 */
class Libxml2Builder : public IBuilder 
{
	
private:
	
	 /**
	  * Fhg Logger.
	  */
	 fhg::log::logger_t _logger;
	 
	 /**
	  * gworkflowdl namespace.
	  */
	 xmlNsPtr _nsGworkflowdlP;
	 
	 /**
	  * operationclass namespace.
	  */
	 xmlNsPtr _nsOperationclassP;

public:
	
	/**
	 * Constructor.
	 */
	Libxml2Builder();

	/**
	 * Destructor.
	 */
	virtual ~Libxml2Builder();

	//////////////////////////
	// Data
	//////////////////////////

	// Interface IBuilder
    Data::ptr_t deserializeData(const std::string &) const throw (WorkflowFormatException);
    std::string serializeData(const Data &) const;
  	// libxml2-specific
    Data::ptr_t elementToData(const xmlNodePtr nodeP) const throw (WorkflowFormatException);
    xmlNodePtr dataToElement(const Data &data) const; 

    //////////////////////////
    // Token
    //////////////////////////

    // Interface IBuilder
	Token::ptr_t deserializeToken(const std::string&) const throw (WorkflowFormatException);
	std::string serializeToken(const Token &) const;
	// libxml2-specific
	Token::ptr_t elementToToken(const xmlNodePtr nodeP) const throw (WorkflowFormatException);
	xmlNodePtr tokenToElement(const Token &) const;

	//////////////////////////
	// Properties
	//////////////////////////

    // Interface IBuilder
	std::string serializeProperties(const Properties& props) const;

	// libxml2-specific
	Properties elementsToProperties(const xmlNodePtr nodeP) const throw (WorkflowFormatException);
	std::pair<std::string,std::string> elementToProperty(const xmlNodePtr nodeP) const throw (WorkflowFormatException);
	xmlNodePtr propertiesToElements(const Properties& props) const;
	
    //////////////////////////
    // Place
    //////////////////////////

    // Interface IBuilder
	Place::ptr_t deserializePlace(const std::string&) const throw (WorkflowFormatException);
	std::string serializePlace(const Place &) const;
	// libxml2-specific
	Place::ptr_t elementToPlace(const xmlNodePtr nodeP) const throw (WorkflowFormatException);
	xmlNodePtr placeToElement(const Place &) const;
	
    //////////////////////////
    // OperationCandidate
    //////////////////////////

    // Interface IBuilder
	OperationCandidate::ptr_t deserializeOperationCandidate(const std::string&) const throw (WorkflowFormatException);
	std::string serializeOperationCandidate(const OperationCandidate &) const;
	// libxml2-specific
	OperationCandidate::ptr_t elementToOperationCandidate(const xmlNodePtr nodeP) const throw (WorkflowFormatException);
	xmlNodePtr operationCandidateToElement(const OperationCandidate &) const;

    //////////////////////////
    // OperationClass
    //////////////////////////

    // Interface IBuilder
	OperationClass::ptr_t deserializeOperationClass(const std::string&) const throw (WorkflowFormatException);
	std::string serializeOperationClass(const OperationClass &) const;
	// libxml2-specific
	OperationClass::ptr_t elementToOperationClass(const xmlNodePtr nodeP) const throw (WorkflowFormatException);
	xmlNodePtr operationClassToElement(const OperationClass &) const;

    //////////////////////////
    // Operation
    //////////////////////////

    // Interface IBuilder
	Operation::ptr_t deserializeOperation(const std::string&) const throw (WorkflowFormatException);
	std::string serializeOperation(const Operation &) const;
	// libxml2-specific
	Operation::ptr_t elementToOperation(const xmlNodePtr nodeP) const throw (WorkflowFormatException);
	xmlNodePtr operationToElement(const Operation &) const;

    //////////////////////////
    // Edge
    //////////////////////////

	// libxml2-specific
	Edge::ptr_t elementToEdge(Workflow::ptr_t, const xmlNodePtr nodeP) const throw (WorkflowFormatException);
	xmlNodePtr edgeToElement(const Edge &) const;

	//////////////////////////
    // Transition
    //////////////////////////

    // Interface IBuilder
	Transition::ptr_t deserializeTransition(Workflow::ptr_t, const std::string&) const throw (WorkflowFormatException);
	std::string serializeTransition(const Transition &) const;
	// libxml2-specific
	Transition::ptr_t elementToTransition(Workflow::ptr_t, const xmlNodePtr nodeP) const throw (WorkflowFormatException);
	xmlNodePtr transitionToElement(const Transition &) const;
	
    //////////////////////////
    // Workflow
    //////////////////////////

    // Interface IBuilder
	Workflow::ptr_t deserializeWorkflow(const std::string&) const throw (WorkflowFormatException);
	Workflow::ptr_t deserializeWorkflowFromFile(const std::string& filename) const throw (WorkflowFormatException);
	std::string serializeWorkflow(const Workflow &) const;
	void serializeWorkflowToFile(const Workflow &, const std::string& filename) const;
	// libxml2-specific
	Workflow::ptr_t elementToWorkflow(const xmlNodePtr nodeP) const throw (WorkflowFormatException);
	xmlNodePtr workflowToElement(const Workflow &) const;
	
private:
	//////////////////////////
	// private helper methods
	//////////////////////////
	static xmlNodePtr nextElementNode(const xmlNodePtr curNodeP);
	static xmlNodePtr nextTextNode(const xmlNodePtr nodeP);
	static bool checkElementName(const xmlNodePtr nodeP,const char* name);
	
}; // end class IBuilder

} // end namespace gwdl

//////////////////////////
// << operators
//////////////////////////

// Data
std::ostream& operator<< (std::ostream &out, gwdl::Data &data);

// Token
std::ostream& operator<< (std::ostream &out, gwdl::Token &token);

// Properties
std::ostream& operator<< (std::ostream &out, gwdl::Properties &props);

// Place
std::ostream& operator<< (std::ostream &out, gwdl::Place &place);

// OperationCandidate
std::ostream& operator<< (std::ostream &out, gwdl::OperationCandidate &ocand);

// OperationClass
std::ostream& operator<< (std::ostream &out, gwdl::OperationClass &oclass);

// Operation
std::ostream& operator<< (std::ostream &out, gwdl::Operation &operation);

// Transition
std::ostream& operator<< (std::ostream &out, gwdl::Transition &transition);

// Workflow
std::ostream& operator<< (std::ostream &out, gwdl::Workflow &workflow);

#endif /*GWDL_LIBXML2BUILDER_H_*/
