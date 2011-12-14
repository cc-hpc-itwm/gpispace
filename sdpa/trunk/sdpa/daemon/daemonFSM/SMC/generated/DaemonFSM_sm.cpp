
  // Finite state machine of the SDPA protocol
  //
#if __GNUC__ >4 || ( __GNUC__==4 && __GNUC_MINOR__ > 1)
#  pragma GCC diagnostic ignored "-Wall"
#  pragma GCC diagnostic ignored "-Wunused"
#  pragma GCC diagnostic ignored "-Weffc++"
#  pragma GCC diagnostic ignored "-Wunused-parameter"
#endif 


#include <string.h>
#include "DaemonFSM.hpp"
#include "DaemonFSM_sm.h"

using namespace statemap;

namespace sdpa
{
    namespace fsm
    {
        namespace smc
        {
            // Static class declarations.
            SMC_DaemonFSM_Down SMC_DaemonFSM::Down("SMC_DaemonFSM::Down", 0);
            SMC_DaemonFSM_Configuring SMC_DaemonFSM::Configuring("SMC_DaemonFSM::Configuring", 1);
            SMC_DaemonFSM_Up SMC_DaemonFSM::Up("SMC_DaemonFSM::Up", 2);

            const int DaemonFSMContext::MIN_INDEX = 0;
            const int DaemonFSMContext::MAX_INDEX = 2;
            DaemonFSMState* DaemonFSMContext::_States[] = 
{
                &SMC_DaemonFSM::Down,
                &SMC_DaemonFSM::Configuring,
                &SMC_DaemonFSM::Up
            };

            DaemonFSMState& DaemonFSMContext::valueOf(int stateId)
            {
                if (stateId < MIN_INDEX || stateId > MAX_INDEX)
                {
                    throw (
                        IndexOutOfBoundsException(
                            stateId, MIN_INDEX, MAX_INDEX));
                }

                return (static_cast<DaemonFSMState&>(*(_States[stateId])));
            }

            void DaemonFSMState::ConfigNok(DaemonFSMContext& context, const sdpa::events::ConfigNokEvent& event)
            {
                Default(context);
                return;
            }

            void DaemonFSMState::ConfigOk(DaemonFSMContext& context, const sdpa::events::ConfigOkEvent& event)
            {
                Default(context);
                return;
            }

            void DaemonFSMState::ConfigRequest(DaemonFSMContext& context, const sdpa::events::ConfigRequestEvent& event)
            {
                Default(context);
                return;
            }

            void DaemonFSMState::DeleteJob(DaemonFSMContext& context, const sdpa::events::DeleteJobEvent& event)
            {
                Default(context);
                return;
            }

            void DaemonFSMState::Error(DaemonFSMContext& context, const sdpa::events::ErrorEvent& event)
            {
                Default(context);
                return;
            }

            void DaemonFSMState::Interrupt(DaemonFSMContext& context, const sdpa::events::InterruptEvent& event)
            {
                Default(context);
                return;
            }

            void DaemonFSMState::RegisterWorker(DaemonFSMContext& context, const sdpa::events::WorkerRegistrationEvent& event)
            {
                Default(context);
                return;
            }

            void DaemonFSMState::RequestJob(DaemonFSMContext& context, const sdpa::events::RequestJobEvent& event)
            {
                Default(context);
                return;
            }

            void DaemonFSMState::StartUp(DaemonFSMContext& context, const sdpa::events::StartUpEvent& event)
            {
                Default(context);
                return;
            }

            void DaemonFSMState::SubmitJob(DaemonFSMContext& context, const sdpa::events::SubmitJobEvent& event)
            {
                Default(context);
                return;
            }

            void DaemonFSMState::Default(DaemonFSMContext& context)
            {
                throw (
                    TransitionUndefinedException(
                        context.getState().getName(),
                        context.getTransition()));

                return;
            }

            void SMC_DaemonFSM_Down::Default(DaemonFSMContext& context)
            {


                return;
            }

            void SMC_DaemonFSM_Down::Error(DaemonFSMContext& context, const sdpa::events::ErrorEvent& event)
            {


                return;
            }

            void SMC_DaemonFSM_Down::StartUp(DaemonFSMContext& context, const sdpa::events::StartUpEvent& event)
            {
                DaemonFSM& ctxt = context.getOwner();

                (context.getState()).Exit(context);
                context.clearState();
                try
                {
                    ctxt.action_configure(event);
                    context.setState(SMC_DaemonFSM::Configuring);
                }
                catch (...)
                {
                    context.setState(SMC_DaemonFSM::Configuring);
                    throw;
                }
                (context.getState()).Entry(context);

                return;
            }

