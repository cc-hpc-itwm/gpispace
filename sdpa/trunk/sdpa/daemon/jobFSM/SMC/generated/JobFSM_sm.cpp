
  // Finite state machine of the SDPA protocol
  //
#if __GNUC__ >4 || ( __GNUC__==4 && __GNUC_MINOR__ > 1)
#  pragma GCC diagnostic ignored "-Wall"
#  pragma GCC diagnostic ignored "-Wunused"
#  pragma GCC diagnostic ignored "-Weffc++"
#  pragma GCC diagnostic ignored "-Wunused-parameter"
#endif


#include <string.h>
#include <sdpa/events/CancelJobAckEvent.hpp>
#include <sdpa/events/CancelJobEvent.hpp>
#include <sdpa/events/DeleteJobAckEvent.hpp>
#include <sdpa/events/DeleteJobEvent.hpp>
#include <sdpa/events/ErrorEvent.hpp>
#include <sdpa/events/JobEvent.hpp>
#include <sdpa/events/JobFailedAckEvent.hpp>
#include <sdpa/events/JobFailedEvent.hpp>
#include <sdpa/events/JobFinishedAckEvent.hpp>
#include <sdpa/events/JobFinishedEvent.hpp>
#include <sdpa/events/JobResultsReplyEvent.hpp>
#include <sdpa/events/JobStatusReplyEvent.hpp>
#include <sdpa/events/MgmtEvent.hpp>
#include <sdpa/events/QueryJobStatusEvent.hpp>
#include <sdpa/events/RetrieveJobResultsEvent.hpp>
#include "JobFSM.hpp"
#include "JobFSM_sm.h"

using namespace statemap;

namespace sdpa
{
    namespace fsm
    {
        namespace smc
        {
            // Static class declarations.
            SDPA_Pending SDPA::Pending("SDPA::Pending", 0);
            SDPA_Running SDPA::Running("SDPA::Running", 1);
            SDPA_Cancelling SDPA::Cancelling("SDPA::Cancelling", 2);
            SDPA_Finished SDPA::Finished("SDPA::Finished", 3);
            SDPA_Failed SDPA::Failed("SDPA::Failed", 4);
            SDPA_Cancelled SDPA::Cancelled("SDPA::Cancelled", 5);

            const int JobFSMContext::MIN_INDEX = 0;
            const int JobFSMContext::MAX_INDEX = 5;
            JobFSMState* JobFSMContext::_States[] = 
{
                &SDPA::Pending,
                &SDPA::Running,
                &SDPA::Cancelling,
                &SDPA::Finished,
                &SDPA::Failed,
                &SDPA::Cancelled
            };

            JobFSMState& JobFSMContext::valueOf(int stateId)
            {
                if (stateId < MIN_INDEX || stateId > MAX_INDEX)
                {
                    throw (
                        IndexOutOfBoundsException(
                            stateId, MIN_INDEX, MAX_INDEX));
                }

                return (static_cast<JobFSMState&>(*(_States[stateId])));
            }

            void JobFSMState::CancelJob(JobFSMContext& context, const sdpa::events::CancelJobEvent* pEvent)
            {
                Default(context);
                return;
            }

            void JobFSMState::CancelJobAck(JobFSMContext& context, const sdpa::events::CancelJobAckEvent* pEvent)
            {
                Default(context);
                return;
            }

            void JobFSMState::DeleteJob(JobFSMContext& context, const sdpa::events::DeleteJobEvent* pEvent)
            {
                Default(context);
                return;
            }

            void JobFSMState::Dispatch(JobFSMContext& context)
            {
                Default(context);
                return;
            }

            void JobFSMState::JobFailed(JobFSMContext& context, const sdpa::events::JobFailedEvent* pEvent)
            {
                Default(context);
                return;
            }

            void JobFSMState::JobFinished(JobFSMContext& context, const sdpa::events::JobFinishedEvent* pEvent)
            {
                Default(context);
                return;
            }

            void JobFSMState::QueryJobStatus(JobFSMContext& context, const sdpa::events::QueryJobStatusEvent* pEvent)
            {
                Default(context);
                return;
            }

            void JobFSMState::RetrieveJobResults(JobFSMContext& context, const sdpa::events::RetrieveJobResultsEvent* pEvent)
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

