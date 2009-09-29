#ifndef SDPA_2_GWES_HPP
#define SDPA_2_GWES_HPP 1

#include <string>
#include <sdpa/wf/types.hpp>
#include <sdpa/daemon/exceptions.hpp>
#include <sdpa/common.hpp>

namespace sdpa { namespace wf {

class Gwes2Sdpa;

/**
 * Interface class for the communication from SDPA to GWES.
 */
class Sdpa2Gwes {

public:

	typedef sdpa::shared_ptr<Sdpa2Gwes> ptr_t;
	/**
	 * Virtual destructor because of virtual methods.
	 */
	virtual ~Sdpa2Gwes() {}

	/**
	 * Notify the GWES that an activity has been dispatched
	 * (state transition from "pending" to "running").
	 * This method is to be invoked by the SDPA.
	 */
	virtual void activityDispatched(const activity_id_t &activityId) throw (sdpa::daemon::NoSuchActivityException) = 0;

	/**
	 * Notify the GWES that an activity has failed
	 * (state transition from "running" to "failed").
	 * This method is to be invoked by the SDPA.
	 */
	virtual void activityFailed(const activity_id_t &activityId, const parameter_list_t &output) throw (sdpa::daemon::NoSuchActivityException) = 0;

	/**
	 * Notify the GWES that an activity has finished
	 * (state transition from running to finished).
	 * This method is to be invoked by the SDPA.
	 */
	virtual void activityFinished(const activity_id_t &activityId, const parameter_list_t &output) throw (sdpa::daemon::NoSuchActivityException) = 0;

	/**
	 * Notify the GWES that an activity has been canceled
	 * (state transition from * to terminated).
	 * This method is to be invoked by the SDPA.
	 */
	virtual void activityCanceled(const activity_id_t &activityId) throw (sdpa::daemon::NoSuchActivityException) = 0;

	/**
	 * Register a SDPA handler that implements the Gwes2Sdpa
	 * interface. This handler is notified on each status
	 * transitions of each workflow. This handler is also used
	 * by the GWES to delegate the execution of activities or
	 * sub workflows to the SDPA.
	 * Currently you can only register ONE handler for a GWES.
	 */
	virtual void registerHandler(Gwes2Sdpa *sdpa) const = 0;

	/**
	 * Submit a workflow to the GWES.
	 * This method is to be invoked by the SDPA.
	 * The GWES will initiate and start the workflow
	 * asynchronously and notifiy the SPDA about status transitions
	 * using the callback methods of the Gwes2Sdpa handler.
	 */
	virtual workflow_id_t submitWorkflow(workflow_t &workflow) = 0; //throw (gwdl::WorkflowFormatException) = 0;

	/**
	 * Cancel a workflow asynchronously.
	 * This method is to be invoked by the SDPA.
	 * The GWES will notify the SPDA about the
	 * completion of the canceling process by calling the
	 * callback method Gwes2Sdpa::workflowCanceled.
	 */
	virtual void cancelWorkflow(const workflow_id_t &workflowId) throw (sdpa::daemon::NoSuchWorkflowException) = 0;
};

}}
#endif
