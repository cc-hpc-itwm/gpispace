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
	
private:
	//////////////////////////
	// private helper methods
	//////////////////////////
	static xmlNodePtr nextElementNode(xmlNodePtr curNodeP);
	static xmlNodePtr nextTextNode(xmlNodePtr nodeP);
	
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

#endif /*GWDL_LIBXML2BUILDER_H_*/
