/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */

// gwdl
#include <gwdl/Defines.h>
#include <gwdl/XMLUtils.h>
#include <gwdl/Token.h>
//fhglog
#include <fhglog/fhglog.hpp>
// std
#include <iostream>

using namespace fhg::log;
using namespace std;

namespace gwdl
{
	
Token* Token::deepCopy() {
	Token* ret = NULL; 
	if (isData()) { 	// data token
		ret = new Token(properties, data->deepCopy());
	} else {             // control token
		ret = new Token(properties, control);
	}
	return ret;
}

} // end namespace gwdl

ostream& operator<<(ostream &out, gwdl::Token &token) 
{	
	out << "<token/>";
	// ToDo: implement using BuilderFactory pattern?
	return out;
}

