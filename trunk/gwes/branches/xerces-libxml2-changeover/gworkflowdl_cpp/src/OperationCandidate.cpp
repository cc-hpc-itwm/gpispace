/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
// gwdl
#include <gwdl/OperationCandidate.h>
//fhglog
#include <fhglog/fhglog.hpp>

using namespace fhg::log;
using namespace std;

namespace gwdl
{

OperationCandidate::OperationCandidate() {
	_id = generateID(); _selected=false; _type = ""; _operationName = ""; _resourceName = ""; _quality = -1.0;
}

AbstractionLevel::abstraction_t OperationCandidate::getAbstractionLevel() const {
	if (_selected) return AbstractionLevel::GREEN;
	else return AbstractionLevel::BLUE;		
}

} // end namespace gwdl
