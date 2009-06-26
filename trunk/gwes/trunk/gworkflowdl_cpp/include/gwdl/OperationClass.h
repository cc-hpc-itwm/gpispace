/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#ifndef CLASSOPERATION_H_
#define CLASSOPERATION_H_
//gwdl
#include <gwdl/OperationCandidate.h>
//xerces-c
#include <xercesc/dom/DOM.hpp>
//std
#include <vector>
#include <string>

namespace gwdl
{

/**
 * The operationClass element, which is represented by this class, wrappes a group of OperationCandidates with a common 
 * functionality. 
 * 
 * @version $Id$
 * @author Andreas Hoheisel and Helge Ros&eacute; &copy; 2008 <a href="http://www.first.fraunhofer.de/">Fraunhofer FIRST</a>  
 */
class OperationClass
{
private:
	
	/**
	 * Candidates for this operation class.
	 */
    std::vector<OperationCandidate*> operationCandidates;
    
    /**
     * Name of this operation class.
     */
    std::string name;
	
public:

	/**
	 * Constructor for empty operation class.
	 */
	OperationClass(){}
	
	/**
	 * Construct operation class from DOMElement.
	 */
	explicit OperationClass(XERCES_CPP_NAMESPACE::DOMElement* element);
	
	/**
	 * Destructor for operation class.
	 */
	virtual ~OperationClass();
	
	/**
	 * Convert this into a DOMElement.
	 * @param doc The master document this element should belong to.
	 * @return The DOMElement.
	 */
	XERCES_CPP_NAMESPACE::DOMElement* toElement(XERCES_CPP_NAMESPACE::DOMDocument* doc);
	
    /**
     * Get level of abstraction.
     * Ranges from RED (very abstract; empty operation) to YELLOW (contains operation class) to BLUE (contains operation candidates)
     * to GREEN (one candidate is selected).
     * @return  color
     */
    int getAbstractionLevel() const;

	/**
	 * Get the name of this operation class.
	 */
	const std::string& getName() const {return name;}

	/**
	 * Set the name of this operation class.
	 */
	void setName(const std::string& _name) {name = _name;}

	/**
	 * Get the candidates of this operation class.
	 */    
    const std::vector<OperationCandidate*>& getOperationCandidates() const;
    
    /**
     * Add an additional concrete operation to the candidates.
     * (allocated OperationCandidate is deleted)
     * @param operation A pointer to the concrete operation.
     */
    void addOperationCandidate(OperationCandidate* p_operation)
    { operationCandidates.push_back(p_operation); } 
    
    /**
     * Remove the i-th concrete operation from operations vector.
     * (allocated OperationCandidate is deleted)
     * @param i The index of the concrete operation.
     */ 
    void removeOperationCandidate(int i);
    
    /**
     * Remove the specified concrete operation from vector.
     * (allocated OperationCandidate is deleted)
     * @param oper A reference to the concrete operation to be removed.
     */
    void removeOperationCandidate(OperationCandidate& oper);
    
    /**
     * Remove all concrete operations from vector.
     * (allocated OperationCandidate is deleted)
     */
    void removeAllOperationCandidates();
	
};

}

#endif /*CLASSOPERATION_H_*/
