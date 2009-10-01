#ifndef GWES_2_SDPA_HPP
#define GWES_2_SDPA_HPP 1

#include <string>
#include <sdpa/wf/types.hpp>
#include <sdpa/daemon/exceptions.hpp>

 //for the exceptions, should be changed -> put common exceptions into some shared folder

namespace sdpa { namespace wf {

/**
 * Interface class for the communication from GWES to SDPA.
 */
class Gwes2Sdpa {

public:
	/**
	 * Virtual destructor because of virtual methods.
	 */
	virtual ~Gwes2Sdpa() {}

    /**
	 *  Submit  a  generic  activity  (atomic   or   complex)   to   the   SDPA.
     *
     * This method is to be  called  by  the  GWES  in  order  to  delegate  the
     * execution of "activities".  The SDPA component  will  then  decide  where
     * to execute this activity (local or remote).
     *
     * @see Sdpa2Gwes::activityDispatched
     * @see Sdpa2Gwes::activityFailed
     * @see Sdpa2Gwes::activityFinished
     * @see Sdpa2Gwes::activityCanceled
     */
	virtual  activity_id_t  submitActivity(const  activity_t  &activity)  =   0;

	/**
	 * Cancel a previously submitted activity.
	 */
	virtual void cancelActivity(const activity_id_t &activityId)  throw (sdpa::daemon::NoSuchActivityException) = 0;

	/**
	 * Notify the SDPA that a workflow finished (state transition
	 * from running to finished).
     *
     * @see Sdpa2Gwes::submitWorkflow
	 */
	virtual void workflowFinished(const workflow_id_t &workflowId) throw (sdpa::daemon::NoSuchWorkflowException) = 0;

	/**
	 * Notify the SDPA that a workflow failed (state transition
	 * from running to failed).
     *
     * @see Sdpa2Gwes::submitWorkflow
	 */
	virtual void workflowFailed(const workflow_id_t &workflowId) throw (sdpa::daemon::NoSuchWorkflowException) = 0;

	/**
	 * Notify the SDPA that a workflow has been canceled (state
	 * transition from * to terminated.
     *
     * @see Sdpa2Gwes::submitWorkflow
	 */
	virtual void workflowCanceled(const workflow_id_t &workflowId) throw (sdpa::daemon::NoSuchWorkflowException) = 0;
};

}}
#endif
