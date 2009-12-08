/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#ifndef DATA_H_
#define DATA_H_
// gwdl
#include <gwdl/Memory.h> // shared_ptr
#include <gwdl/WorkflowFormatException.h>
// std
#include <ostream>
#include <string>

namespace gwdl
{

/**
 * This class handles the data which can be hold by data tokens.
 * Code example:
 * <pre>
 *	Data* data = new Data("<x>6</x>");
 *  cout << *data << endl;
 * </pre>
 * 
 * @version $Id$
 * @author Andreas Hoheisel and Helge Ros&eacute; &copy; 2008 <a href="http://www.first.fraunhofer.de/">Fraunhofer FIRST</a>  
 */
class Data
{

public:

    typedef gwdl::shared_ptr<Data> ptr_t;
    
    /**
     * Internal content type of the Data object.
     */
    typedef std::string content_t;
	
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
	 * Construct data from xml string.
	 * @param xmlstring The xml string representing the data element.
	 */
	explicit Data(const std::string& xmlstring, const int type = TYPE_DATA) throw(WorkflowFormatException);

	/**
	 * Destructor for data element.
	 */
	virtual ~Data();
	
	/**
	 * Returns the content of the data element.
	 * @return The content as XML string.
	 */
	const content_t getContent() const {return _content;}
	
	/**
	 * Serialize data to string.
	 */
	const std::string serialize() const {return _content;}
	
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
	 * Make a deep copy of this Data object and return a shared pointer to the new Data.
	 * @return Shared pointer to the cloned Data object.
	 */ 
	ptr_t deepCopy() const;

private:
	int _type;
    content_t _content;
    
};

}
#endif /*DATA_H_*/