            void SDPA_Pending::CancelJob(JobFSMContext& context, const sdpa::events::CancelJobEvent* pEvent)
            {
                JobFSM& ctxt = context.getOwner();

                (context.getState()).Exit(context);
                context.clearState();
                try
                {
                    ctxt.action_cancel_job_from_pending(*pEvent);
                    context.setState(SDPA::Cancelled);
                }
                catch (...)
                {
                    context.setState(SDPA::Cancelled);
                    throw;
                }
                (context.getState()).Entry(context);

                return;
            }

            void SDPA_Pending::Dispatch(JobFSMContext& context)
            {
                JobFSM& ctxt = context.getOwner();

                (context.getState()).Exit(context);
                context.clearState();
                try
                {
                    ctxt.action_run_job();
                    context.setState(SDPA::Running);
                }
                catch (...)
                {
                    context.setState(SDPA::Running);
                    throw;
                }
                (context.getState()).Entry(context);

                return;
            }

            void SDPA_Pending::QueryJobStatus(JobFSMContext& context, const sdpa::events::QueryJobStatusEvent* pEvent)
            {


                return;
            }

            void SDPA_Running::CancelJob(JobFSMContext& context, const sdpa::events::CancelJobEvent* pEvent)
            {
                JobFSM& ctxt = context.getOwner();

                (context.getState()).Exit(context);
                context.clearState();
                try
                {
                    ctxt.action_cancel_job(*pEvent);
                    context.setState(SDPA::Cancelling);
                }
                catch (...)
                {
                    context.setState(SDPA::Cancelling);
                    throw;
                }
                (context.getState()).Entry(context);

                return;
            }

            void SDPA_Running::CancelJobAck(JobFSMContext& context, const sdpa::events::CancelJobAckEvent* pEvent)
            {

                (context.getState()).Exit(context);
                context.setState(SDPA::Cancelled);
                (context.getState()).Entry(context);

                return;
            }

            void SDPA_Running::JobFailed(JobFSMContext& context, const sdpa::events::JobFailedEvent* pEvent)
            {
                JobFSM& ctxt = context.getOwner();

                (context.getState()).Exit(context);
                context.clearState();
                try
                {
                    ctxt.action_job_failed(*pEvent);
                    context.setState(SDPA::Failed);
                }
                catch (...)
                {
                    context.setState(SDPA::Failed);
                    throw;
                }
                (context.getState()).Entry(context);

                return;
            }

            void SDPA_Running::JobFinished(JobFSMContext& context, const sdpa::events::JobFinishedEvent* pEvent)
            {
                JobFSM& ctxt = context.getOwner();

                (context.getState()).Exit(context);
                context.clearState();
                try
                {
                    ctxt.action_job_finished(*pEvent);
                    context.setState(SDPA::Finished);
                }
                catch (...)
                {
                    context.setState(SDPA::Finished);
                    throw;
                }
                (context.getState()).Entry(context);

                return;
            }

            void SDPA_Running::QueryJobStatus(JobFSMContext& context, const sdpa::events::QueryJobStatusEvent* pEvent)
            {


                return;
            }

            void SDPA_Cancelling::CancelJobAck(JobFSMContext& context, const sdpa::events::CancelJobAckEvent* pEvent)
            {
                JobFSM& ctxt = context.getOwner();

                (context.getState()).Exit(context);
                context.clearState();
                try
                {
                    ctxt.action_cancel_job_ack(*pEvent);
                    context.setState(SDPA::Cancelled);
                }
                catch (...)
                {
                    context.setState(SDPA::Cancelled);
                    throw;
                }
                (context.getState()).Entry(context);

                return;
            }

            void SDPA_Cancelling::JobFailed(JobFSMContext& context, const sdpa::events::JobFailedEvent* pEvent)
            {
                JobFSM& ctxt = context.getOwner();

                (context.getState()).Exit(context);
                context.clearState();
                try
                {
                    ctxt.action_job_failed(*pEvent);
                    context.setState(SDPA::Failed);
                }
                catch (...)
                {
                    context.setState(SDPA::Failed);
                    throw;
                }
                (context.getState()).Entry(context);

                return;
            }

            void SDPA_Cancelling::JobFinished(JobFSMContext& context, const sdpa::events::JobFinishedEvent* pEvent)
            {
                JobFSM& ctxt = context.getOwner();

                (context.getState()).Exit(context);
                context.clearState();
                try
                {
                    ctxt.action_job_finished(*pEvent);
                    context.setState(SDPA::Finished);
                }
                catch (...)
                {
                    context.setState(SDPA::Finished);
                    throw;
                }
                (context.getState()).Entry(context);

                return;
            }

