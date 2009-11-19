/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
//gwdl
#include <gwdl/Operation.h>
//fhglog
#include <fhglog/fhglog.hpp>

using namespace fhg::log;
using namespace std;

namespace gwdl
{

Operation::Operation()
{
	// _operationClassP = NULL; // default for shared pointer
}

Operation::~Operation()
{
	_operationClassP.reset();
}

AbstractionLevel::abstraction_t Operation::getAbstractionLevel() const {	
	if(_operationClassP) return _operationClassP->getAbstractionLevel();
	else return AbstractionLevel::RED;
}

} //end namespace gwdl
