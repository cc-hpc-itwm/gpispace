#ifndef SDPA_WF_WFE_TO_SDPA_HPP
#define SDPA_WF_WFE_TO_SDPA_HPP 1

#include <string>
#include <sdpa/wf/types.hpp>
#include <sdpa/daemon/exceptions.hpp>

namespace sdpa { namespace wf {
  class WFE_to_SDPA {
    public:
      /**
       * Submit a sub workflow to the SDPA.
       * This method is to be called by the GWES in order to delegate
       * the execution of sub workflows.
       * The SDPA will use the callback handler SdpaGwes in order
       * to notify the GWES about status transitions.
      */
      virtual workflow_id_t submitWorkflow(const workflow_t &workflow) = 0;

      /**
       * Submit an atomic activity to the SDPA.
       * This method is to be called by the GWES in order to delegate
       * the execution of activities.
       * The SDPA will use the callback handler SdpaGwes in order
       * to notify the GWES about activity status transitions.
       */
      virtual activity_id_t submitActivity(const activity_t &activity) = 0;

      /**
       * Cancel a sub workflow that has previously been submitted to
       * the SDPA. The parent job has to cancel all children.
       */
      virtual void cancelWorkflow(const workflow_id_t &workflowId) throw (sdpa::daemon::NoSuchWorkflowException) = 0;

      /**
       * Cancel an atomic activity that has previously been submitted to
       * the SDPA.
       */
      virtual void cancelActivity(const activity_id_t &activityId) throw (sdpa::daemon::NoSuchActivityException) = 0;

      /**
       * Notify the SDPA that a workflow finished (state transition
       * from running to finished).
       */
      virtual void workflowFinished(const workflow_id_t &workflowId) throw (sdpa::daemon::NoSuchWorkflowException) = 0;

      /**
       * Notify the SDPA that a workflow failed (state transition
       * from running to failed).
       */
      virtual void workflowFailed(const workflow_id_t &workflowId) throw (sdpa::daemon::NoSuchWorkflowException) = 0;

      /**
       * Notify the SDPA that a workflow has been canceled (state
       * transition from * to terminated.
       */
      virtual void workflowCanceled(const workflow_id_t &workflowId) throw (sdpa::daemon::NoSuchWorkflowException) = 0;

  };
}}
#endif
