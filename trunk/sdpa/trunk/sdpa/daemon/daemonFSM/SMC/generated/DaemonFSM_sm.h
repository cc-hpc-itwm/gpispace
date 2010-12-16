#ifndef _H_DAEMONFSM_SM
#define _H_DAEMONFSM_SM

#define SMC_USES_IOSTREAMS

#include <statemap.h>

namespace sdpa
{
    namespace fsm
    {
        namespace smc
        {
            // Forward declarations.
            class SMC_DaemonFSM;
            class SMC_DaemonFSM_Down;
            class SMC_DaemonFSM_Configuring;
            class SMC_DaemonFSM_Up;
            class SMC_DaemonFSM_Default;
            class DaemonFSMState;
            class DaemonFSMContext;
            class DaemonFSM;

            class DaemonFSMState :
                public statemap::State
            {
            public:

                DaemonFSMState(const char *name, int stateId)
                : statemap::State(name, stateId)
                {};

                virtual void Entry(DaemonFSMContext&) {};
                virtual void Exit(DaemonFSMContext&) {};

                virtual void ConfigNok(DaemonFSMContext& context, const sdpa::events::ConfigNokEvent& event);
                virtual void ConfigOk(DaemonFSMContext& context, const sdpa::events::ConfigOkEvent& event);
                virtual void ConfigRequest(DaemonFSMContext& context, const sdpa::events::ConfigRequestEvent& event);
                virtual void DeleteJob(DaemonFSMContext& context, const sdpa::events::DeleteJobEvent& event);
                virtual void Error(DaemonFSMContext& context, const sdpa::events::ErrorEvent& event);
                virtual void Interrupt(DaemonFSMContext& context, const sdpa::events::InterruptEvent& event);
                virtual void RegisterWorker(DaemonFSMContext& context, const sdpa::events::WorkerRegistrationEvent& event);
                virtual void RequestJob(DaemonFSMContext& context, const sdpa::events::RequestJobEvent& event);
                virtual void StartUp(DaemonFSMContext& context, const sdpa::events::StartUpEvent& event);
                virtual void SubmitJob(DaemonFSMContext& context, const sdpa::events::SubmitJobEvent& event);

            protected:

                virtual void Default(DaemonFSMContext& context);
            };

            class SMC_DaemonFSM
            {
            public:

                static SMC_DaemonFSM_Down Down;
                static SMC_DaemonFSM_Configuring Configuring;
                static SMC_DaemonFSM_Up Up;
            };

            class SMC_DaemonFSM_Default :
                public DaemonFSMState
            {
            public:

                SMC_DaemonFSM_Default(const char *name, int stateId)
                : DaemonFSMState(name, stateId)
                {};

            };

            class SMC_DaemonFSM_Down :
                public SMC_DaemonFSM_Default
            {
            public:
                SMC_DaemonFSM_Down(const char *name, int stateId)
                : SMC_DaemonFSM_Default(name, stateId)
                {};

                void Default(DaemonFSMContext& context);
                void Error(DaemonFSMContext& context, const sdpa::events::ErrorEvent& event);
                void StartUp(DaemonFSMContext& context, const sdpa::events::StartUpEvent& event);
            };

            class SMC_DaemonFSM_Configuring :
                public SMC_DaemonFSM_Default
            {
            public:
                SMC_DaemonFSM_Configuring(const char *name, int stateId)
                : SMC_DaemonFSM_Default(name, stateId)
                {};

                void ConfigNok(DaemonFSMContext& context, const sdpa::events::ConfigNokEvent& event);
                void ConfigOk(DaemonFSMContext& context, const sdpa::events::ConfigOkEvent& event);
                void Error(DaemonFSMContext& context, const sdpa::events::ErrorEvent& event);
            };

            class SMC_DaemonFSM_Up :
                public SMC_DaemonFSM_Default
            {
            public:
                SMC_DaemonFSM_Up(const char *name, int stateId)
                : SMC_DaemonFSM_Default(name, stateId)
                {};

                void ConfigRequest(DaemonFSMContext& context, const sdpa::events::ConfigRequestEvent& event);
                void DeleteJob(DaemonFSMContext& context, const sdpa::events::DeleteJobEvent& event);
                void Error(DaemonFSMContext& context, const sdpa::events::ErrorEvent& event);
                void Interrupt(DaemonFSMContext& context, const sdpa::events::InterruptEvent& event);
                void RegisterWorker(DaemonFSMContext& context, const sdpa::events::WorkerRegistrationEvent& event);
                void RequestJob(DaemonFSMContext& context, const sdpa::events::RequestJobEvent& event);
                void SubmitJob(DaemonFSMContext& context, const sdpa::events::SubmitJobEvent& event);
            };

            class DaemonFSMContext :
                public statemap::FSMContext
            {
            public:

                DaemonFSMContext(DaemonFSM& owner)
                : _owner(owner)
                {
                    setState(SMC_DaemonFSM::Down);
                    SMC_DaemonFSM::Down.Entry(*this);
                };

                DaemonFSM& getOwner() const
                {
                    return (_owner);
                };

                DaemonFSMState& getState() const
                {
                    if (_state == NULL)
                    {
                        throw statemap::StateUndefinedException();
                    }

                    return (dynamic_cast<DaemonFSMState&>(*_state));
                };

                void ConfigNok(const sdpa::events::ConfigNokEvent& event)
                {
                    (getState()).ConfigNok(*this, event);
                };

                void ConfigOk(const sdpa::events::ConfigOkEvent& event)
                {
                    (getState()).ConfigOk(*this, event);
                };

                void ConfigRequest(const sdpa::events::ConfigRequestEvent& event)
                {
                    (getState()).ConfigRequest(*this, event);
                };

                void DeleteJob(const sdpa::events::DeleteJobEvent& event)
                {
                    (getState()).DeleteJob(*this, event);
                };

                void Error(const sdpa::events::ErrorEvent& event)
                {
                    (getState()).Error(*this, event);
                };

                void Interrupt(const sdpa::events::InterruptEvent& event)
                {
                    (getState()).Interrupt(*this, event);
                };

                void RegisterWorker(const sdpa::events::WorkerRegistrationEvent& event)
                {
                    (getState()).RegisterWorker(*this, event);
                };

                void RequestJob(const sdpa::events::RequestJobEvent& event)
                {
                    (getState()).RequestJob(*this, event);
                };

                void StartUp(const sdpa::events::StartUpEvent& event)
                {
                    (getState()).StartUp(*this, event);
                };

                void SubmitJob(const sdpa::events::SubmitJobEvent& event)
                {
                    (getState()).SubmitJob(*this, event);
                };

                static DaemonFSMState& valueOf(int stateId);

            private:

                DaemonFSM& _owner;

            private:

                const static int MIN_INDEX;
                const static int MAX_INDEX;
                static DaemonFSMState* _States[];
            };
        }

    }

}

#endif // _H_DAEMONFSM_SM
