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
#include <vector>
// xerces-c
#include <xercesc/dom/DOM.hpp>

namespace gwdl
{
	
typedef std::map<std::string, std::string>::iterator ITR_Properties;

/**
 * The Properties class is a map used to store properties of Workflow, Transition, Place, and Token.
 * The first part of the map are the property names, the second part the property values.
 *  
 * @version $Id$
 * @author Andreas Hoheisel and Helge Ros&eacute; &copy; 2008 <a href="http://www.first.fraunhofer.de/">Fraunhofer FIRST</a>  
 */ 
class Properties : public std::map<std::string,std::string>
{
private:
	std::vector<XERCES_CPP_NAMESPACE::DOMElement*> dom;
	
public:
	/**
	 * Constructor for properties.
	 */
	Properties() {}
	
	/**
	 * Destructor for properties.
	 */
	virtual ~Properties() {clear(); dom.clear(); }
	
	/**
	 * Constructor from XML.
	 */
	explicit Properties(XERCES_CPP_NAMESPACE::DOMNodeList* list);
	
	/**
	 * Convert this into a DOMElement.
	 * @param doc The master document this elements should belong to.
	 * @return The DOMElement.
	 */
	std::vector<XERCES_CPP_NAMESPACE::DOMElement*>& toElements(XERCES_CPP_NAMESPACE::DOMDocument* doc);
	
	/**
	 * Put new name/value pair into properties.
	 * Overwrites old property with same name.
	 * @param name The name of the property.
	 * @param value The value of the property.
	 */
	void put(const std::string& name, const std::string& value)
	{
		// key not yet in map
		if (find(name)==end()) {
		    insert(std::pair<std::string,std::string>(name,value));		
		} 
		// key already in map, remove old value!
		else {
			erase(name);
		    insert(std::pair<std::string,std::string>(name,value));		
		}
    }
	 
	/**
	 * Remove property with specific name from properties.
	 * @param name The name of the property.
	 */
	void remove(const std::string& name)
	{
	  erase(name);
	}
	
	/**
	 * Get specific property value.
	 * @param name The name of the property.
	 * @return empty string "" if property with name not found.
	 */
	std::string get(const std::string& name)
	{
	  ITR_Properties it = find(name);
      return it != end() ? it->second : "";
	}

	/**
	 * Test if the properties contain specific property name.
	 * @param name The name of the property.
	 */
	bool contains(const std::string& name) {
	  ITR_Properties it = find(name);
	  return (it != end());
	}
	 
	/*
	 * Get number of properties.
	 */
	//int size(); map defines size()
};

}

std::ostream& operator<< (std::ostream &out, gwdl::Properties &props);

#endif /*PROPERTIES_H_*/
