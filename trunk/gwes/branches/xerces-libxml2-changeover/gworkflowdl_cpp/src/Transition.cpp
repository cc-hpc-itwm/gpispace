/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
//fhglog
#include <fhglog/fhglog.hpp>
// gwdl
#include <gwdl/Transition.h>
#include <gwdl/Token.h>
#include <gwdl/Defines.h>
#include <gwdl/Workflow.h>

using namespace fhg::log;
using namespace std;

namespace gwdl
{

Transition::Transition(const string& id) {
	if (id == "") _id=generateID();
	else _id = id;
	_description = "";
	// _operationP = NULL; // default for shared pointer 
	_status = Transition::STATUS_NONE;
	LOG_DEBUG(logger_t(getLogger("gwdl")), "Transition[" << _id << "]");
}

Transition::~Transition() {
	_operationP.reset();
	for(ITR_Edges it=_readEdges.begin(); it!=_readEdges.end(); ++it) (*it).reset();
	_readEdges.clear();
	for(ITR_Edges it=_inEdges.begin(); it!=_inEdges.end(); ++it) (*it).reset();
	_inEdges.clear();
	for(ITR_Edges it=_writeEdges.begin(); it!=_writeEdges.end(); ++it) (*it).reset();
	_writeEdges.clear();
	for(ITR_Edges it=_outEdges.begin(); it!=_outEdges.end(); ++it) (*it).reset();
	_outEdges.clear();
	LOG_DEBUG(logger_t(getLogger("gwdl")), "~Transition[" << _id << "]");
}

/**
 * add an Edge considering its scope.
 * @param edge Edge to be added.
 */
void Transition::addEdge(Edge::ptr_t edgeP) {
	switch(edgeP->getScope()) {
	case(Edge::SCOPE_READ):
		_readEdges.push_back(edgeP);
		break;
	case(Edge::SCOPE_INPUT):
		_inEdges.push_back(edgeP);
		break;
	case(Edge::SCOPE_WRITE):
		_writeEdges.push_back(edgeP);
		break;
	case(Edge::SCOPE_OUTPUT):
		_outEdges.push_back(edgeP);
		break;
	}
}

void Transition::putProperty(const string& name, const string& value) {
	if (_propertiesP == NULL) {
		_propertiesP = Properties::ptr_t(new Properties());
	}
	_propertiesP->put(name,value);
}

bool Transition::isEnabled() {
	// check read edges
	for(ITR_Edges it=_readEdges.begin(); it!=_readEdges.end(); ++it) {
		if((*it)->getPlace()->isEmpty()) return false;
	}
	// check input edges
	for(ITR_Edges it=_inEdges.begin(); it!=_inEdges.end(); ++it) {
		if( (*it)->getPlace()->getNextUnlockedToken() == NULL ) return false;
	}
	// check write edges
	for(ITR_Edges it=_writeEdges.begin(); it!=_writeEdges.end(); ++it) {
		if((*it)->getPlace()->isEmpty()) return false;
	}
	// check output edges
	for(ITR_Edges it=_outEdges.begin(); it!=_outEdges.end(); ++it) {
		Place::ptr_t p = (*it)->getPlace();
		///ToDo: consider reserved space for transitions that locked a "reservation token".
		if(p->getTokenNumber() == p->getCapacity()) return false;
	}
	// else      
	return true;
}

string Transition::generateID() const {
	static long counter = 0; 
	ostringstream oss; 
	oss << "t" << counter++;
	return oss.str();
}

int Transition::getAbstractionLevel() const {	
	if (_operationP) return _operationP->getAbstractionLevel();
	else return AbstractionLevel::BLACK;
}

} // end namespace gwdl
