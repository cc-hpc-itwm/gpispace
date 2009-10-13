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

public:
	
	/**
	 * Constructor.
	 */
	Libxml2Builder();

	/**
	 * Destructor.
	 */
	virtual ~Libxml2Builder();

	// Data
    const Data::ptr_t deserializeData(const std::string &) const;
    const std::string serializeData(const Data::ptr_t &) const;
	
	// Token
	const Token::ptr_t deserializeToken(const std::string&) const;
	const std::string serializeToken(const Token::ptr_t &) const;
	
	// Place
//	virtual Place deserializePlace(const std::string& xmlstring);
//	virtual std::string serializePlace(const Place& place);
	
}; // end class IBuilder

} // end namespace gwdl

#endif /*GWDL_LIBXML2BUILDER_H_*/
