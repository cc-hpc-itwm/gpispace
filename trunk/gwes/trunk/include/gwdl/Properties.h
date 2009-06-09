#ifndef PROPERTIES_H_
#define PROPERTIES_H_
#include <string>
#include <map>
#include <vector>

#include <xercesc/dom/DOM.hpp>

using namespace std;
XERCES_CPP_NAMESPACE_USE

namespace gwdl
{
	
typedef map<string, string>::iterator 	ITR_Properties;

/**
 * The Properties class is a map used to store properties of Workflow, Transition, Place, and Token.
 * The first part of the map are the property names, the second part the property values.
 *  
 * @version $Id$
 * @author Andreas Hoheisel and Helge Ros&eacute; &copy; 2008 <a href="http://www.first.fraunhofer.de/">Fraunhofer FIRST</a>  
 */ 
class Properties : public map<string,string>
{
private:
	vector<DOMElement*> dom;
	
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
	Properties(DOMNodeList* list);
	
	/**
	 * Convert this into a DOMElement.
	 * @param doc The master document this elements should belong to.
	 * @return The DOMElement.
	 */
	vector<DOMElement*>& toElements(DOMDocument* doc);
	
	/**
	 * Put new name/value pair into properties.
	 * Overwrites old property with same name.
	 * @param name The name of the property.
	 * @param value The value of the property.
	 */
	void put(string name, string value)
	{
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
	void remove(string name)
	{
	  erase(name);
	}
	
	/**
	 * Get specific property value.
	 * @param name The name of the property.
	 * @return empty string "" if property with name not found.
	 */
	string get(string name)
	{
	  ITR_Properties it = find(name);
      return it != end() ? it->second : "";
	}

	/**
	 * Test if the properties contain specific property name.
	 * @param name The name of the property.
	 */
	bool contains(string name) {
	  ITR_Properties it = find(name);
	  return (it != end());
	}
	 
	/*
	 * Get number of properties.
	 */
	//int size(); map defines size()
};

}

ostream& operator<< (ostream &out, gwdl::Properties &props);

#endif /*PROPERTIES_H_*/
