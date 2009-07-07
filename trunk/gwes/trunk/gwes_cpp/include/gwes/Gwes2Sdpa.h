#ifndef GWES2SDPA_H_
#define GWES2SDPA_H_
// gwes
#include <gwes/Activity.h>
// gwdl
#include <gwdl/Workflow.h>

namespace gwes
{

/**
 * Interface class for the communication from GWES to SDPA.
 */
class Gwes2Sdpa {

public:

	/**
	 * Type for referencing to a GWorkflowDL workflow object.
	 */
	typedef gwdl::Workflow workflow_t;
	
	/**
	 * Type for referencing to a SPDA activity.
	 * ToDo: replace by specific SPDA activity class. Meanwhile using gwes::Activity.
	 */
	typedef gwes::Activity activity_t;

	/**
	 * Virtual destructor because of virtual methods.
	 */
//	virtual ~Gwes2Sdpa();

	/**
	 * Submit a sub workflow to the SDPA.
	 * This method is to be called by the GWES in order to delegate
	 * the execution of sub workflows.
	 * The SDPA will use the callback handler Sdpa2Gwes in order
	 * to notify the GWES about status transitions.
	 */
	virtual void submitWorkflow(const workflow_t &workflow) = 0; 

	/**
	 * Submit an atomic activity to the SDPA.
	 * This method is to be called by the GWES in order to delegate
	 * the execution of activities.
	 * The SDPA will use the callback handler Sdpa2Gwes in order
	 * to notify the GWES about activity status transitions.
	 */
	virtual void submitActivity(const activity_t &activity) = 0; 

	/**
	 * Cancel a sub workflow that has previously been submitted to
	 * the SDPA. The parent job has to cancel all children.
	 */
	virtual void cancelWorkflow(const workflow_t &workflow) = 0;
	
	/**
	 * Cancel an atomic activity that has previously been submitted to
	 * the SDPA.
	 */
	virtual void cancelActivity(const activity_t &activity) = 0;

	/**
	 * Notify the SDPA that a workflow finished (state transition
	 * from running to finished).
	 */
	virtual void workflowFinished(const workflow_t &workflow) = 0;
	
	/**
	 * Notify the SDPA that a workflow failed (state transition
	 * from running to failed).
	 */
	virtual void workflowFailed(const workflow_t &workflow) = 0;
	
	/**
	 * Notify the SDPA that a workflow has been canceled (state
	 * transition from * to terminated.
	 */ 
	virtual void workflowCanceled(const workflow_t &workflow) = 0;
};

} // end gwes namespace 

#endif /*GWES2SDPA_H_*/
