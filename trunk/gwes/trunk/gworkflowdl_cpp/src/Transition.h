/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#ifndef TRANSITION_H_
#define TRANSITION_H_
//std
#include <vector>
#include <string>
using namespace std;
//xerces-c
#include <xercesc/dom/DOM.hpp>
XERCES_CPP_NAMESPACE_USE
//gwdl
#include <gwdl/Edge.h>
#include <gwdl/Operation.h>
#include <gwdl/Properties.h>

namespace gwdl
{
	
typedef vector<Edge*>::iterator ITR_Edges;

class Workflow;

/**
 * Transtions are represented as spares within a Petri net. 
 * Transitions modify the state of the net which is represented by means of the tokens.
 * Transitions can be connected with places either by read edges, input edges or output edges.
 * <p>Code Example:
 * <pre>
 * // create empty transition
 * Transition *t0 = new Transition("");
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
 * Edge *e0 = new Edge(p0,"input0");
 * Edge *e1 = new Edge(p1,"input1");
 * Edge *e2 = new Edge(p2,"output");
 * t0->addReadEdge(e0);
 * t0->addInEdge(e1);
 * t0->addOutEdge(e2);
 *
 * // add a property  
 * t0->getProperties().put("key1","value1");
 *
 * // add a condition  
 * t0->addCondition("true");
 * 
 * // this is still a control transition (without operation)
 * assert(t0->getAbstractionLevel()==Operation::BLACK);
 *
 * // link this transtion with an operation
 * Operation* op = new Operation(); 
 * OperationClass* opc = new OperationClass();
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

    enum TransitionStatus
    {
     STATUS_NONE=0, STATUS_IDLE, STATUS_ENABLED, STATUS_TRIGGERED, STATUS_FIRING
    };

private:

	string id;
	TransitionStatus status;
    string description;
    Properties properties;
    Operation* operation;
    vector<string> conditions;
    vector<Edge*> readEdges;
    vector<Edge*> inEdges;
    vector<Edge*> writeEdges;
    vector<Edge*> outEdges;
	
	string generateID();
	
public:
	
	/**
	 * Construct empty transition with specific unique identifier.
	 * Note: The unique identifier can not be changed after creating the transition!
	 * @param id The unique identifier of this transition. If id is set to "", then an id will be generated automatically. 
	 */ 
	Transition(string id);
	
	/**
	 * Construct transition from DOMElement.
	 */
	Transition(Workflow* wf, DOMElement* element);

	/**
	 * Destructor.
	 */
	virtual ~Transition();
	
	/**
	 * Convert this into a DOMElement.
	 * @param doc The master document this element should belong to.
	 * @return The DOMElement.
	 */
	DOMElement* toElement(DOMDocument* doc);
	
    /**
     * get transition ID.
     * @return transition ID
     */
    string& getID() {return id;}

    /**
     * set transition description.
     * @param _description AnalysisTransition Description
     */
    void setDescription(string _description){description = _description;}

    /**
     * get transition description.
     * before setDescription getDescription returns DEFAULT_DESCRIPTION
     * @return description
     */
    string& getDescription(){return description;}

	/**
	 * Set all the properties of this transition.
	 * @param props The properties object.
	 */
    void setProperties(Properties _properties) {properties = _properties;}

	/**
	 * Get a reference to the properties of this transition.
	 * @return A reference to the properties.
	 */
    Properties& getProperties() {return properties;}

    /**
     * add an read Edge.
     * @param edge read Edge to be added
     */
    void addReadEdge(Edge* p_edge) {readEdges.push_back(p_edge);}

    void removeReadEdge(int i)
    {
    	delete *(readEdges.begin()+i);
    	readEdges.erase(readEdges.begin()+i);
    }

    /**
     * add an input Edge.
     * @param edge input Edge to be added
     */
    void addInEdge(Edge* p_edge) {inEdges.push_back(p_edge);}

    void removeInEdge(int i) {delete *(inEdges.begin()+i); inEdges.erase(inEdges.begin()+i);}

    /**
     * add an write Edge.
     * @param edge write Edge to be added
     */
    void addWriteEdge(Edge* p_edge) {writeEdges.push_back(p_edge);}

    void removeWriteEdge(int i)
    {
    	delete *(writeEdges.begin()+i);
    	writeEdges.erase(writeEdges.begin()+i);
    }

    /**
     * add an output Edge.
     * @param edge output Edge to be added
     */
    void addOutEdge(Edge* p_edge) {outEdges.push_back(p_edge);}

    void removeOutEdge(int i) {delete *(outEdges.begin()+i); outEdges.erase(inEdges.begin()+i);}

    /**
     * clear read Edges and add array's Edges to read Edges.
     * @param edges array of Edges to set
     */
    void setReadEdges(vector<Edge*> edges){
    	for(ITR_Edges it=readEdges.begin(); it!=readEdges.end(); ++it) delete *it;
    	readEdges.clear();
    	readEdges.insert(readEdges.end(), edges.begin(), edges.end());}

    /**
     * get array of read Edges.
     * @return read Edges
     */
    vector<Edge*>& getReadEdges() {return readEdges;}

    /**
     * clear input Edges and add array's Edges to input Edges.
     * @param edges array of Edges to set
     */
    void setInEdges(vector<Edge*> edges){
    	for(ITR_Edges it=inEdges.begin(); it!=inEdges.end(); ++it) delete *it;
    	inEdges.clear();
    	inEdges.insert(inEdges.end(), edges.begin(), edges.end());}

