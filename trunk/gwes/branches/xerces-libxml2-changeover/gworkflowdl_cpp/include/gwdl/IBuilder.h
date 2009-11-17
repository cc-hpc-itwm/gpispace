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
#include <gwdl/OperationCandidate.h>
#include <gwdl/OperationClass.h>
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

	// Properties
	virtual std::string serializeProperties(const Properties&) const = 0;

	// Place
	virtual Place::ptr_t deserializePlace(const std::string &) const throw (WorkflowFormatException) = 0;
	virtual std::string serializePlace(const Place &) const = 0;

	// OperationCandidate
	virtual OperationCandidate::ptr_t deserializeOperationCandidate(const std::string &) const throw (WorkflowFormatException) = 0;
	virtual std::string serializeOperationCandidate(const OperationCandidate &) const = 0;
	
	// OperationClass
	virtual OperationClass::ptr_t deserializeOperationClass(const std::string &) const throw (WorkflowFormatException) = 0;
	virtual std::string serializeOperationClass(const OperationClass &) const = 0;
	
	// Operation
	virtual Operation::ptr_t deserializeOperation(const std::string &) const throw (WorkflowFormatException) = 0;
	virtual std::string serializeOperation(const Operation &) const = 0;

	// Transition

	// Workflow

}; // end class IBuilder

} // end namespace gwdl

#endif /*GWDL_IBUILDER_H_*/
