/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#ifndef DATA_H_
#define DATA_H_
// gwdl
#include <gwdl/WorkflowFormatException.h>
// xerces
#include <xercesc/dom/DOM.hpp>
// std
#include <ostream>
#include <string>

namespace gwdl
{

/**
 * This class handles the data which can be hold by data tokens.
 * Code example:
 * <pre>
 * 	string* str = new string("<data><x>6</x></data>");
 *	Data* data = new Data(*str);
 *  cout << *data << endl;
 * </pre>
 * 
 * @version $Id$
 * @author Andreas Hoheisel and Helge Ros&eacute; &copy; 2008 <a href="http://www.first.fraunhofer.de/">Fraunhofer FIRST</a>  
 */
class Data
{
private:
	int _type;
	XERCES_CPP_NAMESPACE::DOMElement* data;
    std::string* dataText;
	void trim(std::string& s);
	void setType();
    
public:
	
	enum {
		
		/**
		 * Fault data
		 */
		TYPE_FAULT = -255,
		
		/**
		 * Empty data element
		 */
		TYPE_EMPTY = -1,
		
		/**
		 * Data with generic type.
		 */
		TYPE_DATA = 0,
		
		/**
		 * Data of type file, e.g.,
		 * <pre>
		 * <data><file>file:///tmp/test.dat</file></data>
		 * </pre>
		 */
		TYPE_FILE = 1,
		
		/**
		 * Data of type volume, e.g.,
		 * <pre>
		 * <data><volume>THD_5</volume></data>
		 * </pre>
		 */
		TYPE_VOLUME = 2,
		
		/**
		 * Data of type parameter, e.g.,
		 * <pre>
		 * <data><parameter><input1>5</input1></parameter></data>
		 * </pre>
		 */
		TYPE_PARAMETER = 3
		
	};

	/** 
	 * Constructor for empty data element.
	 */
	Data();
	
	/**
	 * Destructor for data element.
	 */
	virtual ~Data();
	
	/**
	 * Construct data from DOMElement.
	 * @param element The DOM representation of this element.
	 */
	explicit Data(XERCES_CPP_NAMESPACE::DOMElement* element);
	
	/**
	 * Construct data from xml string.
	 * @param xmlstring The xml string representing the data element.
	 */
	explicit Data(const std::string& xmlstring) throw(WorkflowFormatException);
	
	/**
	 * Convert this into a DOMElement.
	 * @return The DOMElement.
	 */
	XERCES_CPP_NAMESPACE::DOMElement* toElement(XERCES_CPP_NAMESPACE::DOMDocument* doc);
	
	/**
	 * Returns the text content of the data element and its descendants.
	 * @return A string containing only the text inside the data XML.
	 */
	std::string* getText();
	
	/** 
	 * Convert the content of this data object into a xml string.
	 * @return The XML string.
	 */
	std::string* toString() const;
	
	/**
	 * Get the type of the data. Can be
	 * <ul>
	 * <li>TYPE_FAULT = -255</li>
	 * <li>TYPE_EMPTY = -1</li>
	 * <li>TYPE_DATA = 0</li>
	 * <li>TYPE_FILE = 1</li>
	 * <li>TYPE_VOLUME = 2</li>
	 * <li>TYPE_PARAMETER = 3</li>
	 * </ul>
	 */
	int getType() const {return _type;}
	
	/**
	 * Make a deep copy of this Data object and return a pointer to the new Data.
	 * Note: Use delete by yourself in order to remove the new Data from memory.
	 * @return Pointer to the cloned Data object.
	 */ 
	Data* deepCopy();

};

}

std::ostream& operator<< (std::ostream &out, gwdl::Data &data);

#endif /*DATA_H_*/
