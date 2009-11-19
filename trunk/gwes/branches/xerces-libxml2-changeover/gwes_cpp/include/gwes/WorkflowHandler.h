/*
 * Copyright 2009 Fraunhofer Gesellschaft, Munich, Germany,
 * for its Fraunhofer Institute for Computer Architecture and Software
 * Technology (FIRST), Berlin, Germany 
 * All rights reserved. 
 */
#ifndef WORKFLOWHANDLER_H_
#define WORKFLOWHANDLER_H_
// gwes
#include <gwes/Types.h>
#include <gwes/ActivityTable.h>
#include <gwes/StateTransitionException.h>
#include <gwes/Channel.h>
#include <gwes/TransitionOccurrence.h>
//gwdl
#include <gwdl/Workflow.h>
#include <gwdl/WorkflowFormatException.h>
//fhglog
#include <fhglog/fhglog.hpp>
//std
#include <vector>
#include <pthread.h>

namespace gwes {

/**
 * Declare class GWES here because of cyclic dependencies of header files.
 * Real declaration refer to GWES.h!
 */
class GWES;

/**
 * This class includes the methods for interpreting and processing workflows. It is used by the GWES class.
 * @version $Id$
 * @author Andreas Hoheisel &copy; 2008 <a href="http://www.first.fraunhofer.de/">Fraunhofer FIRST</a>  
 */ 
class WorkflowHandler : public gwes::Observer
{
public:

	/**
	 * Status of workflow.
	 */
	enum status_t
	{

		/**
		 * Status <B>UNDEFINED = 0</B>. The status remains <CODE>UNDEFINED</CODE> from the construction of the workflow
		 * until the method <CODE>initiateWorkflow()</CODE> has been called.
		 */
		STATUS_UNDEFINED = 0,

		/**
		 * Status <B>INITIATED = 1</B>. The workflow has been created and initialized but not yet been started or aborted.
		 */
		STATUS_INITIATED = 1,

		/**
		 * Status <B>RUNNING = 2</B>. The workflow has been started but it is not yet active or complete and it has not
		 * been suspended or aborted.
		 */
		STATUS_RUNNING = 2,

		/**
		 * Status <B>SUSPENDED = 3</B>. The workflow is quiescent and can be returned
		 * to the running state via <CODE>resumeWorkflow()</CODE>.
		 */
		STATUS_SUSPENDED = 3,

		/**
		 * Status <B>ACTIVE = 4</B>. One ore more activity instances of this workflow are allocated.
		 */
		STATUS_ACTIVE = 4,

		/**
		 * Status <B>TERMINATED = 5</B>. The execution of this workflow instance has aborted before its normal completion.
		 * This can be due to an workflow exception (workflow failed) or via the method <CODE>abortWorkflow()</CODE>.
		 */
		STATUS_TERMINATED = 5,

		/**
		 * Status <B>COMPLETED = 6</B>. The workflow instance has fulfilled the conditions for completion. The workflow
		 * instance will be destroyed.
		 */
		STATUS_COMPLETED = 6,

		/**
		 * Status <B>FAILED = 7</B>. One or more activities have failed. They will be retried.
		 */
		STATUS_FAILED = 7
	};

	/**
	 * Constructor for workflow handler.
	 */
	explicit WorkflowHandler(GWES* gwesP, gwdl::Workflow::ptr_t workflowP, const std::string& userId);

	/**
	 * Destructor. Does NOT delete workflow.
	 */
	virtual ~WorkflowHandler();

	/**
	 * Get the identifier of the workflow handler.
	 */
	const std::string& getID() const {
		return _id;
	}

	/**
	 * Get the current status code of this workflow handler as int.
	 */
	status_t getStatus() const {
		return _status;
	}

	/**
	 * Get current status as string.
	 */
	std::string getStatusAsString() const {
		return getStatusAsString(_status);
	}

	/**
	 * Translate status code to string.
	 */
	static std::string getStatusAsString(status_t status) {
		switch (status) {
		case STATUS_UNDEFINED:
			return std::string("UNDEFINED");
		case STATUS_INITIATED:
			return std::string("INITIATED");
		case STATUS_RUNNING:
			return std::string("RUNNING");
		case STATUS_ACTIVE:
			return std::string("ACTIVE");
		case STATUS_SUSPENDED:
			return std::string("SUSPENDED");
		case STATUS_COMPLETED:
			return std::string("COMPLETED");
		case STATUS_FAILED:
			return std::string("FAILED");
		case STATUS_TERMINATED:
			return std::string("TERMINATED");
		}
		return std::string("UNKNOWN");
	}

