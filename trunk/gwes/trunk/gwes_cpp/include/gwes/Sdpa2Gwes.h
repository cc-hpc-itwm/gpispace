#ifndef SDPA2GWES_H_
#define SDPA2GWES_H_
// gwdl
#include <gwdl/Workflow.h>
// std
#include <string>
#include <list>

namespace gwes
{

class Gwes2Sdpa;

/**
 * Interface class for the communication from SDPA to GWES.
 */
class Sdpa2Gwes {
	
public:
	
	/**
	 * Type for referencing to a GWorkflowDL workflow.
	 * @todo: It could be better to use a reference to the corresponding WorkflowHandler instead!
	 */
	typedef gwdl::Workflow workflow_t;
	
	/**
	 * Type of the workflow identifier.
	 */
	typedef std::string workflow_id_t;
	
	/**
	 * Type fo the activity identifier.
	 */
	typedef std::string activity_id_t;
	
	/**
	 * Type of input/output parameters.
	 */
	typedef std::string parameter_t;
	
	/**
	 * Type of tuple of parameters.
	 */
	typedef std::list<parameter_t> parameter_list_t;

	/**
	 * Virtual destructor because of virtual methods.
	 */
//	virtual ~Sdpa2Gwes();

	/**
	 * Notify the GWES that an activity has been dispatched
	 * (state transition from "pending" to "running").
	 * This method is to be invoked by the SDPA.
	 */
	virtual void activityDispatched(const workflow_id_t &workflowId,
			const activity_id_t &activityId) = 0;

	/**
	 * Notify the GWES that an activity has failed
	 * (state transition from "running" to "failed").
	 * This method is to be invoked by the SDPA.
	 */
	virtual void activityFailed(const workflow_id_t &workflowId,
			const activity_id_t &activityId,
			const parameter_list_t &output) = 0;

	/**
	 * Notify the GWES that an activity has finished
	 * (state transition from running to finished).
	 * This method is to be invoked by the SDPA.
	 */
	virtual void activityFinished(const workflow_id_t &workflowId,
			const activity_id_t &activityId,
			const parameter_list_t &output) = 0;

	/**
	 * Notify the GWES that an activity has been canceled
	 * (state transition from * to terminated).
	 * This method is to be invoked by the SDPA.
	 */
	virtual void activityCanceled(const workflow_id_t &workflowId,
			const activity_id_t &activityId) = 0;

	/**
	 * Register a SDPA handler that implements the Gwes2Sdpa
	 * interface. This handler is notified on each status
	 * transitions of each workflow. This handler is also used
	 * by the GWES to delegate the execution of activities or 
	 * sub workflows to the SDPA. 
	 * Currently you can only register ONE handler for a GWES.
	 */
	virtual void registerHandler(Gwes2Sdpa *sdpa) = 0;

	/**
	 * Submit a workflow to the GWES.
	 * This method is to be invoked by the SDPA.
	 * The GWES will initiate and start the workflow 
	 * asynchronously and notifiy the SPDA about status transitions
	 * using the callback methods of the Gwes2Sdpa handler.  
	 */
	virtual void submitWorkflow(workflow_t &workflow) = 0;

	/**
	 * Cancel a workflow asynchronously.
	 * This method is to be invoked by the SDPA.
	 * The GWES will notifiy the SPDA about the 
	 * completion of the cancelling process by calling the 
	 * callback method Gwes2Sdpa::workflowCanceled.  
	 */
	virtual void cancelWorkflow(workflow_t &workflow) = 0;

};

}

#endif /*SDPA2GWES_H_*/
