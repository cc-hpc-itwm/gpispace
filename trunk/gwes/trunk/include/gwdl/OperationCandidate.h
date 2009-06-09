#ifndef OPERATIONCANDIDATE_H_
#define OPERATIONCANDIDATE_H_
//std
#include <string>
using namespace std;

//xerces-c
#include <iostream>
#include <gwdl/XMLUtils.h>
#include <xercesc/util/OutOfMemoryException.hpp>
#include <xercesc/dom/DOM.hpp>
XERCES_CPP_NAMESPACE_USE

#include <gwdl/Defines.h>

#define X(str) XMLString::transcode((const char*)& str)
#define S(str) XMLString::transcode(str)

namespace gwdl
{

/**
 * The operation candidate is an class that wrappes arbitrary operations.
 * 
 * @version $Id$
 * @author Andreas Hoheisel and Helge Ros&eacute; &copy; 2008 <a href="http://www.first.fraunhofer.de/">Fraunhofer FIRST</a>  
 */
class OperationCandidate
{
	
private:
	
	long id;
	long generateID() {static long counter = 0; return counter++;}
	bool selected;
	string type;
	string operationName;
	string resourceName;
	float quality;
	
public:
	
	/**
	 * Constructor.
	 */
	OperationCandidate();
	
	/**
	 * Constructor for operation candidate to be build from DOMElement.
	 */
	OperationCandidate(DOMElement::DOMElement* element); 
	
	/**
	 * Destructor.
	 */
	virtual ~OperationCandidate() {}
	
	/**
	 * get the unique identifier of this operation candidate.
	 */
	long getID() { return id; }
	
	/**
	 * set selection state of this operation candidate.
	 * @param _selected Set to <code>true</code> if candidate has been selected. 
	 */
	void setSelected(bool _selected) { selected = _selected; }
	
	/**
	 * set selection state to <code>true</code>
	 */
	void setSelected() { selected = true; }
	
	/**
	 * Test if this candidate has been selected.
	 * @return <code>true</code> if candidate has been selected, <code>false</code> otherwise. 
	 */
	bool isSelected() { return selected; }
	
	/**
	 * Set operation type. The supported types depend on the implementation of the workflow handler.
	 */
	void setType(string _type) { type = _type; }
	
	/**
	 * Get the operation type.
	 */
	string& getType() { return type; }
	
	/**
	 * Set operation name.
	 */
	void setOperationName(string name) { operationName = name; }
	
	/**
	 * Get the operation name.
	 */
	string& getOperationName() { return operationName; }
	
	/**
	 * Set resource name.
	 */
	void setResourceName(string name) {resourceName = name; }
	
	/**
	 * Get resource name.
	 */
	string& getResourceName() { return resourceName; }
	
	/**
	 * Set quality of this candidate.
	 * @param _quality Quality as float value ranging from 0 to 1.
	 */
	void setQuality(float _quality) { quality = _quality; }
	
	/**
	 * Get quality of this candidate.
	 * @return Quality as float value ranging from 0 to 1.
	 */
	float getQuality() { return quality; }
	
	/**
	 * Convert this into a DOMElement.
	 * @param doc The master document this element should belong to.
	 * @return The DOMElement.
	 */
	DOMElement* toElement(DOMDocument* doc);
	
    /**
     * get abstraction degree of operation
     * @return  color
     */
    int getAbstractionLevel();		
    
};

}

#endif /*OPERATIONCANDIDATE_H_*/