	/**
	 * Generate a new activity identifier. This identifier consists of the UUID of the workflow a underscore (_) and
	 * a serial number with leading zeros (activity counter).
	 *
	 * @return The new activity identifier.
	 */
	std::string getNewActivityID() const;

	/**
	 * Start this workflow. This method call is asynchronous (non-blocking)
	 * and returns after the status changed to INITIATED.
	 * Status should switch to RUNNING.
	 */
	void startWorkflow() throw (StateTransitionException);

	/**
	 * Execute this workflow. This method call is synchronous (blocking).
	 * It returns after the workflow COMPLETES or TERMINATES.
	 * Status first switches to RUNNING.
	 */
	void executeWorkflow() throw (StateTransitionException, gwdl::WorkflowFormatException);

	/**
	 * Suspend this workflow. Status should switch to SUSPENDED.
	 */
	void suspendWorkflow() throw (StateTransitionException);

	/**
	 * Resume this workflow. Status should switch to RUNNING. 
	 */
	void resumeWorkflow() throw (StateTransitionException);

	/**
	 * Abort this workflow. Status should switch to TERMINATED.
	 */
	void abortWorkflow() throw (StateTransitionException);

	/**
	 * Wait for workflow to change its status.
	 * @param oldStatus The old status
	 * @return The new status
	 */
	status_t waitForStatusChangeFrom(status_t oldStatus);

	/**
	 * Wait for workflow to change its status to a specified status.
	 * @param newStatus The new status code to wait for
	 */
	void waitForStatusChangeTo(status_t newStatus);

	/**
	 * Wait for workflow to change its status to COMPLETED or TERMINATED.
	 */
	void waitForStatusChangeToCompletedOrTerminated();

	long getSleepTime() const {
		return _sleepTime;
	}

	/**
	 * Connect a communication channel to this workflow handler.
	 * @param channel The communication channel containing the source Observer.
	 */ 
	void connect(Channel* channel);

	/**
	 * Overides gwes::Observer::update().
	 * This method is called by the source of the channels connected to this workflow handler.
	 */
	virtual void update(const Event& event);

	/**
	 * Get the workflow which is handled by this WorkflowHandler.
	 * @return A pointer to the workflow.
	 */
	gwdl::Workflow::ptr_t getWorkflow();

	/**
	 * Get the parent GWES.
	 * @return A pointer to the parent GWES.
	 */
	GWES* getGWES() {
		return _gwesP;
	}

	/**
	 * Get the user Id of the user who owns this workflow.
	 * @return A pointer to the parent GWES.
	 */
	const std::string& getUserId() const {
		return _userId;
	}
	
    /////////////////////////////////////////
    // Delegation from Interface Spda2Gwes //
    /////////////////////////////////////////
    
	// transition from pending to running
	void activityDispatched(const activity_id_t &activityId) throw (NoSuchActivityException);

	// transition from running to failed     
	void activityFailed(const activity_id_t &activityId, const parameter_list_t &output) throw (NoSuchActivityException);

	// transition from running to finished
	void activityFinished(const activity_id_t &activityId, const parameter_list_t &output) throw (NoSuchActivityException);

	// transition from * to canceled
	void activityCanceled(const activity_id_t &activityId) throw (NoSuchActivityException);

    Activity *getActivity(const activity_id_t &activityId) throw (NoSuchActivityException);

private:

	/**
	 * Minimum time period in micro seconds for sleeping between two cycles if the workflow was not modified.
	 * Default is 10000 micro seconds
	 */
	const static long SLEEP_TIME_MIN=10000;

	/**
	 * Maximum time period in micro seconds for sleeping between two cycles if the workflow was not modified.
	 * Default is 60000000 micro seconds = 1Minute
	 */
	const static long SLEEP_TIME_MAX=60000000;
	
	/**
	 * Fhg Logger
	 */
	fhg::log::logger_t _logger;

	/**
	 * Current sleep time.
	 */
	long _sleepTime;