            void SMC_DaemonFSM_Configuring::ConfigNok(DaemonFSMContext& context, const sdpa::events::ConfigNokEvent& event)
            {
                DaemonFSM& ctxt = context.getOwner();

                (context.getState()).Exit(context);
                context.clearState();
                try
                {
                    ctxt.action_config_nok(event);
                    context.setState(SMC_DaemonFSM::Down);
                }
                catch (...)
                {
                    context.setState(SMC_DaemonFSM::Down);
                    throw;
                }
                (context.getState()).Entry(context);

                return;
            }

            void SMC_DaemonFSM_Configuring::ConfigOk(DaemonFSMContext& context, const sdpa::events::ConfigOkEvent& event)
            {
                DaemonFSM& ctxt = context.getOwner();

                (context.getState()).Exit(context);
                context.clearState();
                try
                {
                    ctxt.action_config_ok(event);
                    context.setState(SMC_DaemonFSM::Up);
                }
                catch (...)
                {
                    context.setState(SMC_DaemonFSM::Up);
                    throw;
                }
                (context.getState()).Entry(context);

                return;
            }

            void SMC_DaemonFSM_Configuring::Error(DaemonFSMContext& context, const sdpa::events::ErrorEvent& event)
            {


                return;
            }

            void SMC_DaemonFSM_Up::ConfigRequest(DaemonFSMContext& context, const sdpa::events::ConfigRequestEvent& event)
            {
                DaemonFSM& ctxt = context.getOwner();

                DaemonFSMState& EndStateName = context.getState();

                context.clearState();
                try
                {
                    ctxt.action_config_request(event);
                    context.setState(EndStateName);
                }
                catch (...)
                {
                    context.setState(EndStateName);
                    throw;
                }

                return;
            }

            void SMC_DaemonFSM_Up::DeleteJob(DaemonFSMContext& context, const sdpa::events::DeleteJobEvent& event)
            {
                DaemonFSM& ctxt = context.getOwner();

                DaemonFSMState& EndStateName = context.getState();

                context.clearState();
                try
                {
                    ctxt.action_delete_job(event);
                    context.setState(EndStateName);
                }
                catch (...)
                {
                    context.setState(EndStateName);
                    throw;
                }

                return;
            }

            void SMC_DaemonFSM_Up::Error(DaemonFSMContext& context, const sdpa::events::ErrorEvent& event)
            {
                DaemonFSM& ctxt = context.getOwner();

                DaemonFSMState& EndStateName = context.getState();

                context.clearState();
                try
                {
                    ctxt.action_error_event(event);
                    context.setState(EndStateName);
                }
                catch (...)
                {
                    context.setState(EndStateName);
                    throw;
                }

                return;
            }

            void SMC_DaemonFSM_Up::Interrupt(DaemonFSMContext& context, const sdpa::events::InterruptEvent& event)
            {
                DaemonFSM& ctxt = context.getOwner();

                (context.getState()).Exit(context);
                context.clearState();
                try
                {
                    ctxt.action_interrupt(event);
                    context.setState(SMC_DaemonFSM::Down);
                }
                catch (...)
                {
                    context.setState(SMC_DaemonFSM::Down);
                    throw;
                }
                (context.getState()).Entry(context);

                return;
            }

            void SMC_DaemonFSM_Up::RegisterWorker(DaemonFSMContext& context, const sdpa::events::WorkerRegistrationEvent& event)
            {
                DaemonFSM& ctxt = context.getOwner();

                DaemonFSMState& EndStateName = context.getState();

                context.clearState();
                try
                {
                    ctxt.action_register_worker(event);
                    context.setState(EndStateName);
                }
                catch (...)
                {
                    context.setState(EndStateName);
                    throw;
                }

                return;
            }

            void SMC_DaemonFSM_Up::RequestJob(DaemonFSMContext& context, const sdpa::events::RequestJobEvent& event)
            {
                DaemonFSM& ctxt = context.getOwner();

                DaemonFSMState& EndStateName = context.getState();

                context.clearState();
                try
                {
                    ctxt.action_request_job(event);
                    context.setState(EndStateName);
                }
                catch (...)
                {
                    context.setState(EndStateName);
                    throw;
                }

                return;
            }

            void SMC_DaemonFSM_Up::SubmitJob(DaemonFSMContext& context, const sdpa::events::SubmitJobEvent& event)
            {
                DaemonFSM& ctxt = context.getOwner();

                DaemonFSMState& EndStateName = context.getState();

                context.clearState();
                try
                {
                    ctxt.action_submit_job(event);
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
