#ifndef ACTIVITY_H_
#define ACTIVITY_H_
//std
#include <string>
#include <map>
#include <vector>
//gwdl
#include "../../gworkflowdl_cpp/src/Data.h"
#include "../../gworkflowdl_cpp/src/OperationCandidate.h"
//gwes
#include "ActivityException.h"
#include "StateTransitionException.h"
#include "Observer.h"
#include "Event.h"

namespace gwes {

/**
 * Declare class WorkflowHandler here because of cyclic dependencies of header files.
 * Real declaration refer to WorkflowHandler.h!
 */
class WorkflowHandler;

/**
 * Abstract Workflow Activity.
 * @version $Id$
 * @author Andreas Hoheisel &copy; 2008 <a href="http://www.first.fraunhofer.de/">Fraunhofer FIRST</a>  
 */
class Activity {

protected:
	
	std::string _activityImpl;
	std::string _id;
	int _status;
	gwdl::OperationCandidate* _operation;
	
	/**
	 * Vector which contains pointers to the observers of this activity.
	 */
	std::vector<Observer*> _observers;
	
	/**
	 * Hashmap of input data for this activity.
     * The key of the hash map is the edge expression related to the data.
     * The value contains the data of the parameter, e.g. as pointer to the gwdl::Data object.
	 */
	std::map<std::string,gwdl::Data*> _inputs;
	
	/**
	 * Hashmap of output data for this activity.
     * The key of the hash map is the edge expression related to the data.
     * The value contains the data of the parameter, e.g. as pointer to the gwdl::Data object.
	 */
	std::map<std::string,gwdl::Data*> _outputs;

	std::string _faultMessage;
	int _exitCode;
	WorkflowHandler* _wfhP;
	bool _abort;
	bool _suspend;

	long generateID() {
		static long counter = 0;
		return counter++;
	}
	
	void notifyObservers(int type=Event::EVENT_ACTIVITY, std::string message="", std::map<std::string,gwdl::Data*>* data=NULL);

public:

	enum
	{
		/**
		 * Status <B>UNDEFINED = 0</B>. The status remains <CODE>UNDEFINED</CODE> from the construction of the activity
		 * until the method <CODE>initiateActivity()</CODE> has been called.
		 */
		STATUS_UNDEFINED = 0,

		/**
		 * Status <B>INITIATED = 1</B>. The activity has been created and initialized but not yet been started or aborted.
		 */
		STATUS_INITIATED = 1,

		/**
		 * Status <B>RUNNING = 2</B>. The activity has been started but it is not yet active or complete and it has not
		 * been suspended or aborted.
		 */
		STATUS_RUNNING = 2,

		/**
		 * Status <B>SUSPENDED = 3</B>. The activity is quiescent and can be returned
		 * to the running state via <CODE>resumeActivity()</CODE>.
		 */
		STATUS_SUSPENDED = 3,

		/**
		 * Status <B>ACTIVE = 4</B>. Resources related to this activity have been allocated and/or sub activities have
		 * been started.
		 */
		STATUS_ACTIVE = 4,

		/**
		 * Status <B>TERMINATED = 5</B>. The execution of this activity instance has aborted before its normal completion.
		 * This can be due to an activity exception (activity failed) or via the method <CODE>abortActivity()</CODE>.
		 */
		STATUS_TERMINATED = 5,

		/**
		 * Status <B>COMPLETED = 6</B>. The activity instance has fulfilled the conditions for completion. The activity
		 * instance will be destroyed.
		 */
		STATUS_COMPLETED = 6,

		/**
		 * Status <B>FAILED = 7</B>. Activity has failed. It will be retried.
		 */
		STATUS_FAILED = 7
	};

	/**
	 * Constructor.
	 */
	Activity(WorkflowHandler* handler, std::string activityImpl, gwdl::OperationCandidate* operation);
	
	/**
	 * Destructor.
	 */
	virtual ~Activity();
	
	/**
	 * Get activity ID.
	 */
	std::string getID() { return _id; }
	
	/**
	 * Get activity ID.
	 */
	std::string getActivityClass() { return _activityImpl; }
	
	/**
	 * Set the current status of this activity.
	 * @param status The status (refer to STATUS_*).
	 */
	void setStatus(int status);
	
	/**
	 * Get the current status code of this activity.
	 */
	int getStatus() { return _status;}
	
    /**
     * Get the current status of the activity as string.
     *
     * @return string representing the status of the activity.
     *         This String is useful for user-readable output.
     * @see #getStatus()
     * @see #getStatusAsString(int)
     */
    std::string getStatusAsString() { return getStatusAsString(_status); }
    
    /**
     * Convert the status of a activity from an integer to a string.
     *
     * @param status The status code with should be converted into a string.
     * @return string representing the status that corresponds to the status code.
     */
    std::string getStatusAsString(int status);
    
    /**
      * Wait for activity to change its status.
      * @param oldStatus The old status
      * @return The new status
      */
     int waitForStatusChangeFrom(int oldStatus);
     
     /**
      * Wait for activity to change its status to a specified status.
      * @param newStatus The new status code to wait for
      */
     void waitForStatusChangeTo(int newStatus);

     /**
      * Wait for activity to change its status to COMPLETED or TERMINATED.
      */
     void waitForStatusChangeToCompletedOrTerminated();
     
     /**
      * Set activity inputs.
      */
     void setInputs(std::map<std::string,gwdl::Data*> inputs) {_inputs = inputs; }
     
     /**
      * Get activity inputs.
      */
     std::map<std::string,gwdl::Data*>& getInputs() { return _inputs; }
     
     /**
      * Set activity outputs.
      */
     void setOutputs(std::map<std::string,gwdl::Data*> outputs) {_outputs = outputs; }
     
     /**
      * Get activity outputs.
      */
     std::map<std::string,gwdl::Data*>& getOutputs() { return _outputs; }

     /**
      * Set the fault message of this activity.
      */
     void setFaultMessage(std::string message) {_faultMessage = message; }
     
     /**
      * Get the activity fault message.
      * @return Returns the fault message as std::string or "" if there is no message.
      */
     std::string& getFaultMessage() {return _faultMessage;}
     
     /**
      * Attach an observer to this activity.
      * @param observerP A pointer to the observer which should be called if this activity changes.
      */
 	void attachObserver(Observer* observerP);
     
     /**
      * Initiate this activity. Status should switch to INITIATED. Method should only work if the status was
      * UNDEFINED before. Implement this method in all derived classes!
      */
     virtual void initiateActivity() throw (ActivityException,StateTransitionException) = 0;

     /**
      * Start this activity. Status should switch to RUNNING. Implement this method in all derived classes!
      */
     virtual void startActivity() throw (ActivityException,StateTransitionException,gwdl::WorkflowFormatException) = 0;

     /**
      * Suspend this activity. Status should switch to SUSPENDED. Implement this method in all derived classes!
      */
     virtual void suspendActivity() throw (ActivityException,StateTransitionException) = 0;

     /**
      * Resume this activity. Status should switch to RUNNING. Implement this method in all derived classes!
      */
     virtual void resumeActivity() throw (ActivityException,StateTransitionException) = 0;

     /**
      * Abort this activity. Status should switch to TERMINATED. Implement this method in all derived classes!
      */
     virtual void abortActivity() throw (ActivityException,StateTransitionException) = 0;

     /**
      * Restart this activity. Status should switch to INITIATED. Implement this method in all derived classes!
      */
     virtual void restartActivity() throw (ActivityException,StateTransitionException) = 0;

}; // end class Activity

} // end namespace gwes

#endif /*ACTIVITY_H_*/