	/**
	 * Pointer to the workflow to handle.
	 */
	gwdl::Workflow::ptr_t _wfP;

	/**
	 * Pointer to the parent gwes.
	 */
	GWES* _gwesP;

	/**
	 * The start workflow thread.
	 */
	pthread_t _thread;

	/**
	 * The unique workflow id.
	 */
	std::string _id;

	/**
	 * The user who owns the workflow.
	 */
	std::string _userId;

	/**
	 * All current activities are stored in this table.
	 */
	ActivityTable _activityTable;

	/**
	 * The status of this workflow handler.
	 * Use setStatus() to change the status!
	 */
	status_t _status;

	/**
	 * Set _userabort to <code>true</code> if the execution of the workflow should be aborted because
	 * of an external request.
	 */
	bool _userabort;

	/**
	 * Set _systemabort to <code>true</code> if the execution of the workflow should be aborted because
	 * of an internal system event (failure).
	 */
	bool _systemabort;

	/**
	 * Set _suspend to <code>true</code> in order to suspend the execution of the workflow.
	 */
	bool _suspend;
	
	/**
	 * Set _simulation to <code>true</code> in order to switch on simulation of workflows.
	 * Use workflow property <property name="simulation">true</property> to set this flag. 
	 */
	bool _simulation;

	/**
	 * Vector which contains pointers to the communication channels of this workflow.
	 */
	std::vector<Channel*> _channels;

	/**
	 * Creates a new Universal Unique ID for a workflow instance.
	 * The UUID now consists of two parts:
	 * <ul>
	 * <li>the UserID of the user who instantiated the job</li>
	 * <li>an attached UUID</li>
	 * </ul>
	 * The two parts are connected by a underline (_).
	 */
	std::string generateID() const;

	/**
	 * Get a new unique error ID for this workflow.
	 * @return a string with a unique error ID.
	 */
	std::string createNewErrorID() const;

	/**
	 * Get a new unique warn ID for this workflow.
	 * @return a string with a unique warn ID.
	 */
	std::string createNewWarnID() const;

	/** 
	 * Set the status of this workflow handler.
	 */
	void setStatus(status_t status);

	/**
	 * Select one transition occurrence out of all enabled Transitions.
	 */
	TransitionOccurrence* selectTransitionOccurrence(
			std::vector<gwdl::Transition::ptr_t>& enabledTransitions,int step);

	/**
	 * Process red transition with undefined operation.
	 * ///ToDo: Not yet implemented! 
	 * @return <code>true</code> if it was possible to refine the workflow, <code>false</code> otherwise.
	 */
	bool processRedTransition(TransitionOccurrence* toP, int step);

	/**
	 * Process yellow transition with operation class.
	 * ///ToDo: Not yet implemented! 
	 * @return <code>true</code> if it was possible to refine the workflow, <code>false</code> otherwise.
	 */
	bool processYellowTransition(TransitionOccurrence* toP, int step);

	/**
	 * Process blue transition with operation candidates.
	 * ///ToDo: Not yet implemented! 
	 * @return <code>true</code> if it was possible to refine the workflow, <code>false</code> otherwise.
	 */
	bool processBlueTransition(TransitionOccurrence* toP, int step);

	/**
	 * Process green transition with selected concrete operation.
	 * @return <code>true</code> if it was possible to execute the operation , <code>false</code> otherwise.
	 */
	bool processGreenTransition(TransitionOccurrence* toP, int step);

	/**
	 * Process black transition with no operation.
	 * @return <code>true</code> if it was possible to execute the operation, <code>false</code> otherwise.
	 */
	bool processBlackTransition(TransitionOccurrence* toP, int step);

	/**
	 * Check all current activities of this workflow.
	 * As side effect this method removes the tokens from the input places of completed or terminated activities
	 * and puts new tokens to the output places.
	 * @param step The current step for logging purposes.
	 * @return <code>true</code> if the workflow was modified due to changes in the activity states.
	 */
	bool checkActivityStatus(int step) throw (ActivityException);

};

} // end namespace gwes

/**
 * This method is used by pthread_create in order to start this workflow in an own thread.
 * @param workflowHandlerP Pointer to the WorkflowHandler.
 */
void *startWorkflowAsThread(void *workflowHandlerP);

#endif /*WORKFLOWHANDLER_H_*/