            void SDPA_Cancelling::QueryJobStatus(JobFSMContext& context, const sdpa::events::QueryJobStatusEvent* pEvent)
            {


                return;
            }

            void SDPA_Finished::DeleteJob(JobFSMContext& context, const sdpa::events::DeleteJobEvent* pEvent)
            {
                JobFSM& ctxt = context.getOwner();

                JobFSMState& EndStateName = context.getState();

                context.clearState();
                try
                {
                    ctxt.action_delete_job(*pEvent);
                    context.setState(EndStateName);
                }
                catch (...)
                {
                    context.setState(EndStateName);
                    throw;
                }

                return;
            }

            void SDPA_Finished::JobFinished(JobFSMContext& context, const sdpa::events::JobFinishedEvent* pEvent)
            {


                return;
            }

            void SDPA_Finished::QueryJobStatus(JobFSMContext& context, const sdpa::events::QueryJobStatusEvent* pEvent)
            {


                return;
            }

            void SDPA_Finished::RetrieveJobResults(JobFSMContext& context, const sdpa::events::RetrieveJobResultsEvent* pEvent)
            {
                JobFSM& ctxt = context.getOwner();

                JobFSMState& EndStateName = context.getState();

                context.clearState();
                try
                {
                    ctxt.action_retrieve_job_results(*pEvent);
                    context.setState(EndStateName);
                }
                catch (...)
                {
                    context.setState(EndStateName);
                    throw;
                }

                return;
            }

            void SDPA_Failed::DeleteJob(JobFSMContext& context, const sdpa::events::DeleteJobEvent* pEvent)
            {
                JobFSM& ctxt = context.getOwner();

                JobFSMState& EndStateName = context.getState();

                context.clearState();
                try
                {
                    ctxt.action_delete_job(*pEvent);
                    context.setState(EndStateName);
                }
                catch (...)
                {
                    context.setState(EndStateName);
                    throw;
                }

                return;
            }

            void SDPA_Failed::JobFailed(JobFSMContext& context, const sdpa::events::JobFailedEvent* pEvent)
            {


                return;
            }

            void SDPA_Failed::QueryJobStatus(JobFSMContext& context, const sdpa::events::QueryJobStatusEvent* pEvent)
            {


                return;
            }

            void SDPA_Failed::RetrieveJobResults(JobFSMContext& context, const sdpa::events::RetrieveJobResultsEvent* pEvent)
            {
                JobFSM& ctxt = context.getOwner();

                JobFSMState& EndStateName = context.getState();

                context.clearState();
                try
                {
                    ctxt.action_retrieve_job_results(*pEvent);
                    context.setState(EndStateName);
                }
                catch (...)
                {
                    context.setState(EndStateName);
                    throw;
                }

                return;
            }

            void SDPA_Cancelled::CancelJob(JobFSMContext& context, const sdpa::events::CancelJobEvent* pEvent)
            {


                return;
            }

            void SDPA_Cancelled::Default(JobFSMContext& context)
            {


                return;
            }

            void SDPA_Cancelled::DeleteJob(JobFSMContext& context, const sdpa::events::DeleteJobEvent* pEvent)
            {
                JobFSM& ctxt = context.getOwner();

                JobFSMState& EndStateName = context.getState();

                context.clearState();
                try
                {
                    ctxt.action_delete_job(*pEvent);
                    context.setState(EndStateName);
                }
                catch (...)
                {
                    context.setState(EndStateName);
                    throw;
                }

                return;
            }

            void SDPA_Cancelled::QueryJobStatus(JobFSMContext& context, const sdpa::events::QueryJobStatusEvent* pEvent)
            {


                return;
            }

            void SDPA_Cancelled::RetrieveJobResults(JobFSMContext& context, const sdpa::events::RetrieveJobResultsEvent* pEvent)
            {
                JobFSM& ctxt = context.getOwner();

                JobFSMState& EndStateName = context.getState();

                context.clearState();
                try
                {
                    ctxt.action_retrieve_job_results(*pEvent);
                    context.setState(EndStateName);
                }
                catch (...)
                {
                    context.setState(EndStateName);
                    throw;
                }

                return;
            }
        }
    }
}
