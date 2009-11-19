/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
//gwdl
#include <gwdl/Place.h>
#include <gwdl/Transition.h>
#include <gwdl/Token.h>
#include <gwdl/Defines.h>
//fhglog
#include <fhglog/fhglog.hpp>

using namespace fhg::log;
using namespace std;

namespace gwdl
{

Place::Place(const string& id) : _capacity(INT_MAX) {
	if (id == "") _id=generateID();
	else _id = id;
	_capacity = Place_DEFAULT_CAPACITY;
	// _nextUnlockedTokenP = NULL; // is NULL by default because of shared_ptr
	LOG_DEBUG(logger_t(getLogger("gwdl")), "Place[" << _id << "]");
}

Place::~Place() {
	removeAllTokens();
	LOG_DEBUG(logger_t(getLogger("gwdl")), "~Place[" << _id << "]");
}

const string& Place::getID() const {
	return _id;
}

void Place::setTokenType(const string& tokenType) {
	_tokenType = tokenType;
}

const string& Place::getTokenType() const {
	return _tokenType;	
}

bool Place::isEmpty() const {
	return (_tokens.empty());
}

void Place::addToken(Token::ptr_t tokenP) throw(CapacityException) {
	if (_tokens.size() >= _capacity) {
		ostringstream oss; 
		oss << "Trying to put too many tokens on place \"" << _id << "\"";
		throw CapacityException(oss.str()); 
	}
	_tokens.push_back(tokenP);
	_nextUnlockedTokenP.reset();
}

void Place::removeToken(int i) {
	_tokens[i].reset();
	_tokens.erase(_tokens.begin()+i);
	_nextUnlockedTokenP.reset();
}

const vector<Token::ptr_t>& Place::getTokens() const {
	return _tokens;	
}

void Place::removeToken(Token::ptr_t tokenP) {
	for(vector<Token::ptr_t>::iterator it=_tokens.begin(); it != _tokens.end(); ++it) {
		if ((*it)->getID() == tokenP->getID()) {
			(*it).reset();
			_tokens.erase(it);
			_nextUnlockedTokenP.reset();
			break;	
		}	
	}	
}

void Place::removeAllTokens() {
	for(vector<Token::ptr_t>::iterator it=_tokens.begin(); it != _tokens.end(); ++it) (*it).reset();
	_tokens.clear();	
	_nextUnlockedTokenP.reset();
}

void Place::setCapacity(unsigned int capacity) throw(CapacityException) {
	if (capacity < _tokens.size()) {
		ostringstream oss; 
		oss << "There are too many tokens on the place \"" << _id << "\" for setting capacity";
		throw CapacityException(oss.str());
	}
	_capacity = capacity;
}

int Place::getCapacity() const {
	return _capacity;
}

int Place::getTokenNumber() const {
	return _tokens.size();
}

void Place::setDescription(const string& d) {
	_description = d;
}

const string& Place::getDescription() const {
	return _description;	
}

void Place::setProperties(Properties::ptr_t propertiesP) {
	_propertiesP = propertiesP;
}

void Place::putProperty(const string& name, const string& value) {
	if (_propertiesP == NULL) {
		_propertiesP = Properties::ptr_t(new Properties());
	}
	_propertiesP->put(name,value);
}

Properties::ptr_t Place::getProperties() {
	return _propertiesP;	
}

const Properties::ptr_t Place::readProperties() const {
	return _propertiesP;
}

void Place::lockToken(Token::ptr_t tokenP, Transition* transitionP) {
	_nextUnlockedTokenP.reset();
	tokenP->lock(transitionP);
}

void Place::unlockToken(Token::ptr_t tokenP) {
	_nextUnlockedTokenP.reset();
	tokenP->unlock();
}

Token::ptr_t Place::getNextUnlockedToken() {
	if ( _nextUnlockedTokenP == NULL) {
		for (size_t i=0; i<_tokens.size(); i++) {
			if (!_tokens[i]->isLocked()) {
				_nextUnlockedTokenP = _tokens[i];
				break;
			}
		}
	}
	return _nextUnlockedTokenP;
}

string Place::generateID() const {
	static long counter = 0; 
	ostringstream oss; 
	oss << "p" << counter++;
	return oss.str();
}

} // end namespace gwdl
