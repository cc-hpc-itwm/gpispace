/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
// gwdl
#include <gwdl/OperationClass.h>
//fhglog
#include <fhglog/fhglog.hpp>

using namespace std;
using namespace fhg::log;

namespace gwdl
{

OperationClass::~OperationClass() {
	removeAllOperationCandidates();
}

const vector<OperationCandidate::ptr_t>& OperationClass::getOperationCandidates() const {
	return _operationCandidates;
}

AbstractionLevel::abstraction_t OperationClass::getAbstractionLevel() const {
	if(_operationCandidates.size() == 0)	{
		return AbstractionLevel::YELLOW;
	} else {
		int count = 0;
		for (unsigned int i = 0; i < _operationCandidates.size(); i++) {
			if(_operationCandidates[i]->getAbstractionLevel()==AbstractionLevel::GREEN) {
				count++;
			}
		}
		return count == 1 ? AbstractionLevel::GREEN : AbstractionLevel::BLUE;
	}
}

void OperationClass::removeOperationCandidate(int i) {
	(_operationCandidates[i]).reset();
	_operationCandidates.erase(_operationCandidates.begin()+i);
}

void OperationClass::removeOperationCandidate(OperationCandidate::ptr_t ocandP) {
	for (unsigned int i=0; i<_operationCandidates.size(); i++) {
		if (ocandP->getID() == _operationCandidates[i]->getID()) {
			(_operationCandidates[i]).reset();
			_operationCandidates.erase(_operationCandidates.begin()+i);
			break;	
		}	
	}
}

void OperationClass::removeAllOperationCandidates() {
	for (unsigned int i=0; i<_operationCandidates.size(); i++) {
		(_operationCandidates[i]).reset();
	}
	_operationCandidates.clear();
}

}
