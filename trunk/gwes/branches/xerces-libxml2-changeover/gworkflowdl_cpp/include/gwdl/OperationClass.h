/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#ifndef CLASSOPERATION_H_
#define CLASSOPERATION_H_
//gwdl
#include <gwdl/Memory.h> // shared_ptr
#include <gwdl/OperationCandidate.h>
#include <gwdl/AbstractionLevel.h>
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
    std::vector<OperationCandidate::ptr_t> _operationCandidates;
    
    /**
     * Name of this operation class.
     */
    std::string _name;
	
public:
	
    typedef gwdl::shared_ptr<OperationClass> ptr_t;

	/**
	 * Constructor for empty operation class.
	 */
	OperationClass(){}
	
	/**
	 * Destructor for operation class.
	 */
	virtual ~OperationClass();
	
    /**
     * Get level of abstraction.
     * Ranges from RED (very abstract; empty operation) to YELLOW (contains operation class) to BLUE (contains operation candidates)
     * to GREEN (one candidate is selected).
     * @return  color
     */
	AbstractionLevel::abstraction_t getAbstractionLevel() const;

	/**
	 * Get the name of this operation class.
	 */
	const std::string& getName() const {return _name;}

	/**
	 * Set the name of this operation class.
	 */
	void setName(const std::string& name) {_name = name;}

	/**
	 * Get the candidates of this operation class.
	 */    
    const std::vector<OperationCandidate::ptr_t>& getOperationCandidates() const;
    
    /**
     * Add an additional concrete operation to the candidates.
     * (allocated OperationCandidate is deleted)
     * @param operation A pointer to the concrete operation.
     */
    void addOperationCandidate(OperationCandidate::ptr_t ocandP)
    { _operationCandidates.push_back(ocandP); } 
    
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
    void removeOperationCandidate(OperationCandidate::ptr_t ocandP);
    
    /**
     * Remove all concrete operations from vector.
     * (allocated OperationCandidate is deleted)
     */
    void removeAllOperationCandidates();
	
}; // end class OperationClass

} // end namespace gwdl

#endif /*CLASSOPERATION_H_*/
