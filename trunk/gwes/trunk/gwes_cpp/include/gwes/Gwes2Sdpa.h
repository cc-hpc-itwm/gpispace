#ifndef GWES2SDPA_H_
#define GWES2SDPA_H_
// gwes
#include <gwes/Types.h> 
#include <gwes/NoSuchWorkflowException.h>
#include <gwes/NoSuchActivityException.h>

namespace gwes
{

/**
 * Interface class for the communication from GWES to SDPA.
 */
class Gwes2Sdpa {

public:

	// typedefs are declared in Types.h

	/**
	 * Virtual destructor because of virtual methods.
	 */
	virtual ~Gwes2Sdpa();

	/**
	 * Submit an atomic activity to the SDPA.
	 * This method is to be called by the GWES in order to delegate
	 * the execution of activities. An activity may refer to the 
	 * execution of a sub workflow.
	 * The SDPA will use the callback handler Sdpa2Gwes in order
	 * to notify the GWES about activity status transitions.
	 */
	virtual activity_id_t submitActivity(activity_t &activity) = 0; 

	/**
	 * Cancel an atomic activity that has previously been submitted to
	 * the SDPA using submitActivity.
	 */
	virtual void cancelActivity(const activity_id_t &activityId)  throw (NoSuchActivityException) = 0;

	/**
	 * Notify the SDPA that a workflow finished (state transition
	 * from running to finished). 
	 * This is a callback listener method to monitor workflows submitted 
	 * to the GWES using the method Spda2Gwes.submitWorkflow().
	 */
	virtual void workflowFinished(const workflow_id_t &workflowId) throw (NoSuchWorkflowException) = 0;
	
	/**
	 * Notify the SDPA that a workflow failed (state transition
	 * from running to failed).
	 * This is a callback listener method to monitor workflows submitted 
	 * to the GWES using the method Spda2Gwes.submitWorkflow().
	 */
	virtual void workflowFailed(const workflow_id_t &workflowId) throw (NoSuchWorkflowException) = 0;
	
	/**
	 * Notify the SDPA that a workflow has been canceled (state
	 * transition from * to terminated.
	 * This is a callback listener method to monitor workflows submitted 
	 * to the GWES using the method Spda2Gwes.submitWorkflow().
	 */ 
	virtual void workflowCanceled(const workflow_id_t &workflowId) throw (NoSuchWorkflowException) = 0;
};

} // end gwes namespace 

#endif /*GWES2SDPA_H_*/
