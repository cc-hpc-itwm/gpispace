/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#ifndef PROPERTIES_H_
#define PROPERTIES_H_
// std
#include <string>
#include <map>

namespace gwdl
{
	
typedef std::map<std::string, std::string>::iterator ITR_Properties;
typedef std::map<std::string, std::string>::const_iterator CITR_Properties;

/**
 * The Properties class is a map used to store properties of Workflow, Transition, Place, and Token.
 * The first part of the map are the property names, the second part the property values.
 *  
 * @version $Id$
 * @author Andreas Hoheisel and Helge Ros&eacute; &copy; 2008 <a href="http://www.first.fraunhofer.de/">Fraunhofer FIRST</a>  
 */ 
class Properties : public std::map<std::string,std::string>
{
	
public:
	
	/**
	 * Constructor for properties.
	 */
	Properties();
	
	/**
	 * Destructor for properties.
	 */
	virtual ~Properties();
	
	/**
	 * Put new name/value pair into properties.
	 * Overwrites old property with same name.
	 * @param name The name of the property.
	 * @param value The value of the property.
	 */
	void put(const std::string& name, const std::string& value);

	/**
	 * Remove property with specific name from properties.
	 * @param name The name of the property.
	 */
	void remove(const std::string& name);
	
	/**
	 * Get specific property value.
	 * @param name The name of the property.
	 * @return empty string "" if property with name not found.
	 */
	std::string get(const std::string& name);

	/**
	 * Test if the properties contain specific property name.
	 * @param name The name of the property.
	 */
	bool contains(const std::string& name); 
	 
	/*
	 * Get number of properties.
	 */
	//int size(); map defines size()
	
	/**
	 * Make a deep copy of this object.
	 * Caller must take care of destroying the object afterwards.
	 * @return Pointer to cloned Properties.
	 */ 
	Properties* deepCopy() const;
	
}; // end class Properties

} // end namespace gwdl

#endif /*PROPERTIES_H_*/
