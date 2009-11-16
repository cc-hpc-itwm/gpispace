/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#ifndef TOKENPARAMETER_H_
#define TOKENPARAMETER_H_

#include <gwes/ITokenParameter.h>

//gwdl
#include <gwdl/Token.h>
#include <gwdl/Edge.h>

namespace gwes 
{

class TokenParameter : public ITokenParameter
{

public:

	/**
	 * Scope of parameter.
	 */
	enum Scope
	{
		SCOPE_READ = 0,
		SCOPE_INPUT = 1,
		SCOPE_WRITE = 2,
		SCOPE_OUTPUT = 3
	};
	
	/**
	 * Pointer to edge.
	 */
	gwdl::Edge* edgeP;
	
	/**
	 * Pointer to token.
	 */
	gwdl::Token* tokenP;
	
	Scope scope;

	/**
	 * Constructor.
	 */
	explicit TokenParameter(gwdl::Token* _tokenP, gwdl::Edge* _edgeP, Scope _scope) {
		tokenP = _tokenP;
		edgeP = _edgeP;
		scope = _scope;
	}

};

} // end namespace gwes 
	
#endif /*TOKENPARAMETER_H_*/
