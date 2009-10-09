#ifndef SDPA2GWES_H_
#define SDPA2GWES_H_
// gwes
#include <gwes/Types.h> 
#include <gwes/NoSuchWorkflowException.h>
#include <gwes/NoSuchActivityException.h>
// gwdl
#include <gwdl/WorkflowFormatException.h>

#include <stdexcept>

namespace gwes
{

class Gwes2Sdpa;

/**
 * Interface class for the communication from SDPA to GWES.
 */
class Sdpa2Gwes {
	
public:
    
    // exceptions
    class Sdpa2GwesException : public std::runtime_error
    {
    public:
      explicit Sdpa2GwesException(const std::string &a_reason)
        : std::runtime_error(a_reason)
      {
      }
    };

    class NoSuchWorkflow : public Sdpa2GwesException
    {
    public:
      explicit NoSuchWorkflow(const gwdl::IWorkflow::workflow_id_t &wid)
        : Sdpa2GwesException(std::string("no such workflow: ") + wid) {}
    };

    class NoSuchActivity : public Sdpa2GwesException
    {
    public:
      explicit NoSuchActivity(const gwes::IActivity::activity_id_t &aid)
        : Sdpa2GwesException(std::string("no such activity: ") + aid) {}
    };

	// typedefs have been moved to Types.h

	/**
	 * Virtual destructor because of virtual methods.
	 */
	virtual ~Sdpa2Gwes();

	/**
	 * Notify the GWES that an activity has been dispatched
	 * (state transition from "pending" to "running").
	 * This method is to be invoked by the SDPA.
	 * This is a callback listener method to monitor activities submitted 
	 * to the SDPA using the method Gwes2Sdpa.submitActivity().
	 */
	virtual void activityDispatched(const workflow_id_t &workflowId
                                  , const activity_id_t &activityId) throw (NoSuchWorkflow,NoSuchActivity) = 0;

	/**
	 * Notify the GWES that an activity has failed
	 * (state transition from "running" to "failed").
	 * This method is to be invoked by the SDPA.
	 * This is a callback listener method to monitor activities submitted 
	 * to the SDPA using the method Gwes2Sdpa.submitActivity().
	 */
	virtual void activityFailed(const workflow_id_t &workflowId
                              , const activity_id_t &activityId
                              , const parameter_list_t &output) throw (NoSuchWorkflow,NoSuchActivity) = 0;

	/**
	 * Notify the GWES that an activity has finished
	 * (state transition from running to finished).
	 * This method is to be invoked by the SDPA.
	 * This is a callback listener method to monitor activities submitted 
	 * to the SDPA using the method Gwes2Sdpa.submitActivity().
	 */
	virtual void activityFinished(const workflow_id_t &workflowId
                                , const activity_id_t &activityId
                                , const parameter_list_t &output) throw (NoSuchWorkflow,NoSuchActivity) = 0;

	/**
	 * Notify the GWES that an activity has been canceled
	 * (state transition from * to terminated).
	 * This method is to be invoked by the SDPA.
	 * This is a callback listener method to monitor activities submitted 
	 * to the SDPA using the method Gwes2Sdpa.submitActivity().
	 */
	virtual void activityCanceled(const workflow_id_t &workflowId
                                , const activity_id_t &activityId) throw (NoSuchWorkflow,NoSuchActivity) = 0;

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
	 * UnRegister a SDPA handler that implements the Gwes2Sdpa
	 * interface. This handler is notified on each status
	 * transitions of each workflow. This handler is also used
	 * by the GWES to delegate the execution of activities or 
	 * sub workflows to the SDPA. 
	 * Currently you can only register ONE handler for a GWES.
	 */
	virtual void unregisterHandler(Gwes2Sdpa *sdpa) = 0;

	/**
	 * Submit a workflow to the GWES.
	 * This method is to be invoked by the SDPA.
	 * The GWES will initiate and start the workflow 
	 * asynchronously and notifiy the SPDA about status transitions
	 * using the callback methods of the Gwes2Sdpa handler.  
	 */
	virtual workflow_id_t submitWorkflow(workflow_t &workflow) throw (std::exception) = 0;

	/**
	 * Cancel a workflow asynchronously.
	 * This method is to be invoked by the SDPA.
	 * The GWES will notifiy the SPDA about the 
	 * completion of the cancelling process by calling the 
	 * callback method Gwes2Sdpa::workflowCanceled.  
	 */
	virtual void cancelWorkflow(const workflow_id_t &workflowId) throw (std::exception) = 0;
};

}

#endif /*SDPA2GWES_H_*/
