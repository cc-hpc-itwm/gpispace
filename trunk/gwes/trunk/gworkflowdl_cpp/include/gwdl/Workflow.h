/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#ifndef WORKFLOW_H_
#define WORKFLOW_H_
// gwdl
#include <gwdl/Memory.h> // shared_ptr
#include <gwdl/IWorkflow.h>
#include <gwdl/Defines.h>
#include <gwdl/Place.h>
#include <gwdl/Transition.h>
#include <gwdl/Properties.h>
#include <gwdl/NoSuchWorkflowElement.h>
#include <gwdl/WorkflowFormatException.h>
// std
#include <map>
#include <vector>
#include <string>

namespace gwdl
{
	
/**
 * The Workflow is the main class of the GWorkflowDL-C++-API.
 * It represents a distributed algorithm by means of a Petri net. A Workflow contains Places (-> Place), which represent the
 * state and the data of the workflow and Transitions (-> Transition), which represent Operations and state transitions.
 * Places and Transitions can be connected by means of edges (-> Edge) (input, output ,read, and write edges).
 * <p>Code Example:
 * <pre>
 * 	// create empty workflow
 *	Workflow::ptr_t wf = Workflow::ptr_t(new Workflow("test-workflow"));
 *
 *	// add description
 *	wf->setDescription("This is the description of the workflow");
 *	assert(wf->getDescription()=="This is the description of the workflow") ;
 *
 *	// add properties
 *	wf->putProperty("b_name1","value1");	
 *	wf->putProperty("a_name2","value2");	
 *	assert(wf->getProperties()->get("b_name1")=="value1");
 *	
 *	// add places
 *	Place::ptr_t p0 = new Place("p0");
 *	Place::ptr_t p1 = new Place("p1");
 *	wf->addPlace(p0);
 *	wf->addPlace(p1);
 *	assert(wf->placeCount()==2);
 * 	
 *	// create transition
 *	Transition::ptr_t t0 = Transition::ptr_t(new Transition("t0"));
 *	
 *	// input edge from p0 to t0
 *	Edge::ptr_t arc0 = new Edge(Edge::SCOPE_INPUT, wf->getPlace("p0"));
 *	t0->addEdge(arc0);
 * 
 *	// output edge from t0 to p1
 *	Edge::ptr_t arc1 = new Edge(Edge::SCOPE_OUTPUT, wf->getPlace("p1"));
 *	t0->addEdge(arc1);
 *	
 *	// add  transition	
 *	wf->addTransition(t0); 
 *
 *	// transition is not enabled
 *	assert(wf->getTransition("t0")->isEnabled()==false);	
 *			
 *	// add token
 *	Token::ptr_t d0 = new Token(true);
 *	wf->getPlace("p0")->addToken(d0);
 *
 *	// transition is now enabled
 *	assert(wf->getTransition("t0")->isEnabled()==true);
 *	
 *	// add operation to transition
 *	Operation::ptr_t op = Operation::ptr_t(new Operation());
 *	wf->getTransition("t0")->setOperation(op);	
 *	OperationClass::ptr_t opc = OperationClass::ptr_t(new OperationClass());
 *	opc->setName("mean-value");
 *	wf->getTransition("t0")->getOperation()->setOperationClass(opc);
 *	OperationCandidate::ptr_t po = OperationCandidate::ptr_t(new OperationCandidate());
 *  po->setType("psp");
 *	po->setOperationName("alg-mean-value");
 *	po->setResourceName("phastgrid");
 *	po->setSelected(true);
 *	wf->getTransition("t0")->getOperation()->getOperationClass()->addOperationCandidate(po);
 *
 *	// print workflow to stdout	
 *	cout << *wf << endl;
 *	
 *	// safe workflow to file
 *	wf->saveToFile("/tmp/wf.xml");
 *
 *	// load workflow from file 
 *	Workflow::ptr_t wf2 = Workflow::ptr_t(new Workflow("/tmp/wf.xml");
 * 
 *	// delete workflows
 *	delete wf; 
 *  delete wf2;
 * </pre>
 * 
 * <p>XML Example:
 * <pre>
 * &lt;?xml version="1.0" encoding="UTF-16" standalone="no" ?&gt;
 * &lt;workflow xmlns="http://www.gridworkflow.org/gworkflowdl" ID="test_workflow" xmlns:xsd="http://www.w3.org/2001/XMLSchema" xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance" xsi:schemaLocation="http://www.gridworkflow.org/gworkflowdl http://www.gridworkflow.org/kwfgrid/src/xsd/gworkflowdl_2_0.xsd"&gt;
 *  &lt;description&gt;This is the description of the workflow&lt;/description&gt;
 *  &lt;property name="a_name2"&gt;value2&lt;/property&gt;
 *  &lt;property name="b_name1"&gt;value1&lt;/property&gt;
 *  &lt;place ID="p0"&gt;
 *   &lt;token&gt;
 *    &lt;control&gt;true&lt;/control&gt;
 *   &lt;/token&gt;
 *  &lt;/place&gt;
 *  &lt;place ID="p1"/&gt;
 *  &lt;transition ID="t0"&gt;
 *   &lt;inputPlace placeID="p0"/&gt;
 *   &lt;outputPlace placeID="p1"/&gt;
 *   &lt;operation&gt;
 *     &lt;oc:operationClass xmlns:oc="http://www.gridworkflow.org/gworkflowdl/operationclass" name="mean-value"&gt;
 *       &lt;oc:OperationCandidate type="psp" operationName="alg-mean-value" resourceName="phastgrid" selected="true"/&gt;
 *     &lt;/oc:operationClass&gt;
 *   &lt;/operation&gt;
 *  &lt;/transition&gt;
 * &lt;/workflow&gt;
 * </pre>
 * 
 * @version $Id$
 * @author Andreas Hoheisel and Helge Ros&eacute; &copy; 2008 <a href="http://www.first.fraunhofer.de/">Fraunhofer FIRST</a>  
 */
class Workflow : public IWorkflow
{

private:
    std::string _id;
    std::string _description;
    Properties::ptr_t _propertiesP;
    std::vector<Transition::ptr_t> _transitions;
    std::vector<Transition::ptr_t> _enabledTransitions;
    