    /**
     * get array of input Edges.
     * @return input Edges
     */
    vector<Edge*>& getInEdges() {return inEdges;}

    /**
     * clear write Edges and add array's Edges to write Edges.
     * @param edges array of Edges to set
     */
    void setWriteEdges(vector<Edge*> edges){
    	for(ITR_Edges it=writeEdges.begin(); it!=writeEdges.end(); ++it) delete *it;
    	writeEdges.clear();
    	writeEdges.insert(writeEdges.end(), edges.begin(), edges.end());}

    /**
     * get array of write Edges.
     * @return write Edges
     */
    vector<Edge*>& getWriteEdges() {return writeEdges;}

    /**
     * clear output Edges and add array's Edges to output Edges.
     * @param edges array of output Edges to be set
     */
    void setOutEdges(vector<Edge*> edges) {
    	for(ITR_Edges it=outEdges.begin(); it!=outEdges.end(); ++it) delete *it;
    	outEdges.clear();
    	outEdges.insert(outEdges.end(), edges.begin(), edges.end());}

    /**
     * get array of output Edges.
     * @return output edges
     */
    vector<Edge*>& getOutEdges() {return outEdges;}

    /**
     * set transition's Operation.
     * (allocated operation is deleted)
     * @param _operation Operation to be set
     */
    void setOperation(Operation* p_operation)
    {if(operation != NULL){ delete operation;} operation = p_operation;}

    /**
     * get Transitions Operation.
     * @return operation Operation of the transition
     */
    Operation* getOperation() { return operation;}

    /**
     * set transition's condition.
     * @param _conditions condition to be set
     */
    void setConditions(vector<string> _conditions) {conditions.clear();
    	conditions.insert(conditions.end(), _conditions.begin(), _conditions.end());}

    void addCondition(string condition) {conditions.push_back(condition);}

    void removeCondition(int i) {conditions.erase(conditions.begin()+i);}

    /**
     * get transition's condition.
     * @return condition condition of the transition
     */
    vector<string>& getConditions() {return conditions;}

    // advanced methods that make life easier

    /**
     * set transition's status.
     * @param _status transition status
     */
    void setStatus(TransitionStatus _status) {status = _status;}

    /**
     * get status of the transition.
     * before setStatus getStatus returns DEFAULT_STATUS
     * @return status of Transition
     */
    TransitionStatus getStatus() {return status;}

    /**
     * remove read edge by place ID.
     * @param placeID Place ID of Edge to remove
     */
    void removeReadEdge(string placeID)
    {
      for(ITR_Edges it=readEdges.begin(); it!=readEdges.end(); ++it)
        if((*it)->getPlace()->getID() == placeID){delete *it; readEdges.erase(it); break;}
    }

    /**
     * remove input edge by place ID.
     * @param placeID Place ID of Edge to remove
     */
    void removeInEdge(string placeID)
    {
      for(ITR_Edges it=inEdges.begin(); it!=inEdges.end(); ++it)
        if((*it)->getPlace()->getID() == placeID){delete *it; inEdges.erase(it); break;}
    }

    /**
     * remove write edge by place ID.
     * @param placeID Place ID of Edge to remove
     */
    void removeWriteEdge(string placeID)
    {
      for(ITR_Edges it=writeEdges.begin(); it!=writeEdges.end(); ++it)
        if((*it)->getPlace()->getID() == placeID){delete *it; writeEdges.erase(it); break;}
    }

    /**
     * remove output edge by place ID.
     * @param placeID Place ID of Edge to remove
     */
    void removeOutEdge(string placeID)
    {
      for(ITR_Edges it=outEdges.begin(); it!=outEdges.end(); ++it)
        if((*it)->getPlace()->getID() == placeID){delete *it; outEdges.erase(it); break;}
    }

    /**
     * get read edge by place ID.
     * if no input Edge has a Place with ID then return null
     * @param placeID of Edge to get
     * @return read edge
     */
    Edge* getReadEdge(string placeID)
    {
      for(ITR_Edges it=readEdges.begin(); it!=readEdges.end(); ++it)
        if((*it)->getPlace()->getID() == placeID){return *it;}
      return NULL;
    }

    /**
     * get input edge by place ID.
     * if no input Edge has a Place with ID then return null
     * @param placeID of Edge to get
     * @return input edge
     */
    Edge* getInEdge(string placeID)
    {
      for(ITR_Edges it=inEdges.begin(); it!=inEdges.end(); ++it)
        if((*it)->getPlace()->getID() == placeID){return *it;}
      return NULL;
    }

    /**
     * get write edge by place ID.
     * if no input Edge has a Place with ID then return null
     * @param placeID of Edge to get
     * @return write edge
     */
    Edge* getWriteEdge(string placeID)
    {
      for(ITR_Edges it=writeEdges.begin(); it!=writeEdges.end(); ++it)
        if((*it)->getPlace()->getID() == placeID){return *it;}
      return NULL;
    }

    /**
     * get output edge by place ID.
     * if no output Edge has a Place with ID then return null
     * @param placeID Place identifier
     * @return output edge
     */
    Edge* getOutEdge(string placeID)
    {
      for(ITR_Edges it=outEdges.begin(); it!=outEdges.end(); ++it)
        if((*it)->getPlace()->getID() == placeID){return *it;}
      return NULL;
    }

    int getAbstractionLevel();

    /**
     * tests wheather transition is isEnabled
     * @return   true if transition is isEnabled, false else
     */
    bool isEnabled();
    
};

}

ostream& operator<< (ostream &out, gwdl::Transition &trans);

#endif /*TRANSITION_H_*/
