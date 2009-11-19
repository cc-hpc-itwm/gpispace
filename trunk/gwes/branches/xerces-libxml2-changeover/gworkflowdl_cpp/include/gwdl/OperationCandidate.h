/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#ifndef OPERATIONCANDIDATE_H_
#define OPERATIONCANDIDATE_H_
//gwdl
#include <gwdl/Memory.h> // shared_ptr
#include <gwdl/AbstractionLevel.h>
//std
#include <string>

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
	
	long _id;
	long generateID() {static long counter = 0; return counter++;}
	bool _selected;
	std::string _type;
	std::string _operationName;
	std::string _resourceName;
	float _quality;
	
public:
	
    typedef gwdl::shared_ptr<OperationCandidate> ptr_t;
	
	/**
	 * Constructor.
	 */
	OperationCandidate();
	
	/**
	 * Destructor.
	 */
	virtual ~OperationCandidate() {}
	
	/**
	 * get the unique identifier of this operation candidate.
	 */
	long getID() const { return _id; }
	
	/**
	 * set selection state of this operation candidate.
	 * @param _selected Set to <code>true</code> if candidate has been selected. 
	 */
	void setSelected(bool selected) { _selected = selected; }
	
	/**
	 * set selection state to <code>true</code>
	 */
	void setSelected() { _selected = true; }
	
	/**
	 * Test if this candidate has been selected.
	 * @return <code>true</code> if candidate has been selected, <code>false</code> otherwise. 
	 */
	bool isSelected() const { return _selected; }
	
	/**
	 * Set operation type. The supported types depend on the implementation of the workflow handler.
	 */
	void setType(const std::string& type) { _type = type; }
	
	/**
	 * Get the operation type.
	 */
	const std::string& getType() const { return _type; }
	
	/**
	 * Set operation name.
	 */
	void setOperationName(const std::string& name) { _operationName = name; }
	
	/**
	 * Get the operation name.
	 */
	const std::string& getOperationName() const { return _operationName; }
	
	/**
	 * Set resource name.
	 */
	void setResourceName(const std::string& name) {_resourceName = name; }
	
	/**
	 * Get resource name.
	 */
	const std::string& getResourceName() const { return _resourceName; }
	
	/**
	 * Set quality of this candidate.
	 * @param _quality Quality as float value ranging from 0 to 1.
	 */
	void setQuality(float quality) { _quality = quality; }
	
	/**
	 * Get quality of this candidate.
	 * @return Quality as float value ranging from 0 to 1.
	 */
	float getQuality() const { return _quality; }
	
    /**
     * get abstraction degree of operation
     * @return  color
     */
	AbstractionLevel::abstraction_t getAbstractionLevel() const;		
    
}; // end class OperationCandidate

} // end namespace gwdl

#endif /*OPERATIONCANDIDATE_H_*/