    /**
     * Map of places.
     * Key is the place ID, the value contains a shared pointer to the place.
     */
    std::map<std::string, Place::ptr_t> _places;
    	
public:

	typedef gwdl::shared_ptr<Workflow> ptr_t;

    /**
	 * Constructor for empty workflow.
	 */
	explicit Workflow(const std::string& id);
	
	/**
	 * Destructor.
	 */
	virtual ~Workflow();
	
    /**
     * get workflow id.
     */
    const workflow_id_t & getID() const {return _id;}
    
    /**
     * set workflow id.
     */
    void setID(const workflow_id_t &id) {_id = id;}

    /**
     * Appends a place to the registered places.
     * Note that a copy of this place will added and not a reference or pointer!
     * @param place to add
     */
    void addPlace(Place::ptr_t placeP) {_places.insert(std::pair<std::string, Place::ptr_t>(placeP->getID(),placeP));}
    
    /**
     * Return the map of places of the workflow.
     * @return place map.
     */
    std::map<std::string, Place::ptr_t>& getPlaces() { return _places; }

    /**
     * Return a read-only map of places of the workflow.
     * @return place map.
     */
    const std::map<std::string, Place::ptr_t>& readPlaces() const { return _places; }

    virtual gwdl::workflow_result_t getResults() const;

    /**
     * Get the ith place.
     */
    Place::ptr_t getPlace(unsigned int i) throw (NoSuchWorkflowElement);
    
    /**
     * retrieve a place by its identifier ID.
     * @param id place ID
     * @return place  place to find
     */
    Place::ptr_t getPlace(const std::string& id) throw (NoSuchWorkflowElement);

    /**
     * Get the index of a specific place.
     * @param id The place ID.
     * @return The place index.
     */
    unsigned int getPlaceIndex(const std::string& id) throw (NoSuchWorkflowElement);

    /**
     * Remove the i-th place.
     * @param i The place index.
     */
    void removePlace(unsigned int i) throw (NoSuchWorkflowElement);
	
	/**
	 * Get the total number of places.
	 * @return The number of places as unsigned int.
	 */
    unsigned int placeCount() const {return _places.size();}

    /**
     * Add a transition to the workflow.
     * (allocated transition is deleted)
     * @param transition The transition to add.
     */
    void addTransition(Transition::ptr_t transitionP) {_transitions.push_back(transitionP);}

	/**
	 * Remove a transition specified by its index.
	 * (allocated transition is deleted)
	 * @param i The index of the transition.
	 */
    void removeTransition(unsigned int i) throw (NoSuchWorkflowElement);

    /**
     * Retrieve a transition by its identifier ID.
     * @param id The transition ID.
     * @return A reference to the transition.
     */
    Transition::ptr_t getTransition(const std::string& id) throw (NoSuchWorkflowElement);
    
    /**
     * Get the index of a specific transition.
     * @param id The transition ID.
     * @return The transition index.
     */
    unsigned int getTransitionIndex(const std::string& id) const throw (NoSuchWorkflowElement);

	/**
	 * Retieve a transition by its index.
	 * @param i The transition index.
	 * @return A reference to the transition.
	 */
    Transition::ptr_t getTransition(unsigned int i) throw (NoSuchWorkflowElement);

	/**
	 * Get the total number of transitions within this workflow.
	 * @return The number of transitions as unsigned int.
	 */ 
    unsigned int transitionCount() const {return _transitions.size();}

    /**
     * return the transitions of the workflow as a vector.
     * @return Reference to transition vector.
     */
    std::vector<Transition::ptr_t>& getTransitions() {return _transitions;}

    /**
     * return a read-only vector of the transitions of the workflow.
     * @return read-only reference to transition vector.
     */
    const std::vector<Transition::ptr_t>& readTransitions() const {return _transitions;}

    // advanced methods that make life easier

    /**
     * return the workflow's enabled transitions as vector.
     * @return vector of enabled transitions.
     */
    std::vector<Transition::ptr_t>& getEnabledTransitions();

    /**
     * Set the human-readable workflow description.
     * @param _description Workflow description.
     */
    void setDescription(const std::string& description) {_description = description;}

    /**
     * Get the human-readable workflow description.
     * @return description
     */
    const std::string& getDescription() const {return _description;}

	/**
	 * Get the properties of this workflow.
	 * You can use this method to modify properties.
	 * Returns NULL if there are no properties. 
	 * @return A shared pointer to the workflow properties or NULL if there are no properties.
	 */
    Properties::ptr_t getProperties() {return _propertiesP;}

	/**
	 * Get the properties of this workflow for read-only.
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
	 * Set all the properties of this workflow.
	 * @param _properties The workflow properties.
	 */
    void setProperties(Properties::ptr_t propertiesP){_propertiesP = propertiesP;}

}; // end class Workflow

} // end namespace gwdl

#endif /*WORKFLOW_H_*/
