/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#ifndef WORKFLOW_H_
#define WORKFLOW_H_
// std
#include <map>
#include <vector>
#include <string>
using namespace std;
// xerces-c
#include <xercesc/dom/DOM.hpp>
XERCES_CPP_NAMESPACE_USE
// gwdl
#include <gwdl/Defines.h>
#include <gwdl/Place.h>
#include <gwdl/Transition.h>
#include <gwdl/Properties.h>
#include <gwdl/NoSuchWorkflowElement.h>

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
 *	Workflow *wf = new Workflow();
 *  wf->setID("test-workflow");
 *
 *	// add description
 *	wf->setDescription("This is the description of the workflow");
 *	assert(wf->getDescription()=="This is the description of the workflow") ;
 *
 *	// add properties
 *	wf->getProperties().put("b_name1","value1");	
 *	wf->getProperties().put("a_name2","value2");	
 *	assert(wf->getProperties().get("b_name1")=="value1");
 *	
 *	// add places
 *	Place* p0 = new Place("p0");
 *	Place* p1 = new Place("p1");
 *	wf->addPlace(p0);
 *	wf->addPlace(p1);
 *	assert(wf->placeCount()==2);
 * 	
 *	// create transition
 *	Transition* t0 = new Transition("t0");
 *	
 *	// input edge from p0 to t0
 *	Edge* arc0 = new Edge(wf->getPlace("p0"));
 *	t0->addInEdge(arc0);
 * 
 *	// output edge from t0 to p1
 *	Edge* arc1 = new Edge(wf->getPlace("p1"));
 *	t0->addOutEdge(arc1);
 *	
 *	// add  transition	
 *	wf->addTransition(t0); 
 *
 *	// transition is not enabled
 *	assert(wf->getTransition("t0")->isEnabled()==false);	
 *			
 *	// add token
 *	Token* d0 = new Token(true);
 *	wf->getPlace("p0")->addToken(d0);
 *
 *	// transition is now enabled
 *	assert(wf->getTransition("t0")->isEnabled()==true);
 *	
 *	// add operation to transition
 *	Operation* op = new Operation();
 *	wf->getTransition("t0")->setOperation(op);	
 *	OperationClass* opc = new OperationClass();
 *	opc->setName("mean-value");
 *	wf->getTransition("t0")->getOperation()->setOperationClass(opc);
 *	OperationCandidate* po = new OperationCandidate();
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
 *	Workflow* wf2 = new Workflow("/tmp/wf.xml");
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
class Workflow
{

private:
    string 					id;
    string 					description;
    Properties 				properties;
    vector<Transition*> 	transitions;
    vector<Transition*> 	enabledTransitions;
    map<string, Place*> 	places;
    vector<string>			placeids;
    vector<Place*> 			placeList;
    	
public:

    /**
	 * Constructor for empty workflow.
	 */
	Workflow();
	
	/**
	 * Construct workflow from DOMElement.
	 */
	Workflow(DOMElement* element);
	
	/**
	 * Construct workflow from file.
	 * @param filename The filename of the GWorkflowDL file including its path. 
	 */
	Workflow(string filename);

	/**
	 * Destructor.
	 */
	virtual ~Workflow();
	
	/**
	 * Convert this into a DOMDocument.
	 * @return A pointer on the DOMDocument.
	 */
	DOMDocument* toDocument();
	
	/**
	 * Save this workflow to an XML GWorkflowDL file
	 * @param filename The name of the file. 
	 */
	void saveToFile(string filename);
	
    /**
     * get workflow id.
     */
    string& getID() {return id;}
    
    /**
     * set workflow id.
     */
    void setID(string _id) {id=_id;}

    /**
     * Appends a place to the registered places.
     * Note that a copy of this place will added and not a reference or pointer!
     * @param place to add
     */
    void addPlace(Place* place)
      {places.insert(pair<string, Place*>(place->getID(),place));}
    
    /**
     * Get the ith place.
     */
    Place* getPlace(unsigned int i) throw (NoSuchWorkflowElement);
    
    /**
     *  get all place ids.
     */
    vector<string>& getPlaceIDs()
    {
      placeids.clear();
      for(map<string,Place*>::iterator it=places.begin(); it!=places.end(); ++it)
      {
        placeids.push_back(it->first);
      }
      return placeids;
    }

