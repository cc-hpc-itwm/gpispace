/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#ifndef GWDL_IBUILDER_H_
#define GWDL_IBUILDER_H_

// gwdl
#include <gwdl/Data.h>
#include <gwdl/Token.h>
#include <gwdl/Place.h>
#include <gwdl/Transition.h>
#include <gwdl/Workflow.h>
#include <gwdl/WorkflowFormatException.h>
// std
#include <string>

namespace gwdl 
{

class IBuilder 
{

public:

	// Data 
    virtual Data::ptr_t deserializeData(const std::string &) const throw (WorkflowFormatException) = 0;
    virtual std::string serializeData(const Data &) const = 0;

    // Token
	virtual Token::ptr_t deserializeToken(const std::string &) const throw (WorkflowFormatException) = 0;
	virtual std::string serializeToken(const Token &) const = 0;
	
	// Place
	
	// Transition
	
	// Operation ...
	
	// Workflow
	
}; // end class IBuilder

} // end namespace gwdl

#endif /*GWDL_IBUILDER_H_*/
