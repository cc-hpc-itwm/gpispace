#ifndef CLASSOPERATION_H_
#define CLASSOPERATION_H_
//std
#include <vector>
#include <string>
using namespace std;
//xerces-c
#include <xercesc/dom/DOM.hpp>
XERCES_CPP_NAMESPACE_USE
//gwdl
#include <gwdl/OperationCandidate.h>

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
    vector<OperationCandidate*> operationCandidates;
    
    /**
     * Name of this operation class.
     */
    string name;
	
public:

	/**
	 * Constructor for empty operation class.
	 */
	OperationClass(){}
	
	/**
	 * Construct operation class from DOMElement.
	 */
	OperationClass(DOMElement* element);
	
	/**
	 * Destructor for operation class.
	 */
	virtual ~OperationClass();
	
	/**
	 * Convert this into a DOMElement.
	 * @param doc The master document this element should belong to.
	 * @return The DOMElement.
	 */
	DOMElement* toElement(DOMDocument* doc);
	
    /**
     * Get level of abstraction.
     * Ranges from RED (very abstract; empty operation) to YELLOW (contains operation class) to BLUE (contains operation candidates)
     * to GREEN (one candidate is selected).
     * @return  color
     */
    int getAbstractionLevel();

	/**
	 * Get the name of this operation class.
	 */
	string& getName() {return name;}

	/**
	 * Set the name of this operation class.
	 */
	void setName(string _name) {name = _name;}

	/**
	 * Get the candidates of this operation class.
	 */    
    vector<OperationCandidate*>& getOperationCandidates();
    
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
