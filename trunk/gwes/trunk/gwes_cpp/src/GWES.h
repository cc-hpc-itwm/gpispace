/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#ifndef GWES_H_
#define GWES_H_
//std
#include <string>
#include <vector>
//gworkflowdl
#include "../../gworkflowdl_cpp/src/Workflow.h"
#include "../../gworkflowdl_cpp/src/Properties.h"
#include "../../gworkflowdl_cpp/src/WorkflowFormatException.h"
//gwes
#include "StateTransitionException.h"
#include "NoSuchWorkflowException.h"
#include "WorkflowHandlerTable.h"
#include "Observer.h"
#include "Channel.h"

namespace gwes
{

/**
 * The GWES is the main class of the gwes_cpp library which is used to invoke workflows regarding the GWorkflowDL syntax.
 * @version $Id$
 * @author Andreas Hoheisel &copy; 2008 <a href="http://www.first.fraunhofer.de/">Fraunhofer FIRST</a>  
 */ 
class GWES
{
	
private:

	 WorkflowHandlerTable _wfht;
	 
	 std::vector<std::string> _workflowIds;
	
public:
	
	/**
	 * Constructor for GWES.
	 */
	GWES();
	
	/**
	 * Destructor for GWES
	 */
	virtual ~GWES();
	
    /**
     * Initiates a Grid workflow from a string containing its GWorkflowDL representation with a certain userID.
     *
     * @param gworkflowdl The GWorkflowDL description of the workflow.
     * @param userId      The ID of the user who owns the workflow.
     * @return The reference to the workflow.
     */
    gwdl::Workflow& initiate(std::string gworkflowdl, std::string userId) throw(gwdl::WorkflowFormatException,StateTransitionException);

	/**
 	 * Initiates a Grid workflow.
 	 * Initiated workflows get a new workflow id which is returned by this method. 
 	 *
 	 * @param workflow The reference of the workflow.
     * @param userId The ID of the user who owns the workflow.
 	 * @return The unique workflow ID.
 	 */
    std::string initiate(gwdl::Workflow& workflow, std::string userId) throw(StateTransitionException);
    
    /**
     * Connect a communication channel to a specific workflow handler of this GWES.
     * The destination Observer of the channel will be set by the GWES. 
     * @param channel The communication channel containing the source Observer.
     * @param workflow A reference to the workflow.
     */ 
    void connect(Channel* channel, gwdl::Workflow& workflow);

    /**
     * Connect a communication channel to a specific workflow handler of this GWES.
     * The destination Observer of the channel will be set by the GWES. 
     * @param channel The communication channel containing the source Observer.
     * @param workflowId The identifier of the workflow.
     */ 
    void connect(Channel* channel, std::string workflowId);

    /**
     * Start a certain workflow. 
     * The processing of the workflow is done asynchronously. The method does not block. Use getStatus() in order to 
     * poll for the status of the workflow.
     *
     * @param workflow The reference of the workflow to start.
     */
    void start(gwdl::Workflow& workflow) throw(StateTransitionException,NoSuchWorkflowException);
    
    /**
     * Start a certain workflow. 
     * The processing of the workflow is done asynchronously. The method does not block. Use getStatus() in order to 
     * poll for the status of the workflow.
     *
     * @param workflowId The identifier of the workflow.
     */
    void start(std::string workflowId) throw(StateTransitionException,NoSuchWorkflowException);
    
    /**
     * Execute the whole workflow.
     * The processing of the workflow is done synchronously, i.e., the method blocks until the workflow COMPLETES or TERMINATES.
     * 
     * @param workflow The reference of the workflow to start.
     */
    void execute(gwdl::Workflow& workflow) throw(StateTransitionException);

    /**
     * Execute the whole workflow.
     * The processing of the workflow is done synchronously, i.e., the method blocks until the workflow COMPLETES or TERMINATES.
     * 
     * @param workflowId The identifier of the workflow.
     */
    void execute(std::string workflowId) throw(StateTransitionException);

    /**
     * Suspend a certain workflow.
     * This method returns after the workflow has been fully suspended. This may need some time.
     *
     * @param workflow The reference to the workflow.
     */
    void suspend(gwdl::Workflow& workflow) throw(StateTransitionException);

