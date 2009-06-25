
  // Finite state machine of the SDPA protocol
  //


#include <string.h>
#include "JobFSM.hpp"
#include "JobFSM_sm.h"

using namespace statemap;

namespace sdpa
{
    namespace fsm
    {
        // Static class declarations.
        SMC_JobFSM_Pending SMC_JobFSM::Pending("SMC_JobFSM::Pending", 0);
        SMC_JobFSM_Running SMC_JobFSM::Running("SMC_JobFSM::Running", 1);
        SMC_JobFSM_Cancelling SMC_JobFSM::Cancelling("SMC_JobFSM::Cancelling", 2);
        SMC_JobFSM_Finished SMC_JobFSM::Finished("SMC_JobFSM::Finished", 3);
        SMC_JobFSM_Failed SMC_JobFSM::Failed("SMC_JobFSM::Failed", 4);
        SMC_JobFSM_Cancelled SMC_JobFSM::Cancelled("SMC_JobFSM::Cancelled", 5);

        void JobFSMState::CancelJob(JobFSMContext& context, sdpa::events::CancelJobEvent& event)
        {
            Default(context);
            return;
        }

        void JobFSMState::CancelJobAck(JobFSMContext& context, sdpa::events::CancelJobAckEvent& event)
        {
            Default(context);
            return;
        }

        void JobFSMState::JobFailed(JobFSMContext& context, sdpa::events::JobFailedEvent& event)
        {
            Default(context);
            return;
        }

        void JobFSMState::JobFinished(JobFSMContext& context, sdpa::events::JobFinishedEvent& event)
        {
            Default(context);
            return;
        }

        void JobFSMState::QueryJobStatus(JobFSMContext& context, sdpa::events::QueryJobStatusEvent& event)
        {
            Default(context);
            return;
        }

        void JobFSMState::RetriveResultsEvent(JobFSMContext& context, sdpa::events::RetriveResultsEvent& event)
        {
            Default(context);
            return;
        }

        void JobFSMState::RunJob(JobFSMContext& context, sdpa::events::RunJobEvent& event)
        {
            Default(context);
            return;
        }

        void JobFSMState::Default(JobFSMContext& context)
        {
            throw (
                TransitionUndefinedException(
                    context.getState().getName(),
                    context.getTransition()));

            return;
        }

        void SMC_JobFSM_Pending::CancelJob(JobFSMContext& context, sdpa::events::CancelJobEvent& event)
        {
            JobFSM& ctxt(context.getOwner());

            (context.getState()).Exit(context);
            context.clearState();
            try
            {
                ctxt.action_cancel(event);
                context.setState(SMC_JobFSM::Cancelled);
            }
            catch (...)
            {
                context.setState(SMC_JobFSM::Cancelled);
                throw;
            }
            (context.getState()).Entry(context);

            return;
        }

        void SMC_JobFSM_Pending::QueryJobStatus(JobFSMContext& context, sdpa::events::QueryJobStatusEvent& event)
        {
            JobFSM& ctxt(context.getOwner());

            (context.getState()).Exit(context);
            context.clearState();
            try
            {
                ctxt.action_query_status(event);
                context.setState(SMC_JobFSM::Pending);
            }
            catch (...)
            {
                context.setState(SMC_JobFSM::Pending);
                throw;
            }
            (context.getState()).Entry(context);

            return;
        }

        void SMC_JobFSM_Pending::RunJob(JobFSMContext& context, sdpa::events::RunJobEvent& event)
        {
            JobFSM& ctxt(context.getOwner());

            (context.getState()).Exit(context);
            context.clearState();
            try
            {
                ctxt.action_dispatch(event);
                context.setState(SMC_JobFSM::Running);
            }
            catch (...)
            {
                context.setState(SMC_JobFSM::Running);
                throw;
            }
            (context.getState()).Entry(context);

            return;
        }

        void SMC_JobFSM_Running::CancelJob(JobFSMContext& context, sdpa::events::CancelJobEvent& event)
        {
            JobFSM& ctxt(context.getOwner());

            (context.getState()).Exit(context);
            context.clearState();
            try
            {
                ctxt.action_cancel(event);
                context.setState(SMC_JobFSM::Cancelling);
            }
            catch (...)
            {
                context.setState(SMC_JobFSM::Cancelling);
                throw;
            }
            (context.getState()).Entry(context);

            return;
        }

