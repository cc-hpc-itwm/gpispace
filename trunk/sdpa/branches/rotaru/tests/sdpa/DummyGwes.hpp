#ifndef test_SDPA_2_GWES_HPP
#define test_SDPA_2_GWES_HPP 1

#include <cppunit/extensions/HelperMacros.h>
#include "sdpa/logging.hpp"
#include <sdpa/wf/Sdpa2Gwes.hpp>
#include <sdpa/wf/Gwes2Sdpa.hpp>
#include <sdpa/JobId.hpp>


using namespace sdpa::wf;

class DummyGwes : public Sdpa2Gwes {
private:
	SDPA_DECLARE_LOGGER();
public:
	DummyGwes() : SDPA_INIT_LOGGER("sdpa.tests.DummyGwes")
	{
		ptr_Gwes2SdpaHandler = NULL;
		SDPA_LOG_DEBUG("Dummy workflow engine created ...");
	}

	/**
	 * Notify the GWES that an activity has been dispatched
	 * (state transition from "pending" to "running").
	 * This method is to be invoked by the SDPA.
	 */
	void activityDispatched(const workflow_id_t &workflowId,
			const activity_id_t &activityId) throw (sdpa::daemon::NoSuchWorkflowException, sdpa::daemon::NoSuchActivityException)
     {
		SDPA_LOG_DEBUG("Called activityDispatched ...");
     }

	/**
	 * Notify the GWES that an activity has failed
	 * (state transition from "running" to "failed").
	 * This method is to be invoked by the SDPA.
	 */
	void activityFailed(const workflow_id_t &workflowId,
			const activity_id_t &activityId,
			const parameter_list_t &output) throw (sdpa::daemon::NoSuchWorkflowException, sdpa::daemon::NoSuchActivityException)
	{
		SDPA_LOG_DEBUG("Called activityFailed ...");
	}

	/**
	 * Notify the GWES that an activity has finished
	 * (state transition from running to finished).
	 * This method is to be invoked by the SDPA.
	 */
	void activityFinished(const workflow_id_t &workflowId,
			const activity_id_t &activityId,
			const parameter_list_t &output) throw (sdpa::daemon::NoSuchWorkflowException, sdpa::daemon::NoSuchActivityException)
	{
		SDPA_LOG_DEBUG("Called activityFinished ...");
	}

	/**
	 * Notify the GWES that an activity has been canceled
	 * (state transition from * to terminated).
	 * This method is to be invoked by the SDPA.
	 */
	void activityCanceled(const workflow_id_t &workflowId,
			const activity_id_t &activityId) throw (sdpa::daemon::NoSuchWorkflowException, sdpa::daemon::NoSuchActivityException)
    {
		SDPA_LOG_DEBUG("Called activityCanceled ...");
    }

	/**
	 * Register a SDPA handler that implements the Gwes2Sdpa
	 * interface. This handler is notified on each status
	 * transitions of each workflow. This handler is also used
	 * by the GWES to delegate the execution of activities or
	 * sub workflows to the SDPA.
	 * Currently you can only register ONE handler for a GWES.
	 */
	void registerHandler(Gwes2Sdpa *pSdpa) const
	{
		ptr_Gwes2SdpaHandler = pSdpa;
		SDPA_LOG_DEBUG("Called registerHandler ...");
	}

	/**
	 * Submit a workflow to the GWES.
	 * This method is to be invoked by the SDPA.
	 * The GWES will initiate and start the workflow
	 * asynchronously and notifiy the SPDA about status transitions
	 * using the callback methods of the Gwes2Sdpa handler.
	 */
	workflow_id_t submitWorkflow(workflow_t &workflow) //throw (gwdl::WorkflowFormatException) {}
	{
		// GWES is supposed to parse the workflow and generate a suite of
		// sub-workflows or activities that are sent to SDPA
		// GWES assigns an unique workflow_id which will be used as a job_id
		// on SDPA side
		SDPA_LOG_DEBUG("Called submitWorkflow ...");

		// Here, GWES is supposed to create new workflows ....

		sdpa::JobId job_id;
		workflow.setId(job_id.str());
		// for testing purposes, generate here a sub-workflow (the same)
		SDPA_LOG_DEBUG("Gwes calls submitWorkflow ...");

		if(ptr_Gwes2SdpaHandler)
		{
			//SDPA_LOG_DEBUG("Valid pointer to SDPA ...");
			ptr_Gwes2SdpaHandler->submitWorkflow(workflow);
		}
		else
			SDPA_LOG_ERROR("SDPA not registered. NULL pointer ...");

		return "0";
	}

	/**
	 * Cancel a workflow asynchronously.
	 * This method is to be invoked by the SDPA.
	 * The GWES will notifiy the SPDA about the
	 * completion of the cancelling process by calling the
	 * callback method Gwes2Sdpa::workflowCanceled.
	 */
	void cancelWorkflow(const workflow_id_t &workflowId) throw (sdpa::daemon::NoSuchWorkflowException)
	{
		SDPA_LOG_DEBUG("Called cancelWorkflow ...");
	}

private:
	mutable Gwes2Sdpa *ptr_Gwes2SdpaHandler;
};

#endif
