/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#ifndef OPERATION_H_
#define OPERATION_H_

#include <string>
using namespace std;

#include <xercesc/dom/DOM.hpp>
XERCES_CPP_NAMESPACE_USE

#include <gwdl/OperationClass.h>

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
 * Operation* op = new Operation();
 * cout << *op << endl;
 * assert(op->getAbstractionLevel()==Operation::RED);
 *   
 * // create yellow operation
 * OperationClass* opc = new OperationClass();
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
	OperationClass* operationClass;
	
public:

    enum
    {
    /**
     * no operation
     */
     BLACK = -1,
    /**
     * no Classoperation specified
     */
     RED,
    /**
     * WSOperationClass owl specified
     */
     YELLOW = 1,

    /**
     * List of WSOperations given, non (or more than one) selected
     */
     BLUE   = 2,

    /**
     * List given and one WSOperation selected
     */
     GREEN  = 3
    };

	/**
	 * Constructor for empty (red) operation.
	 */
	Operation();
	
	/**
	 * Construct Operation from DOMElement.
	 * @param element The DOMElement to build the operation from.
	 */
	Operation(DOMElement* element);
	
	/**
	 * Desctructor for operation.
	 */
	~Operation();
	
	/**
	 * Convert this into a DOMElement.
	 * @param doc The master document this element should belong to.
	 * @return The DOMElement.
	 */
	DOMElement* toElement(DOMDocument* doc);

	/**
	 * Set the operation class for this operation.
	 * (allocated OperationClass is deleted)
	 * @param oc The operation class assosiated with this operation.
	 */
    void setOperationClass(OperationClass* oc)
    {if(operationClass != NULL){delete operationClass;} operationClass = oc;}

    /**
     * Get the operation class of this operation.
     * @return A reference to the operation class.
     */
    OperationClass* getOperationClass() {return operationClass;}

    /**
     * Get level of abstraction.
     * Ranges from RED (very abstract; empty operation) to YELLOW (contains operation class) to BLUE (contains operation candidates)
     * to GREEN (one candidate is selected).
     * @return  color
     */
    int getAbstractionLevel();
};

}

ostream& operator<< (ostream &out, gwdl::Operation &operation);

#endif /*OPERATION_H_*/
