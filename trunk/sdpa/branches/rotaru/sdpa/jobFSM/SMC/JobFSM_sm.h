#ifndef _H_JOBFSM_SM
#define _H_JOBFSM_SM

#define SMC_USES_IOSTREAMS

#include <statemap.h>

namespace sdpa
{
    namespace fsm
    {
        // Forward declarations.
        class SMC_JobFSM;
        class SMC_JobFSM_Pending;
        class SMC_JobFSM_Running;
        class SMC_JobFSM_Cancelling;
        class SMC_JobFSM_Finished;
        class SMC_JobFSM_Failed;
        class SMC_JobFSM_Cancelled;
        class SMC_JobFSM_Default;
        class JobFSMState;
        class JobFSMContext;
        class JobFSM;

        class JobFSMState :
            public statemap::State
        {
        public:

            JobFSMState(const char *name, int stateId)
            : statemap::State(name, stateId)
            {};

            virtual void Entry(JobFSMContext&) {};
            virtual void Exit(JobFSMContext&) {};

            virtual void CancelJob(JobFSMContext& context, sdpa::events::CancelJobEvent& event);
            virtual void CancelJobAck(JobFSMContext& context, sdpa::events::CancelJobAckEvent& event);
            virtual void JobFailed(JobFSMContext& context, sdpa::events::JobFailedEvent& event);
            virtual void JobFinished(JobFSMContext& context, sdpa::events::JobFinishedEvent& event);
            virtual void QueryJobStatus(JobFSMContext& context, sdpa::events::QueryJobStatusEvent& event);
            virtual void RetriveResultsEvent(JobFSMContext& context, sdpa::events::RetriveResultsEvent& event);
            virtual void RunJob(JobFSMContext& context, sdpa::events::RunJobEvent& event);

        protected:

            virtual void Default(JobFSMContext& context);
        };

        class SMC_JobFSM
        {
        public:

            static SMC_JobFSM_Pending Pending;
            static SMC_JobFSM_Running Running;
            static SMC_JobFSM_Cancelling Cancelling;
            static SMC_JobFSM_Finished Finished;
            static SMC_JobFSM_Failed Failed;
            static SMC_JobFSM_Cancelled Cancelled;
        };

        class SMC_JobFSM_Default :
            public JobFSMState
        {
        public:

            SMC_JobFSM_Default(const char *name, int stateId)
            : JobFSMState(name, stateId)
            {};

        };

        class SMC_JobFSM_Pending :
            public SMC_JobFSM_Default
        {
        public:
            SMC_JobFSM_Pending(const char *name, int stateId)
            : SMC_JobFSM_Default(name, stateId)
            {};

            void CancelJob(JobFSMContext& context, sdpa::events::CancelJobEvent& event);
            void QueryJobStatus(JobFSMContext& context, sdpa::events::QueryJobStatusEvent& event);
            void RunJob(JobFSMContext& context, sdpa::events::RunJobEvent& event);
        };

        class SMC_JobFSM_Running :
            public SMC_JobFSM_Default
        {
        public:
            SMC_JobFSM_Running(const char *name, int stateId)
            : SMC_JobFSM_Default(name, stateId)
            {};

            void CancelJob(JobFSMContext& context, sdpa::events::CancelJobEvent& event);
            void JobFailed(JobFSMContext& context, sdpa::events::JobFailedEvent& event);
            void JobFinished(JobFSMContext& context, sdpa::events::JobFinishedEvent& event);
            void QueryJobStatus(JobFSMContext& context, sdpa::events::QueryJobStatusEvent& event);
        };

        class SMC_JobFSM_Cancelling :
            public SMC_JobFSM_Default
        {
        public:
            SMC_JobFSM_Cancelling(const char *name, int stateId)
            : SMC_JobFSM_Default(name, stateId)
            {};

            void CancelJobAck(JobFSMContext& context, sdpa::events::CancelJobAckEvent& event);
            void QueryJobStatus(JobFSMContext& context, sdpa::events::QueryJobStatusEvent& event);
        };

        class SMC_JobFSM_Finished :
            public SMC_JobFSM_Default
        {
        public:
            SMC_JobFSM_Finished(const char *name, int stateId)
            : SMC_JobFSM_Default(name, stateId)
            {};

            void QueryJobStatus(JobFSMContext& context, sdpa::events::QueryJobStatusEvent& event);
            void RetriveResultsEvent(JobFSMContext& context, sdpa::events::RetriveResultsEvent& event);
        };

        class SMC_JobFSM_Failed :
            public SMC_JobFSM_Default
        {
        public:
            SMC_JobFSM_Failed(const char *name, int stateId)
            : SMC_JobFSM_Default(name, stateId)
            {};

            void QueryJobStatus(JobFSMContext& context, sdpa::events::QueryJobStatusEvent& event);
        };

        class SMC_JobFSM_Cancelled :
            public SMC_JobFSM_Default
        {
        public:
            SMC_JobFSM_Cancelled(const char *name, int stateId)
            : SMC_JobFSM_Default(name, stateId)
            {};

            void QueryJobStatus(JobFSMContext& context, sdpa::events::QueryJobStatusEvent& event);
        };

        class JobFSMContext :
            public statemap::FSMContext
        {
        public:

            JobFSMContext(JobFSM& owner)
            : _owner(owner)
            {
                setState(SMC_JobFSM::Pending);
                SMC_JobFSM::Pending.Entry(*this);
            };

            JobFSM& getOwner() const
            {
                return (_owner);
            };

            JobFSMState& getState() const
            {
                if (_state == NULL)
                {
                    throw statemap::StateUndefinedException();
                }

                return (dynamic_cast<JobFSMState&>(*_state));
            };

            void CancelJob(sdpa::events::CancelJobEvent& event)
            {
                (getState()).CancelJob(*this, event);
            };

            void CancelJobAck(sdpa::events::CancelJobAckEvent& event)
            {
                (getState()).CancelJobAck(*this, event);
            };

            void JobFailed(sdpa::events::JobFailedEvent& event)
            {
                (getState()).JobFailed(*this, event);
            };

            void JobFinished(sdpa::events::JobFinishedEvent& event)
            {
                (getState()).JobFinished(*this, event);
            };

            void QueryJobStatus(sdpa::events::QueryJobStatusEvent& event)
            {
                (getState()).QueryJobStatus(*this, event);
            };

            void RetriveResultsEvent(sdpa::events::RetriveResultsEvent& event)
            {
                (getState()).RetriveResultsEvent(*this, event);
            };

            void RunJob(sdpa::events::RunJobEvent& event)
            {
                (getState()).RunJob(*this, event);
            };

        private:

            JobFSM& _owner;
        };
    }

}

#endif // _H_JOBFSM_SM
