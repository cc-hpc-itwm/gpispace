/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
//gwdl
#include <gwdl/Workflow.h>
#include <gwdl/Token.h>
//fhglog
#include <fhglog/fhglog.hpp>

using namespace fhg::log;
using namespace std;

namespace gwdl {

Workflow::Workflow(const workflow_id_t& id) {
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

gwdl::workflow_result_t Workflow::getResults() const
{
  gwdl::workflow_result_t results;

  // iterate over all places
  for (place_map_t::const_iterator a_place(_places.begin()); a_place != _places.end(); ++a_place)
  {
    const std::string &place_name = a_place->first;
    const Place::ptr_t place = a_place->second;

    try
    {
      gwdl::token_list_t tokens;

      DLOG(DEBUG, "getting tokens from place: " << place_name);
      typedef std::vector<Token::ptr_t> place_token_list_t;

      place_token_list_t place_tokens = place->getTokens();

      for (place_token_list_t::const_iterator gwdl_token(place_tokens.begin()); gwdl_token != place_tokens.end(); ++gwdl_token)
      {
    	  //ToDo: is it really neccessary to make a deep copy? Why not to reuse tokens?
        tokens.push_back((*gwdl_token)->deepCopy());
      }
      DLOG(DEBUG, "found " << tokens.size() << " tokens on place " << place_name);

      results.insert(std::make_pair(place_name, tokens));
    }
    catch (const gwdl::NoSuchWorkflowElement &)
    {
      LOG(WARN, "Inconsistencey detected: the workflow does not contain place: " << place_name);
    }
  }
  return results;
}

Place::ptr_t& Workflow::getPlace(unsigned int i) throw (NoSuchWorkflowElement) {
	unsigned int j=0;
	for(map<string,Place::ptr_t>::iterator it=_places.begin(); it!=_places.end(); ++it)	{
		if(j++ == i) return (it->second);
	}
	// no such workflow element
	ostringstream message; 
	message << "Place with index=\"" << i << "\" is not available!";
	throw NoSuchWorkflowElement(message.str());
}

Place::ptr_t& Workflow::getPlace(const string& id) throw (NoSuchWorkflowElement) {	
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

Transition::ptr_t& Workflow::getTransition(const string& id) throw (NoSuchWorkflowElement) {
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

Transition::ptr_t& Workflow::getTransition(unsigned int i) throw (NoSuchWorkflowElement) {	
	if (i>=_transitions.size())	{
		// no such workflow element
		ostringstream message; 
		message << "Transition with index=\"" << i << "\" is not available!";
		throw NoSuchWorkflowElement(message.str());
	}
	return _transitions[i];
}

void Workflow::putProperty(const string& name, const string& value) {
	_properties.put(name,value);
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