    /**
     * retrieve a place by its identifier ID.
     * @param id place ID
     * @return place  place to find
     */
    Place* getPlace(string id) throw (NoSuchWorkflowElement);

    /**
     * Get the index of a specific place.
     * @param id The place ID.
     * @return The place index.
     */
    unsigned int getPlaceIndex(string id) throw (NoSuchWorkflowElement);

    /**
     * Remove the i-th place.
     * @param i The place index.
     */
    void removePlace(unsigned int i) throw (NoSuchWorkflowElement);
	
	/**
	 * Get the total number of places.
	 * @return The number of places as unsigned int.
	 */
    unsigned int placeCount() {return places.size();}

    /**
     * Add a transition to the workflow.
     * (allocated transition is deleted)
     * @param transition The transition to add.
     */
    void addTransition(Transition* transition) {transitions.push_back(transition);}

	/**
	 * Remove a transition specified by its index.
	 * (allocated transition is deleted)
	 * @param i The index of the transition.
	 */
    void removeTransition(unsigned int i) throw (NoSuchWorkflowElement);

    /**
     * Get all transition identifiers.
     * @return vector of all transition identifiers.
     */
    vector<string>* getTransitionIDs()
    {
      vector<string>* v = new vector<string>();
      for(unsigned int i=0; i<transitions.size(); ++i)
      {
        v->push_back(transitions[i]->getID());
      }
      return v;
    }

    /**
     * Retrieve a transition by its identifier ID.
     * @param id The transition ID.
     * @return A reference to the transition.
     */
    Transition* getTransition(string id) throw (NoSuchWorkflowElement);
    
    /**
     * Get the index of a specific transition.
     * @param id The transition ID.
     * @return The transition index.
     */
    unsigned int getTransitionIndex(string id) throw (NoSuchWorkflowElement);

	/**
	 * Retieve a transition by its index.
	 * @param i The transition index.
	 * @return A reference to the transition.
	 */
    Transition* getTransition(unsigned int i) throw (NoSuchWorkflowElement);

	/**
	 * Get the total number of transitions within this workflow.
	 * @return The number of transitions as unsigned int.
	 */ 
    unsigned int transitionCount() {return transitions.size();}

    /**
     * Set the workflow description.
     * @param _description Workflow description.
     */
    void setDescription(string _description) {description = _description;}

    /**
     * Get the workflow description.
     * @return description
     */
    string& getDescription() {return description;}

	/**
	 * Get the properties of this workflow.
	 * @return A reference to the workflow properties.
	 */
    Properties& getProperties() {return properties;}

	/**
	 * Set all the properties of this workflow.
	 * @param _properties The workflow properties.
	 */
    void setProperties(Properties _properties){properties = _properties;}

    /**
     * clear workflow's transition container and add the transitions of the vector.
     * (allocated transition is deleted)
     * @param _transitions Transitions to be set
     */
    void setTransitions(vector<Transition*> _transitions);

    /**
     * return the transitions of the workflow as a vector.
     * @return Reference to transition vector.
     */
    vector<Transition*>& getTransitions() {return transitions;}

    /**
     * Clear workflow's place container and add the places of the vector.
     * @param _places vector.
     */
    void setPlaces(vector<Place*>& _places)
    {
     places.clear();
     for(unsigned int i=0; i<_places.size(); ++i)
      {
        places.insert(pair<string, Place*>(_places[i]->getID(),_places[i])); 
      }
    }

    /**
     * Return the places of the workflow as vector.
     * ()
     * @return place vector.
     */
    vector<Place*>& getPlaces()
    {
      placeList.clear();
      for(map<string,Place*>::iterator it=places.begin(); it!=places.end(); ++it)
      {
        placeList.push_back(it->second);
      }
      return placeList;
    }

    // advanced methods that make life easier

    /**
     * return the workflow's enabled transitions as vector.
     * @return vector of enabled transitions.
     */
    vector<Transition*>& getEnabledTransitions()
    {
      enabledTransitions.clear();
      for(vector<Transition*>::iterator it=transitions.begin(); it != transitions.end(); ++it)
      {
        if((*it)->isEnabled()) enabledTransitions.push_back(*it);
      }
      return enabledTransitions;
    }
};

}

ostream& operator<< (ostream &out, gwdl::Workflow &wf);

#endif /*WORKFLOW_H_*/
