/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#ifndef WORKFLOWHANDLERTABLE_H_
#define WORKFLOWHANDLERTABLE_H_
//std
#include <string>
#include <map>
//gwes
#include <gwes/WorkflowHandler.h>
#include <gwes/NoSuchWorkflowException.h>

namespace gwes {

/**
 * The WorkflowHandlerTable holds all current available workflow handlers that are used by the GWES.
 * The first value of the map contains the workflow identifier, the second value the pointers to the workflow handlers.
 * @version $Id$
 * @author Andreas Hoheisel &copy; 2008 <a href="http://www.first.fraunhofer.de/">Fraunhofer FIRST</a>  
 */ 
class WorkflowHandlerTable : public std::map<std::string,WorkflowHandler*> {

public:
	/**
	 * Constructor.
	 */
	WorkflowHandlerTable();
	
	/**
	 * Destructor. Deletes also all workflow handlers that are inside the table. 
	 */
	virtual ~WorkflowHandlerTable();

	/**
	 * Put a pointer to a workflow handler using its identifier as key to the table.
	 * @return The identifier of this workflow handler.
	 */
	std::string put(WorkflowHandler* wfhP);
	
	/**
	 * Get specific workflow handler.
	 * @param id The id of the workflow handler.
	 * @return the pointer to the workflow handler.
	 */
	WorkflowHandler* get(const std::string& id) throw (NoSuchWorkflowException);
	
	/** 
	 * Delete the workflow handler and erase it from the table.
	 * @param id The identifier of the workflow to remove.
	 */
	void remove(const std::string& id);
	
};

}

#endif /*WORKFLOWHANDLERTABLE_H_*/
