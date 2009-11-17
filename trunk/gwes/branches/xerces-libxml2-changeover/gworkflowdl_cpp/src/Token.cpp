/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */

// gwdl
#include <gwdl/Defines.h>
#include <gwdl/Token.h>
//fhglog
#include <fhglog/fhglog.hpp>

using namespace fhg::log;
using namespace std;

namespace gwdl
{

/**
 * Construct a default control token with control = <code>true</code>.
 */ 
Token::Token() {
	_id = generateID(); 
	//_dataP=NULL; 
	_control = CONTROL_TRUE; 
	//_lockP = NULL; // default for shared pointer
//	LOG_DEBUG(logger_t(getLogger("gwdl")), "Token[" << _id << "]");
}

/**
 * Constructor for control token with specified value.
 * @param _control Boolean value of the control token.
 */ 
Token::Token(control_t control) {
	_id = generateID(); 
	//_dataP=NULL; 
	_control = control; 
	// _lockP = NULL; // default for shared pointer
//	LOG_DEBUG(logger_t(getLogger("gwdl")), "Token[" << _id << "]");
}

/**
 * Constructor for control token with specified value and properties.
 * @param _properties The properties of this token.
 * @param _control The control value of this token.
 */
Token::Token(Properties::ptr_t propertiesP, control_t control) {
	_id = generateID(); 
	//_dataP=NULL; 
	_control = control; 
	_propertiesP = propertiesP; 
	// _lockP = NULL; // default for shared pointer
//	LOG_DEBUG(logger_t(getLogger("gwdl")), "Token[" << _id << "] with " << _propertiesP->size() << " properties.");
}

/**
 * Constructor for data token.
 * Note: When the token is deleted, then also the data object will be deleted! 
 * @param _data XML content of the data token as data object.
 */  
Token::Token(Data::ptr_t dataP) {
	_id = generateID(); 
	_dataP = dataP;	
	// _lockP = NULL; // default for shared pointer
//	LOG_DEBUG(logger_t(getLogger("gwdl")), "Token[" << _id << "]");
}

/**
 * Constructor for data token with specific properties.
 * Note: When the token is deleted, then also the data object will be deleted! 
 * @param _properties The properties of this data token.
 * @param _data The data of this token.
 */
Token::Token(Properties::ptr_t propertiesP, Data::ptr_t dataP) {
	_id = generateID(); 
	_propertiesP = propertiesP; 
	_dataP = dataP; 
	// _lockP = NULL; // default for shared pointer
//	LOG_DEBUG(logger_t(getLogger("gwdl")), "Token[" << _id << "] with " << _propertiesP->size() << " properties.");
} 

Token::~Token() {
	// done by Data::shared_ptr
	//if (isData()) delete data;
	//data = NULL;
//	LOG_DEBUG(logger_t(getLogger("gwdl")), "~Token[" << _id << "]");
}

Token::ptr_t Token::deepCopy() {
	Token::ptr_t ret; 
	if (isData()) { 	// data token
		ret = Token::ptr_t(new Token(_propertiesP->deepCopy(), _dataP->deepCopy()));
		LOG_INFO(logger_t(getLogger("gwdl")), "generating deepCopy of data token " << _id << " with id " << ret->getID() );
	} else {             // control token
		ret = Token::ptr_t(new Token(_propertiesP->deepCopy(), _control));
		LOG_INFO(logger_t(getLogger("gwdl")), "generating deepCopy of control token " << _id << " with id " << ret->getID() );
	}
	return ret;
}

} // end namespace gwdl
