/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
// gwes
#include <gwes/GWES.h>
#include <gwes/WorkflowHandler.h>
// std
#include <iostream>
#include <sstream>

XERCES_CPP_NAMESPACE_USE
using namespace std;
using namespace gwdl;
using namespace gwes;

namespace gwes
{

/**
 * Constructor for GWES.
 */
GWES::GWES()
{
	_sdpaHandler = NULL;
}

/**
 * Destructor for GWES
 */
GWES::~GWES()
{
}

Sdpa2Gwes::~Sdpa2Gwes()
{
}

/**
  * Initiates a Grid workflow from a string containing its GWorkflowDL representation with a certain userID.
  *
  * @param gworkflowdl The GWorkflowDL description of the workflow.
  * @param userID      The ID of the user who owns the workflow.
  * @return The reference to the workflow.
  */
Workflow& GWES::initiate(const string& gworkflowdl, const string& userId) throw(WorkflowFormatException,StateTransitionException) 
{
	cout << "gwes::GWES::initiate() ... " << endl;
	// deserialize string
    DOMElement* element = (XMLUtils::Instance()->deserialize(gworkflowdl))->getDocumentElement();
	// create new workflow object
	Workflow *workflow = new Workflow(element);
	WorkflowHandler* wfhP = new WorkflowHandler(this,workflow,userId);
	_wfht.put(wfhP);
	return *workflow;
}

/**
 * Initiates a Grid workflow.
 * @param workflow The reference of the workflow.
 * @return The unique workflow ID.
 */
string GWES::initiate(gwdl::Workflow& workflow, const string& userId) throw(StateTransitionException)
{
	cout << "gwes::GWES::initiate() ... " << endl;
	WorkflowHandler* wfhP = new WorkflowHandler(this,&workflow,userId);
	return _wfht.put(wfhP);
}

/**
 * Connect a communication channel to a specific workflow handler of this GWES.
 * The destination Observer of the channel will be set by the GWES. 
 * @param channel The communication channel containing the source Observer.
 * @param workflow A reference to the workflow.
 */ 
void GWES::connect(Channel* channel, gwdl::Workflow& workflow) {
	connect(channel, workflow.getID());
}

/**
 * Connect a communication channel to a specific workflow handler of this GWES.
 * The destination Observer of the channel will be set by the GWES. 
 * @param channel The communication channel containing the source Observer.
 * @param workflowId The identifier of the workflow.
 */ 
void GWES::connect(Channel* channel, const string& workflowId) {
	cout << "gwes::GWES::connect(" << workflowId << ") ... " << endl;
	_wfht.get(workflowId)->connect(channel);
}

/**
  * Start a certain workflow. 
  * The processing of the workflow is done asynchronously. The method does not block. Use getStatus() in order to 
  * poll for the status of the workflow, or attach an Observer.
  *
  * @param workflow The reference of the workflow to start.
  */
void GWES::start(gwdl::Workflow& workflow) throw(StateTransitionException,NoSuchWorkflowException) 
{
	start(workflow.getID());
}

/**
 * Start a certain workflow. 
 * The processing of the workflow is done asynchronously. The method does not block. Use getStatus() in order to 
 * poll for the status of the workflow.
 *
 * @param workflowId The identifier of the workflow.
 */
void GWES::start(const string& workflowId) throw(StateTransitionException,NoSuchWorkflowException) {
	cout << "gwes::GWES::start(" << workflowId << ") ... " << endl;
	WorkflowHandler* wfhP = _wfht.get(workflowId);
	wfhP->startWorkflow();
}
    
/**
 * Execute the whole workflow.
 * The processing of the workflow is done synchronously, i.e., the method blocks until the workflow COMPLETES or TERMINATES.
 * 
 * @param workflow The reference of the workflow to start.
 */
void GWES::execute(Workflow& workflow) throw(StateTransitionException, WorkflowFormatException) 
{
	execute(workflow.getID());
}	

/**
 * Execute the whole workflow.
 * The processing of the workflow is done synchronously, i.e., the method blocks until the workflow COMPLETES or TERMINATES.
 * 
 * @param workflowId The identifier of the workflow.
 */
void GWES::execute(const string& workflowId) throw(StateTransitionException, WorkflowFormatException) {
	cout << "gwes::GWES::execute(" << workflowId << ") ... " << endl;
	WorkflowHandler* wfhP = _wfht.get(workflowId);
	wfhP->executeWorkflow();
}

/**
 * Suspend a certain workflow.
 * This method returns after the workflow has been fully suspended. This may need some time.
 *
 * @param workflow The reference to the workflow.
 */
void GWES::suspend(Workflow& workflow) throw(StateTransitionException)
{
	suspend(workflow.getID());
}

/**
 * Suspend a certain workflow.
 * This method returns after the workflow has been fully suspended. This may need some time.
 *
 * @param workflowId The identifier of the workflow.
 */
void GWES::suspend(const string& workflowId) throw(StateTransitionException) {
	cout << "gwes::GWES::suspend(" << workflowId << ") ... " << endl;
	WorkflowHandler* wfhP = _wfht.get(workflowId);
	wfhP->suspendWorkflow();
}

/**
 * Resume a certain workflow that has been suspended before.
 *
 * @param workflow The reference to the workflow.
 */
void GWES::resume(Workflow& workflow) throw(StateTransitionException) 
{
	resume(workflow.getID());
}

/**
 * Resume a certain workflow that has been suspended before.
 *
 * @param workflowId The identifier of the workflow.
 */
void GWES::resume(const string& workflowId) throw(StateTransitionException) {
	cout << "gwes::GWES::resume(" << workflowId << ") ... " << endl;
	WorkflowHandler* wfhP = _wfht.get(workflowId);
	wfhP->resumeWorkflow();
}

/**
 * Abort a certain workflow.
 * If currently running activities can not be aborted, then this methods waits until they complete or terminate.
 * Aborted workflows will switch to the status "TERINATED".
 *
 * @param workflow The reference to the workflow.
 */
void GWES::abort(Workflow& workflow) throw(StateTransitionException)
{
	abort(workflow.getID());
}

/**
 * Abort a certain workflow.
 * If currently running activities can not be aborted, then this methods waits until they complete or terminate.
 * Aborted workflows will switch to the status "TERINATED".
 *
 * @param workflowId The identifier of the workflow.
 */
void GWES::abort(const string& workflowId) throw(StateTransitionException) {
	cout << "gwes::GWES::abort(" << workflowId << ") ... " << endl;
	WorkflowHandler* wfhP = _wfht.get(workflowId);
	wfhP->abortWorkflow();
}

/**
 * Get the current GWorkflowDL document of the workflow specified by its reference.
 *
 * @param workflow The reference to the workflow.
 * @return A reference to the current workflow description as string.
 */
string& GWES::getWorkflowDescription(Workflow& workflow)
{
	DOMDocument* docP = workflow.toDocument();
	string* strP = XMLUtils::Instance()->serialize(docP,true);
	return *strP;	
}

/**
 * Get the current GWorkflowDL document of the workflow specified by its reference.
 *
 * @param workflowId The identifier of the workflow.
 * @return A reference to the current workflow description as string.
 */
string& GWES::getWorkflowDescription(const string& workflowId) {
	cout << "gwes::GWES::getWorkflowDescription(" << workflowId << ") ... " << endl;
	WorkflowHandler* wfhP = _wfht.get(workflowId);
	gwdl::Workflow* wfP = wfhP->getWorkflow();
	return getWorkflowDescription(*wfP);
}

/**
 * Get the current status code of the workflow specified by its reference.
 * Valid codes are:
 * <code>
 * <li>STATUS_UNDEFINED = 0
 * <li>STATUS_INITIATED = 1
 * <li>STATUS_RUNNING = 2
 * <li>STATUS_SUSPENDED = 3
 * <li>STATUS_ACTIVE = 4
 * <li>STATUS_TERMINATED = 5
 * <li>STATUS_COMPLETED = 6
 * <li>STATUS_FAILED = 7
 * </code>
 *
 * @param workflow The reference to the workflow.
 * @return The current state of the workflow.
 */
unsigned int GWES::getStatus(Workflow& workflow)
{
	return getStatus(workflow.getID());
}

/**
 * Get the current status code of the workflow specified by its identifier.
 * @param workflowId The identifier of the workflow.
 * @return The current state of the workflow.
 */
unsigned int GWES::getStatus(const string& workflowId) {
	cout << "gwes::GWES::getStatus(" << workflowId << ") ... " << endl;
	WorkflowHandler* wfhP = _wfht.get(workflowId);
	return wfhP->getStatus();
}

/**
 * Get the current status string of the workflow specified by its reference.
 */
string GWES::getStatusAsString(gwdl::Workflow& workflow) {
	return getStatusAsString(workflow.getID());
}

/**
 * Get the current status string of the workflow specified by its identifier.
 */
string GWES::getStatusAsString(const string& workflowId) {
	cout << "gwes::GWES::getStatusAsString(" << workflowId << ") ... " << endl;
	WorkflowHandler* wfhP = _wfht.get(workflowId);
	return wfhP->getStatusAsString();
}

/**
 * Get the identifiers of all the workflows that are handled by this Grid Workflow Execution Service.
 * @return An array of strings with the workflowIDs.
 */
const vector<string>& GWES::getWorkflowIDs()
{
	_workflowIds.clear();
	for (WorkflowHandlerTable::iterator it=_wfht.begin(); it!=_wfht.end(); ++it) {
		_workflowIds.push_back(it->first);
	}
	return _workflowIds;		
}
    
/**
 * Get a reference to the workflow handler table with contains all workflow handlers of this GWES.
 */
WorkflowHandlerTable& GWES::getWorkflowHandlerTable() {
	return _wfht;
}

/**
 * Remove a specific workflow from gwes.
 */
void GWES::remove(Workflow& workflow) {
	remove(workflow.getID());
}

/**
 * Remove a specific workflow from gwes.
 * This method deletes the workflow handler but not the workflow itself.
 * @param workflowId The identifier of the workflow.
 */
void GWES::remove(const string& workflowId) {
	cout << "gwes::GWES::remove(" << workflowId << ") ... " << endl;
	_wfht.remove(workflowId);
}

///////////////////////////////////
// Interface Spda2Gwes           //
///////////////////////////////////

/**
 * Uses WorkflowHandlerTable to delegate method call to WorkflowHandler.
 */
void GWES::activityDispatched(const workflow_id_t &workflowId, const activity_id_t &activityId) throw (NoSuchWorkflowException,NoSuchActivityException) {
	_wfht.get(workflowId)->activityDispatched(activityId);
}

/**
 * Uses WorkflowHandlerTable to delegate method call to WorkflowHandler.
 */
void GWES::activityFailed(const workflow_id_t &workflowId, const activity_id_t &activityId, const parameter_list_t &output) throw (NoSuchWorkflowException,NoSuchActivityException) {
	// ToDo: throw NoSuch{Workflow|Activity}Exception?
	_wfht.get(workflowId)->activityFailed(activityId,output);
}

/**
 * Uses WorkflowHandlerTable to delegate method call to WorkflowHandler.
 */
void GWES::activityFinished(const workflow_id_t &workflowId, const activity_id_t &activityId, const parameter_list_t &output) throw (NoSuchWorkflowException,NoSuchActivityException) {
	// ToDo: throw NoSuch{Workflow|Activity}Exception?
	_wfht.get(workflowId)->activityFinished(activityId,output);
}

/**
 * Uses WorkflowHandlerTable to delegate method call to WorkflowHandler.
 */
void GWES::activityCanceled(const workflow_id_t &workflowId, const activity_id_t &activityId) throw (NoSuchWorkflowException,NoSuchActivityException) {
	// ToDo: throw NoSuch{Workflow|Activity}Exception?
	_wfht.get(workflowId)->activityCanceled(activityId);
}

/**
 * Register the SDPA handler. 
 */
void GWES::registerHandler(Gwes2Sdpa *sdpa) {
	_sdpaHandler = sdpa;
}

/**
 * Initiate and start a workflow.
 */
Sdpa2Gwes::workflow_id_t GWES::submitWorkflow(workflow_t &workflow) throw (WorkflowFormatException) {
	string workflowId = initiate(workflow,"sdpa");
	start(workflowId);
	return workflowId;
}

/**
 * Cancel a workflow.
 */
void GWES::cancelWorkflow(workflow_id_t &workflowId) throw (NoSuchWorkflowException) {
	abort(workflowId);
}

} // end namespace gwes