    /**
     * Suspend a certain workflow.
     * This method returns after the workflow has been fully suspended. This may need some time.
     *
     * @param workflowId The identifier of the workflow.
     */
    void suspend(std::string workflowId) throw(StateTransitionException);

    /**
     * Resume a certain workflow that has been suspended before.
     *
     * @param workflow The reference to the workflow.
     */
    void resume(gwdl::Workflow& workflow) throw(StateTransitionException);

    /**
     * Resume a certain workflow that has been suspended before.
     *
     * @param workflowId The identifier of the workflow.
     */
    void resume(std::string workflowId) throw(StateTransitionException);

    /**
     * Abort a certain workflow.
     * If currently running activities can not be aborted, then this methods waits until they complete or terminate.
     * Aborted workflows will switch to the status "TERINATED".
     *
     * @param workflow The reference to the workflow.
     */
    void abort(gwdl::Workflow& workflow) throw(StateTransitionException);

    /**
     * Abort a certain workflow.
     * If currently running activities can not be aborted, then this methods waits until they complete or terminate.
     * Aborted workflows will switch to the status "TERINATED".
     *
     * @param workflowId The identifier of the workflow.
     */
    void abort(std::string workflowId) throw(StateTransitionException);

    /**
     * Get the current GWorkflowDL document of the workflow specified by its reference.
     *
     * @param workflow The reference to the workflow.
     * @return A reference to the current workflow description as std::string.
     */
    std::string& getWorkflowDescription(gwdl::Workflow& workflow);

    /**
     * Get the current GWorkflowDL document of the workflow specified by its reference.
     *
     * @param workflowId The identifier of the workflow.
     * @return A reference to the current workflow description as string.
     */
    std::string& getWorkflowDescription(std::string workflowId);

    /**
     * Get the current status code of the workflow specified by its reference.
     * Valid codes are:
     * <pre>
     * STATUS_UNDEFINED = 0
     * STATUS_INITIATED = 1
     * STATUS_RUNNING = 2
     * STATUS_SUSPENDED = 3
     * STATUS_ACTIVE = 4
     * STATUS_TERMINATED = 5
     * STATUS_COMPLETED = 6
     * </pre>
     *
     * @param workflow The reference to the workflow.
     * @return The current state of the workflow.
     */
    unsigned int getStatus(gwdl::Workflow& workflow);

    /**
     * Get the current status code of the workflow specified by its identifier.
     * Valid codes are:
     * <pre>
     * STATUS_UNDEFINED = 0
     * STATUS_INITIATED = 1
     * STATUS_RUNNING = 2
     * STATUS_SUSPENDED = 3
     * STATUS_ACTIVE = 4
     * STATUS_TERMINATED = 5
     * STATUS_COMPLETED = 6
     * </pre>
     *
     * @param workflowId The identifier of the workflow.
     * @return The current state of the workflow.
     */
    unsigned int getStatus(std::string workflowId);

    /**
     * Get the current status string of the workflow specified by its reference.
     *
     * @param workflow The reference to the workflow.
     * @return The current state of the workflow as string.
     */
    std::string getStatusAsString(gwdl::Workflow& workflow);

    /**
     * Get the current status string of the workflow specified by its identifier.
     *
     * @param workflowId The identifier of the workflow.
     * @return The current state of the workflow as string.
     */
    std::string getStatusAsString(std::string workflowId);

    /**
     * Get the identifiers of all the workflows that are handled by this Grid Workflow Execution Service.
     * @return An array of strings with the workflowIDs.
     */
    std::vector<std::string>& getWorkflowIDs();
    
    /**
     * Get a reference to the workflow handler table with contains all workflow handlers of this GWES.
     */
    WorkflowHandlerTable& getWorkflowHandlerTable();

    /**
     * Remove a specific workflow from gwes.
     * @param workflow Reference to workflow.
     */
    void remove(gwdl::Workflow& workflow);

    /**
     * Remove a specific workflow from gwes.
     * @param workflowId The identifier of the workflow.
     */
    void remove(std::string workflowId);

};

}

#endif /*GWES_H_*/
