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
// std
#include <string>

namespace gwdl 
{

class IBuilder 
{

public:

	// Data 
	virtual Data deserializeData(const std::string& xmlstring) = 0;
	virtual std::string serialize(const Data& data) = 0;
	
	// Token
	virtual Token deserializeToken(const std::string& xmlstring) = 0;
	virtual std::string serialize(const Token& token) = 0;
	
	// Place
	virtual Place deserializePlace(const std::string& xmlstring) = 0;
	virtual std::string serialize(const Place& place) = 0;
	
	// Transition
	
	// Operation ...
	
	// Workflow
	
}; // end class IBuilder

} // end namespace gwdl

#endif /*GWDL_IBUILDER_H_*/
