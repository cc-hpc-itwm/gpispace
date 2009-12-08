/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#ifndef TRANSITION_H_
#define TRANSITION_H_
//gwdl
#include <gwdl/Memory.h> // shared_ptr
#include <gwdl/Edge.h>
#include <gwdl/Operation.h>
#include <gwdl/Properties.h>
//std
#include <vector>
#include <string>

namespace gwdl
{
	
typedef std::vector<Edge::ptr_t>::iterator ITR_Edges;

class Workflow;

/**
 * Transtions are represented as spares within a Petri net. 
 * Transitions modify the state of the net which is represented by means of the tokens.
 * Transitions can be connected with places either by read edges, input edges or output edges.
 * <p>Code Example:
 * <pre>
 * // create empty transition
 * Transition *t0 = Transition::ptr_t(new Transition(""));
 * cout << *t0 << endl;
 *   
 * // set description
 * t0->setDescription("This is the description of the transition"); 
 * assert(t0->getDescription()=="This is the description of the transition");
 *		
 * // Transition is connected to three places
 * Place *p0 = new Place("");
 * Place *p1 = new Place("");
 * Place *p2 = new Place("");
 * Edge *e0 = new Edge(Edge::SCOPE_INPUT, p0,"input0");
 * Edge *e1 = new Edge(Edge::SCOPE_INPUT, p1,"input1");
 * Edge *e2 = new Edge(Edge::SCOPE_OUTPUT, p2,"output");
 * t0->addEdge(e0);
 * t0->addEdge(e1);
 * t0->addEdge(e2);
 *
 * // add a property  
 * t0->putProperty("key1","value1");
 *
 * // add a condition  
 * t0->addCondition("true");
 * 
 * // this is still a control transition (without operation)
 * assert(t0->getAbstractionLevel()==Operation::BLACK);
 *
 * // link this transtion with an operation
 * Operation::ptr_t op = Operation::ptr_t(new Operation()); 
 * OperationClass::ptr_t opc = OperationClass::ptr_t(new OperationClass());
 * opc->setName("calculateEverything");
 * op->setOperationClass(opc);
 * t0->setOperation(op);
 * assert(t0->getAbstractionLevel()==Operation::YELLOW);
 *
 * PreStackProOperation* pspop1 = new PreStackProOperation();
 * pspop1->setOperationName("calculate1");
 * pspop1->setResourceName("big_machine");
 * pspop1->setQuality(0.9);
 * pspop1->setSelected();
 * t0->getOperation()->getOperationClass()->addOperationCandidate(pspop1);
 * assert(t0->getAbstractionLevel()==Operation::GREEN);
 * 
 * cout << *t0 << endl;
 *
 * // delete transition  		
 * delete t0;
 * </pre>
 * 
 * <p>XML Example:
 * <pre>
 * &lt;transition xmlns="http://www.gridworkflow.org/gworkflowdl" ID="t2"&gt;
 *  &lt;description&gt;This is the description of the transition&lt;/description&gt;
 *  &lt;property name="key1"&gt;value1&lt;/property&gt;
 *  &lt;readPlace edgeExpression="input0" placeID="p1"/&gt;
 *  &lt;inputPlace edgeExpression="input1" placeID="p2"/&gt;
 *  &lt;outputPlace edgeExpression="output" placeID="p3"/&gt;
 *  &lt;condition&gt;true&lt;/condition&gt;
 *  &lt;operation&gt;
 *    &lt;oc:operationClass xmlns:oc="http://www.gridworkflow.org/gworkflowdl/operationclass" name="calculateEverything"&gt;
 *      &lt;oc:preStackProOperation operationName="calculate1" quality="0.9" resourceName="big_machine" selected="true"/&gt;
 *    &lt;/oc:operationClass&gt;
 *  &lt;/operation&gt;
 * &lt;/transition&gt;
 * </pre>
 *  
 * @version $Id$
 * @author Andreas Hoheisel and Helge Ros&eacute; &copy; 2008 <a href="http://www.first.fraunhofer.de/">Fraunhofer FIRST</a>  
 *
 */  
class Transition
{
public:
	
    typedef gwdl::shared_ptr<Transition> ptr_t;

    enum TransitionStatus
    {
     STATUS_NONE=0, STATUS_IDLE, STATUS_ENABLED, STATUS_TRIGGERED, STATUS_FIRING
    };

private:

