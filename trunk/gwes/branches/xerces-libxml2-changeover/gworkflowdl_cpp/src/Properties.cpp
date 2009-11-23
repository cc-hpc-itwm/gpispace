/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */

// gwdl
#include <gwdl/Properties.h>
//fhglog
#include <fhglog/fhglog.hpp>

using namespace std;

namespace gwdl
{


/**
 * Constructor.
 */
Properties::Properties() {
	LOG_DEBUG(logger_t(getLogger("gwdl")), "Properties()");
}

/**
 * Destructor.
 */
Properties::~Properties() {
	//clear();
	LOG_DEBUG(logger_t(getLogger("gwdl")), "~Properties()");
}

/**
 * Put new name/value pair into properties.
 * Overwrites old property with same name.
 * @param name The name of the property.
 * @param value The value of the property.
 */
void Properties::put(const string& name, const string& value) {
	// key not yet in map
	if (find(name)==end()) {
	    insert(pair<string,string>(name,value));		
	} 
	// key already in map, remove old value!
	else {
		erase(name);
	    insert(pair<string,string>(name,value));		
	}
}
 
/**
 * Remove property with specific name from properties.
 * @param name The name of the property.
 */
void Properties::remove(const string& name) {
  erase(name);
}

/**
 * Get specific property value.
 * @param name The name of the property.
 * @return empty string "" if property with name not found.
 */
string Properties::get(const string& name) {
  ITR_Properties it = find(name);
  return it != end() ? it->second : "";
}

/**
 * Test if the properties contain specific property name.
 * @param name The name of the property.
 */
bool Properties::contains(const string& name) {
  ITR_Properties it = find(name);
  return (it != end());
}
 
Properties* Properties::deepCopy() const {
	Properties* propsP = new Properties();
	propsP->insert(begin(),end());
	LOG_DEBUG(logger_t(getLogger("gwdl")), "Properties::deepCopy()");
	return propsP;
}

/*
 * Get number of properties.
 */
//int size(); map defines size()

} // end namespace gwdl

