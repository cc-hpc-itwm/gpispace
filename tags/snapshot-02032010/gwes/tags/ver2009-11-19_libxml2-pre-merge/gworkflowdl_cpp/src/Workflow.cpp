/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
//gwdl
#include <gwdl/Workflow.h>
//fhglog
#include <fhglog/fhglog.hpp>

using namespace fhg::log;
using namespace std;

namespace gwdl {

Workflow::Workflow(const string& id) {
	_id = (id == "") ? WORKFLOW_DEFAULT_ID : id;
	_description = WORKFLOW_DEFAULT_DESCRIPTION;
	LOG_DEBUG(logger_t(getLogger("gwdl")), "Workflow[" << _id << "]");
}

Workflow::~Workflow() {
	for(vector<Transition::ptr_t>::iterator it=_transitions.begin(); it!=_transitions.end(); ++it) (*it).reset();
	_transitions.clear();
	_enabledTransitions.clear();
	for(map<string,Place::ptr_t>::iterator it=_places.begin(); it!=_places.end(); ++it) (it->second).reset();
	_places.clear();
	LOG_DEBUG(logger_t(getLogger("gwdl")), "~Workflow[" << _id << "]");
}

Place::ptr_t Workflow::getPlace(unsigned int i) throw (NoSuchWorkflowElement) {
	unsigned int j=0;
	for(map<string,Place::ptr_t>::iterator it=_places.begin(); it!=_places.end(); ++it)	{
		if(j++ == i) return (it->second);
	}
	// no such workflow element
	ostringstream message; 
	message << "Place with index=\"" << i << "\" is not available!";
	throw NoSuchWorkflowElement(message.str());
}

Place::ptr_t Workflow::getPlace(const string& id) throw (NoSuchWorkflowElement) {	
	map<string,Place::ptr_t>::iterator iter = _places.find(id);
	if (iter!=_places.end()) return (iter->second);
	// no such workflow element
	ostringstream message; 
	message << "Place with id=\"" << id << "\" is not available!";
	throw NoSuchWorkflowElement(message.str());
}

unsigned int Workflow::getPlaceIndex(const string& id) throw (NoSuchWorkflowElement) {
	int j=0;
	for(map<string,Place::ptr_t>::iterator it=_places.begin(); it!=_places.end(); ++it) {
		if(it->first == id) return j;
		++j;
	}
	// no such workflow element
	ostringstream message; 
	message << "Place with id=\"" << id << "\" is not available!";
	throw NoSuchWorkflowElement(message.str());
}

void Workflow::removePlace(unsigned int i) throw (NoSuchWorkflowElement) {
	if (i>=_places.size()) {
		// no such workflow element
		ostringstream message; 
		message << "Place with index=\"" << i << "\" is not available!";
		throw NoSuchWorkflowElement(message.str());
	}
	unsigned int j=0;
	for(map<string,Place::ptr_t>::iterator it=_places.begin(); it!=_places.end(); ++it)	{
		if(j++ == i) {
			(it->second).reset();
			_places.erase(it);
			break;
		}
	}
}

void Workflow::removeTransition(unsigned int i) throw (NoSuchWorkflowElement) {	
	if (i>=_transitions.size())	{
		// no such workflow element
		ostringstream message; 
		message << "Transition with index=\"" << i << "\" is not available!";
		throw NoSuchWorkflowElement(message.str());
	}
	Transition::ptr_t tP = *(_transitions.begin()+i);
	tP.reset();
	_transitions.erase(_transitions.begin()+i);
}

Transition::ptr_t Workflow::getTransition(const string& id) throw (NoSuchWorkflowElement) {
	for(unsigned int i=0; i<_transitions.size(); ++i) {
		if(_transitions[i]->getID() == id) return _transitions[i];
	}
	// no such workflow element
	ostringstream message; 
	message << "Transition with id=\"" << id << "\" is not available!";
	throw NoSuchWorkflowElement(message.str());
}

unsigned int Workflow::getTransitionIndex(const string& id) const throw (NoSuchWorkflowElement) {
	for(unsigned int i=0; i<_transitions.size(); ++i) {
		if(_transitions[i]->getID() == id) return i;
	}
	// no such workflow element
	ostringstream message; 
	message << "Transition with id=\"" << id << "\" is not available!";
	throw NoSuchWorkflowElement(message.str());
}

Transition::ptr_t Workflow::getTransition(unsigned int i) throw (NoSuchWorkflowElement) {	
	if (i>=_transitions.size())	{
		// no such workflow element
		ostringstream message; 
		message << "Transition with index=\"" << i << "\" is not available!";
		throw NoSuchWorkflowElement(message.str());
	}
	return _transitions[i];
}

void Workflow::putProperty(const string& name, const string& value) {
	if (_propertiesP == NULL) {
		_propertiesP = Properties::ptr_t(new Properties());
	}
	_propertiesP->put(name,value);
}

/**
 * return the workflow's enabled _transitions as vector.
 * @return vector of enabled _transitions.
 */
vector<Transition::ptr_t>& Workflow::getEnabledTransitions() {
  _enabledTransitions.clear();
  for(vector<Transition::ptr_t>::iterator it=_transitions.begin(); it != _transitions.end(); ++it) {
    if((*it)->isEnabled()) _enabledTransitions.push_back(*it);
  }
  return _enabledTransitions;
}

} // end namespace gwdl