	std::string _id;
	TransitionStatus _status;
    std::string _description;
    Properties::ptr_t _propertiesP;
    Operation::ptr_t _operationP;
    std::vector<std::string> _conditions;
    std::vector<Edge::ptr_t> _readEdges;
    std::vector<Edge::ptr_t> _inEdges;
    std::vector<Edge::ptr_t> _writeEdges;
    std::vector<Edge::ptr_t> _outEdges;
	
	std::string generateID() const;
	
public:
	
	/**
	 * Construct empty transition with specific unique identifier.
	 * Note: The unique identifier can not be changed after creating the transition!
	 * @param id The unique identifier of this transition. If id is set to "", then an id will be generated automatically. 
	 */ 
	explicit Transition(const std::string& id);
	
	/**
	 * Destructor.
	 */
	virtual ~Transition();
	
    /**
     * get transition ID.
     * @return transition ID
     */
    const std::string& getID() const {return _id;}

    /**
     * set transition description.
     * @param _description AnalysisTransition Description
     */
    void setDescription(const std::string& description){_description = description;}

    /**
     * get transition description.
     * before setDescription getDescription returns DEFAULT_DESCRIPTION
     * @return description
     */
    const std::string& getDescription() const {return _description;}

	/**
	 * Set all the properties of this transition.
	 * @param props The properties object.
	 */
    void setProperties(Properties::ptr_t propertiesP) {_propertiesP = propertiesP;}

	/**
	 * Get a shared pointer to the properties of this transition.
	 * Returns NULL if there are no properties. 
	 * @return A shared pointer to the transition properties or NULL if there are no properties.
	 */
    Properties::ptr_t getProperties() {return _propertiesP;}

	/**
	 * Get a read-only shared pointer to the properties of this transition.
	 * Returns NULL if there are no properties. 
	 * @return A read-only shared pointer to the properties or NULL if there are no properties.
	 */
    const Properties::ptr_t readProperties() const {return _propertiesP;}
    
	/**
	 * Put new name/value pair into properties.
	 * Overwrites old property with same name. Generates new Property map if required. 
	 * @param name The name of the property.
	 * @param value The value of the property.
	 */
	void putProperty(const std::string& name, const std::string& value);
	
    /**
     * add an Edge.
     * @param edge Edge to be added
     */
    void addEdge(Edge::ptr_t edgeP);

    void removeReadEdge(int i) { (*(_readEdges.begin()+i)).reset();	_readEdges.erase(_readEdges.begin()+i);}

    void removeInEdge(int i) {(*(_inEdges.begin()+i)).reset(); _inEdges.erase(_inEdges.begin()+i);}

    void removeWriteEdge(int i) { (*(_writeEdges.begin()+i)).reset(); _writeEdges.erase(_writeEdges.begin()+i);}

    void removeOutEdge(int i) {(*(_outEdges.begin()+i)).reset(); _outEdges.erase(_inEdges.begin()+i);}

    /**
     * get array of read Edges.
     * @return read Edges
     */
    const std::vector<Edge::ptr_t>& getReadEdges() const {return _readEdges;}

    /**
     * get array of input Edges.
     * @return input Edges
     */
    const std::vector<Edge::ptr_t>& getInEdges() const {return _inEdges;}

    /**
     * get array of write Edges.
     * @return write Edges
     */
    const std::vector<Edge::ptr_t>& getWriteEdges() const {return _writeEdges;}

    /**
     * get array of output Edges.
     * @return output edges
     */
    const std::vector<Edge::ptr_t>& getOutEdges() const {return _outEdges;}

    /**
     * set transition's Operation.
     * (allocated operation is deleted)
     * @param _operation Operation to be set
     */
    void setOperation(Operation::ptr_t operationP) {_operationP = operationP;}

    /**
     * get Transitions Operation.
     * You may use this operation to modify the operation object.
     * @return operation Operation of the transition
     */
    Operation::ptr_t getOperation() { return _operationP;}

    /**
     * get a read-only shared pointer to transitions Operation.
     * @return operation Operation of the transition
     */
    const Operation::ptr_t readOperation() const { return _operationP;}

