/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#ifndef TOKENPARAMETER_H_
#define TOKENPARAMETER_H_
//gwdl
#include <gwdl/Token.h>
#include <gwdl/Edge.h>

namespace gwes 
{

class TokenParameter 
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
	gwdl::Edge::ptr_t edgeP;
	
	/**
	 * Pointer to token.
	 */
	gwdl::Token::ptr_t tokenP;
	
	Scope scope;

	/**
	 * Constructor with tokenP = NULL.
	 */
	explicit TokenParameter(gwdl::Edge::ptr_t _edgeP, Scope _scope) {
		edgeP = _edgeP;
		scope = _scope;
	}

	/**
	 * Constructor.
	 */
	explicit TokenParameter(gwdl::Token::ptr_t _tokenP, gwdl::Edge::ptr_t _edgeP, Scope _scope) {
		tokenP = _tokenP;
		edgeP = _edgeP;
		scope = _scope;
	}

};

} // end namespace gwes 
	
#endif /*TOKENPARAMETER_H_*/
