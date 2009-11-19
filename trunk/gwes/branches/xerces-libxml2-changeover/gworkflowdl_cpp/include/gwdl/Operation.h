/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#ifndef OPERATION_H_
#define OPERATION_H_

// gwdl
#include <gwdl/Memory.h> // shared_ptr
#include <gwdl/OperationClass.h>
#include <gwdl/AbstractionLevel.h>
// std
#include <string>

namespace gwdl
{

/**
 * Transitions can be linked to operations which are represented by this class.
 * The operation can have a specific abstraction level:
 * <ul>
 * <li>Operation::RED = abstract empty operation with no additional information.
 * <li>Operation::YELLOW = operation contains an operation class that specifies the name of the functionality.
 * <li>Operation::BLUE = operation contains operation class and a set of candidates which fullfill the required functionality.
 * <li>Operation::GREEN = one of the candidates has been selected for execution.
 * </ul>
 * <p>Code Example:</p>
 * <pre>
 * // create red operation
 * Operation::ptr_t op = Operation::ptr_t(new Operation());
 * cout << *op << endl;
 * assert(op->getAbstractionLevel()==Operation::RED);
 *   
 * // create yellow operation
 * OperationClass::ptr_t opc = OperationClass::ptr_t(new OperationClass());
 * opc->setName("calculateEverything");
 * op->setOperationClass(opc);
 * cout << *op << endl;
 * assert(op->getAbstractionLevel()==Operation::YELLOW);
 *
 * // create blue operation  
 * PreStackProOperation* pspop1 = new PreStackProOperation();
 * pspop1->setOperationName("calculate1");
 * pspop1->setResourceName("big_machine");
 * pspop1->setQuality(0.9);
 * op->getOperationClass()->addOperationCandidate(pspop1);
 * PreStackProOperation* pspop2 = new PreStackProOperation();
 * pspop2->setOperationName("calculate2");
 * pspop2->setResourceName("big_machine");
 * op->getOperationClass()->addOperationCandidate(pspop2);
 * assert(op->getOperationClass()->getOperationCandidates().size()==2);
 * cout << *op << endl; 
 * assert(op->getAbstractionLevel()==Operation::BLUE);
 *  
 * // create green operation
 * pspop1->setSelected();
 * cout << *op << endl; 
 * assert(op->getAbstractionLevel()==Operation::GREEN);
 * 
 * // delete operation   
 * delete op;
 * </pre>
 * 
 * <p>Example XML:
 * <pre>
 * &lt;operation xmlns="http://www.gridworkflow.org/gworkflowdl"&gt;
 *  &lt;oc:operationClass xmlns:oc="http://www.gridworkflow.org/gworkflowdl/operationclass" name="calculateEverything"&gt;
 *   &lt;oc:preStackProOperation operationName="calculate1" quality="0.9" resourceName="big_machine" selected="true"/&gt;
 *   &lt;oc:preStackProOperation operationName="calculate2" resourceName="big_machine"/&gt;
 *  &lt;/oc:operationClass&gt;
 * &lt;/operation&gt;
 * </pre>
 * 
 * @version $Id$
 * @author Andreas Hoheisel and Helge Ros&eacute; &copy; 2008 <a href="http://www.first.fraunhofer.de/">Fraunhofer FIRST</a>  
 */
class Operation
{
	
private:
	OperationClass::ptr_t _operationClassP;
	
public:

    typedef gwdl::shared_ptr<Operation> ptr_t;
	
	/**
	 * Constructor for empty (red) operation.
	 */
	Operation();
	
	/**
	 * Desctructor for operation.
	 */
	~Operation();
	
	/**
	 * Set the operation class for this operation.
	 * (allocated OperationClass is deleted)
	 * @param oc The operation class assosiated with this operation.
	 */
    void setOperationClass(OperationClass::ptr_t& oc) {_operationClassP = oc;}

    /**
     * Get the operation class of this operation.
     * @return A shared pointer to the operation class.
     */
    OperationClass::ptr_t& getOperationClass() {return _operationClassP;}

    /**
     * Get the operation class of this operation read-only.
     * @return A shared pointer to the operation class.
     */
    const OperationClass::ptr_t& readOperationClass() const {return _operationClassP;}

    /**
     * Get level of abstraction.
     * Ranges from RED (very abstract; empty operation) to YELLOW (contains operation class) to BLUE (contains operation candidates)
     * to GREEN (one candidate is selected).
     * @return  color
     */
    AbstractionLevel::abstraction_t getAbstractionLevel() const;
};

}

#endif /*OPERATION_H_*/