        void SMC_JobFSM_Running::JobFailed(JobFSMContext& context, sdpa::events::JobFailedEvent& event)
        {
            JobFSM& ctxt(context.getOwner());

            (context.getState()).Exit(context);
            context.clearState();
            try
            {
                ctxt.action_job_failed(event);
                context.setState(SMC_JobFSM::Failed);
            }
            catch (...)
            {
                context.setState(SMC_JobFSM::Failed);
                throw;
            }
            (context.getState()).Entry(context);

            return;
        }

        void SMC_JobFSM_Running::JobFinished(JobFSMContext& context, sdpa::events::JobFinishedEvent& event)
        {
            JobFSM& ctxt(context.getOwner());

            (context.getState()).Exit(context);
            context.clearState();
            try
            {
                ctxt.action_job_finished(event);
                context.setState(SMC_JobFSM::Finished);
            }
            catch (...)
            {
                context.setState(SMC_JobFSM::Finished);
                throw;
            }
            (context.getState()).Entry(context);

            return;
        }

        void SMC_JobFSM_Running::QueryJobStatus(JobFSMContext& context, sdpa::events::QueryJobStatusEvent& event)
        {
            JobFSM& ctxt(context.getOwner());

            (context.getState()).Exit(context);
            context.clearState();
            try
            {
                ctxt.action_query_status(event);
                context.setState(SMC_JobFSM::Running);
            }
            catch (...)
            {
                context.setState(SMC_JobFSM::Running);
                throw;
            }
            (context.getState()).Entry(context);

            return;
        }

        void SMC_JobFSM_Cancelling::CancelJobAck(JobFSMContext& context, sdpa::events::CancelJobAckEvent& event)
        {
            JobFSM& ctxt(context.getOwner());

            (context.getState()).Exit(context);
            context.clearState();
            try
            {
                ctxt.action_cancel_ack(event);
                context.setState(SMC_JobFSM::Cancelled);
            }
            catch (...)
            {
                context.setState(SMC_JobFSM::Cancelled);
                throw;
            }
            (context.getState()).Entry(context);

            return;
        }

        void SMC_JobFSM_Cancelling::QueryJobStatus(JobFSMContext& context, sdpa::events::QueryJobStatusEvent& event)
        {
            JobFSM& ctxt(context.getOwner());

            (context.getState()).Exit(context);
            context.clearState();
            try
            {
                ctxt.action_query_status(event);
                context.setState(SMC_JobFSM::Cancelling);
            }
            catch (...)
            {
                context.setState(SMC_JobFSM::Cancelling);
                throw;
            }
            (context.getState()).Entry(context);

            return;
        }

        void SMC_JobFSM_Finished::QueryJobStatus(JobFSMContext& context, sdpa::events::QueryJobStatusEvent& event)
        {
            JobFSM& ctxt(context.getOwner());

            (context.getState()).Exit(context);
            context.clearState();
            try
            {
                ctxt.action_query_status(event);
                context.setState(SMC_JobFSM::Finished);
            }
            catch (...)
            {
                context.setState(SMC_JobFSM::Finished);
                throw;
            }
            (context.getState()).Entry(context);

            return;
        }

        void SMC_JobFSM_Finished::RetriveResultsEvent(JobFSMContext& context, sdpa::events::RetriveResultsEvent& event)
        {
            JobFSM& ctxt(context.getOwner());

            (context.getState()).Exit(context);
            context.clearState();
            try
            {
                ctxt.action_retrieve_results(event);
                context.setState(SMC_JobFSM::Finished);
            }
            catch (...)
            {
                context.setState(SMC_JobFSM::Finished);
                throw;
            }
            (context.getState()).Entry(context);

            return;
        }

        void SMC_JobFSM_Failed::QueryJobStatus(JobFSMContext& context, sdpa::events::QueryJobStatusEvent& event)
        {
            JobFSM& ctxt(context.getOwner());

            (context.getState()).Exit(context);
            context.clearState();
            try
            {
                ctxt.action_query_status(event);
                context.setState(SMC_JobFSM::Failed);
            }
            catch (...)
            {
                context.setState(SMC_JobFSM::Failed);
                throw;
            }
            (context.getState()).Entry(context);

            return;
        }

        void SMC_JobFSM_Cancelled::QueryJobStatus(JobFSMContext& context, sdpa::events::QueryJobStatusEvent& event)
        {
            JobFSM& ctxt(context.getOwner());

            (context.getState()).Exit(context);
            context.clearState();
            try
            {
                ctxt.action_query_status(event);
                context.setState(SMC_JobFSM::Cancelled);
            }
            catch (...)
            {
                context.setState(SMC_JobFSM::Cancelled);
                throw;
            }
            (context.getState()).Entry(context);

            return;
        }
    }
}