    /**
     * set transition's condition.
     * @param _conditions condition to be set
     */
    void setConditions(std::vector<std::string> conditions) {_conditions.clear();
    	_conditions.insert(_conditions.end(), conditions.begin(), conditions.end());}

    void addCondition(std::string condition) {_conditions.push_back(condition);}

    void removeCondition(int i) {_conditions.erase(_conditions.begin()+i);}

    /**
     * get transition's condition.
     * @return condition condition of the transition
     */
    const std::vector<std::string>& getConditions() const {return _conditions;}

    // advanced methods that make life easier

    /**
     * set transition's status.
     * @param _status transition status
     */
    void setStatus(const TransitionStatus& status) {_status = status;}

    /**
     * get status of the transition.
     * before setStatus getStatus returns DEFAULT_STATUS
     * @return status of Transition
     */
    const TransitionStatus& getStatus() const {return _status;}

    /**
     * remove read edge by place ID.
     * @param placeID Place ID of Edge to remove
     */
    void removeReadEdge(const std::string& placeID)
    {
      for(ITR_Edges it=_readEdges.begin(); it!=_readEdges.end(); ++it)
        if((*it)->getPlace()->getID() == placeID){(*it).reset(); _readEdges.erase(it); break;}
    }

    /**
     * remove input edge by place ID.
     * @param placeID Place ID of Edge to remove
     */
    void removeInEdge(const std::string& placeID)
    {
      for(ITR_Edges it=_inEdges.begin(); it!=_inEdges.end(); ++it)
        if((*it)->getPlace()->getID() == placeID){(*it).reset(); _inEdges.erase(it); break;}
    }

    /**
     * remove write edge by place ID.
     * @param placeID Place ID of Edge to remove
     */
    void removeWriteEdge(const std::string& placeID)
    {
      for(ITR_Edges it=_writeEdges.begin(); it!=_writeEdges.end(); ++it)
        if((*it)->getPlace()->getID() == placeID){(*it).reset(); _writeEdges.erase(it); break;}
    }

    /**
     * remove output edge by place ID.
     * @param placeID Place ID of Edge to remove
     */
    void removeOutEdge(const std::string& placeID)
    {
      for(ITR_Edges it=_outEdges.begin(); it!=_outEdges.end(); ++it)
        if((*it)->getPlace()->getID() == placeID){(*it).reset(); _outEdges.erase(it); break;}
    }

    /**
     * get read edge by place ID.
     * if no input Edge has a Place with ID then return null
     * @param placeID of Edge to get
     * @return read edge
     */
    Edge::ptr_t getReadEdge(const std::string& placeID)
    {
      for(ITR_Edges it=_readEdges.begin(); it!=_readEdges.end(); ++it)
        if((*it)->getPlace()->getID() == placeID){return *it;}
      return Edge::ptr_t();
    }

    /**
     * get input edge by place ID.
     * if no input Edge has a Place with ID then return null
     * @param placeID of Edge to get
     * @return input edge
     */
    Edge::ptr_t getInEdge(const std::string& placeID)
    {
      for(ITR_Edges it=_inEdges.begin(); it!=_inEdges.end(); ++it)
        if((*it)->getPlace()->getID() == placeID){return *it;}
      return Edge::ptr_t();
    }

    /**
     * get write edge by place ID.
     * if no input Edge has a Place with ID then return null
     * @param placeID of Edge to get
     * @return write edge
     */
    Edge::ptr_t getWriteEdge(const std::string& placeID)
    {
      for(ITR_Edges it=_writeEdges.begin(); it!=_writeEdges.end(); ++it)
        if((*it)->getPlace()->getID() == placeID){return *it;}
      return Edge::ptr_t();
    }

    /**
     * get output edge by place ID.
     * if no output Edge has a Place with ID then return null
     * @param placeID Place identifier
     * @return output edge
     */
    Edge::ptr_t getOutEdge(const std::string& placeID)
    {
      for(ITR_Edges it=_outEdges.begin(); it!=_outEdges.end(); ++it)
        if((*it)->getPlace()->getID() == placeID){return *it;}
      return Edge::ptr_t();
    }

    int getAbstractionLevel() const;

    /**
     * tests whether transition is isEnabled
     * @return   true if transition is isEnabled, false else
     */
    bool isEnabled();
    
}; // end class Transition

} // end namespace gwdl

#endif /*TRANSITION_H_*/
