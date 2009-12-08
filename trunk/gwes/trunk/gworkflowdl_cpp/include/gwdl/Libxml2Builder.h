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
    const Data::ptr_t deserializeData(const std::string &) const;
    const std::string serializeData(const Data::ptr_t &) const;
  	// libxml2-specific
    const xmlNodePtr dataToElement(const Data &data) const; 

    //////////////////////////
    // Token
    //////////////////////////
    // Interface IBuilder
	const Token::ptr_t deserializeToken(const std::string&) const;
	const std::string serializeToken(const Token::ptr_t &) const;
	// libxml2-specific
	const std::string serializeToken(const Token &) const;
	const Token::ptr_t elementToToken(const xmlNodePtr nodeP) const;
	const xmlNodePtr tokenToElement(const Token &) const;
	
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
