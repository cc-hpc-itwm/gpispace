/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
// gwes
#include <gwes/GWES.h>
#include <gwes/WorkflowHandler.h>
#include <gwes/Utils.h>
#include <gwdl/XMLUtils.h>
#include <gwdl/Workflow.h>

XERCES_CPP_NAMESPACE_USE
using namespace std;
using namespace gwdl;
using namespace gwes;

namespace gwes
{

  struct mutex_lock
  {
    mutex_lock(pthread_mutex_t &mutex)
      : mtx(mutex)
    {
      pthread_mutex_lock(&mtx);
    }
    ~mutex_lock()
    {
      pthread_mutex_unlock(&mtx);
    }

    pthread_mutex_t &mtx;
  };


/**
 * Constructor for GWES.
 */
GWES::GWES(const std::string &a_workflow_directory)
  : _sdpaHandler(NULL)
  , workflow_directory_(a_workflow_directory)
  , _logger(fhg::log::getLogger("gwes")) 
{
	Utils::setEnvironmentVariables();
    pthread_mutex_init(&monitor_lock_, NULL);
    if (workflow_directory_.empty())
    {
      workflow_directory_ = getenv("GWES_CPP_HOME");
      LOG(DEBUG, "falling back to GWES_CPP_HOME value for workflow-directory: " << workflow_directory_);
    }
    else
    {
      LOG(DEBUG, "looking for sub-workflows in: " << workflow_directory_);
    }
}

/**
 * Destructor for GWES
 */
GWES::~GWES()
{
  pthread_mutex_destroy(&monitor_lock_);
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
	LOG_DEBUG(_logger, "initiate() ... ");
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
	LOG_DEBUG(_logger, "initiate() ... ");
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
	LOG_DEBUG(_logger, "connect(" << workflowId << ") ... ");
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
	LOG_DEBUG(_logger, "start(" << workflowId << ") ... ");
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
	LOG_DEBUG(_logger, "execute(" << workflowId << ") ... ");
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
	LOG_DEBUG(_logger, "suspend(" << workflowId << ") ... ");
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
	LOG_DEBUG(_logger, "resume(" << workflowId << ") ... ");
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
	LOG_DEBUG(_logger, "abort(" << workflowId << ") ... ");
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
	LOG_DEBUG(_logger, "getWorkflowDescription(" << workflowId << ") ... ");
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
	LOG_DEBUG(_logger, "getStatus(" << workflowId << ") ... ");
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
	LOG_DEBUG(_logger, "getStatusAsString(" << workflowId << ") ... ");
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
	LOG_DEBUG(_logger, "remove(" << workflowId << ") ... ");
	_wfht.remove(workflowId);
}

///////////////////////////////////
// Interface Spda2Gwes           //
///////////////////////////////////

/**
 * Uses WorkflowHandlerTable to delegate method call to WorkflowHandler.
 */
void GWES::activityDispatched(const workflow_id_t &workflowId, const activity_id_t &activityId) throw (NoSuchWorkflow,NoSuchActivity) {
    mutex_lock lock(monitor_lock_);
	_wfht.get(workflowId)->activityDispatched(activityId);
}

/**
 * Uses WorkflowHandlerTable to delegate method call to WorkflowHandler.
 */
void GWES::activityFailed(const workflow_id_t &workflowId, const activity_id_t &activityId, const parameter_list_t &output) throw (NoSuchWorkflow,NoSuchActivity) {
    mutex_lock lock(monitor_lock_);
	_wfht.get(workflowId)->activityFailed(activityId,output);
}

/**
 * Uses WorkflowHandlerTable to delegate method call to WorkflowHandler.
 */
void GWES::activityFinished(const workflow_id_t &workflowId, const activity_id_t &activityId, const parameter_list_t &output) throw (NoSuchWorkflow,NoSuchActivity) {
    mutex_lock lock(monitor_lock_);
	_wfht.get(workflowId)->activityFinished(activityId,output);
}

/**
 * Uses WorkflowHandlerTable to delegate method call to WorkflowHandler.
 */
void GWES::activityCanceled(const workflow_id_t &workflowId, const activity_id_t &activityId) throw (NoSuchWorkflow,NoSuchActivity) {
    mutex_lock lock(monitor_lock_);
	_wfht.get(workflowId)->activityCanceled(activityId);
}

/**
 * Register the SDPA handler. 
 */
void GWES::registerHandler(Gwes2Sdpa *sdpa) {
    mutex_lock lock(monitor_lock_);
	_sdpaHandler = sdpa;
}

/**
 * UnRegister the SDPA handler. 
 */
void GWES::unregisterHandler(Gwes2Sdpa *sdpa) {
    mutex_lock lock(monitor_lock_);
    if (_sdpaHandler == sdpa)
    {
	  _sdpaHandler = NULL;
    }
    else
    {
	  LOG_ERROR(_logger, "tried to unregister a not-registered handler!");
    }
}

/**
 * Initiate and start a workflow.
 */
workflow_id_t GWES::submitWorkflow(workflow_t &workflow) throw (std::exception) { //(WorkflowFormatException) {
    mutex_lock lock(monitor_lock_);
	string workflowId = initiate(static_cast<Workflow&>(workflow),"sdpa");
	start(workflowId);
	return workflowId;
}

/**
 * Cancel a workflow.
 */
void GWES::cancelWorkflow(const workflow_id_t &workflowId) throw (NoSuchWorkflow) {
    mutex_lock lock(monitor_lock_);
    try
    {
  	abort(workflowId);
    }
    catch (const NoSuchWorkflowException &nswfe)
    {
      throw NoSuchWorkflow(workflowId);
    }
}

gwdl::IWorkflow *GWES::deserializeWorkflow(const std::string &bytes) throw (std::runtime_error)
{
    mutex_lock lock(monitor_lock_);
  XERCES_CPP_NAMESPACE::DOMElement* element = (gwdl::XMLUtils::Instance()->deserialize(bytes))->getDocumentElement();
  if (element)
  {
    return new gwdl::Workflow(element);
  }
  else
  {
    throw std::runtime_error("could not deserialize input string");
  }
}

std::string GWES::serializeWorkflow(const gwdl::IWorkflow &workflow) throw (std::runtime_error)
{
    mutex_lock lock(monitor_lock_);
  gwdl::Workflow &real_workflow = static_cast<gwdl::Workflow&>(const_cast<IWorkflow&>(workflow));
  std::ostringstream ostr;
  ostr << real_workflow;
  return ostr.str();
}

workflow_t &GWES::getWorkflow(const workflow_id_t &workflowId) throw (NoSuchWorkflow)
{
  return *_wfht.get(workflowId)->getWorkflow();
}

activity_t &GWES::getActivity(const workflow_id_t &workflowId, const activity_id_t &activityId) throw (NoSuchWorkflow, NoSuchActivity)
{
  return *_wfht.get(workflowId)->getActivity(activityId);
}

} // end namespace gwes
