#ifndef _H_JOBFSM_SM
#define _H_JOBFSM_SM

#define SMC_USES_IOSTREAMS

#include <statemap.h>

namespace sdpa
{
    namespace fsm
    {
        namespace smc
        {
            // Forward declarations.
            class SDPA;
            class SDPA_Pending;
            class SDPA_Running;
            class SDPA_Cancelling;
            class SDPA_Finished;
            class SDPA_Failed;
            class SDPA_Cancelled;
            class SDPA_Default;
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

                virtual void CancelJob(JobFSMContext& context, const sdpa::events::CancelJobEvent* pEvent);
                virtual void CancelJobAck(JobFSMContext& context, const sdpa::events::CancelJobAckEvent* pEvent);
                virtual void DeleteJob(JobFSMContext& context, const sdpa::events::DeleteJobEvent* pEvent);
                virtual void Dispatch(JobFSMContext& context);
                virtual void JobFailed(JobFSMContext& context, const sdpa::events::JobFailedEvent* pEvent);
                virtual void JobFinished(JobFSMContext& context, const sdpa::events::JobFinishedEvent* pEvent);
                virtual void QueryJobStatus(JobFSMContext& context, const sdpa::events::QueryJobStatusEvent* pEvent);
                virtual void RetrieveJobResults(JobFSMContext& context, const sdpa::events::RetrieveJobResultsEvent* pEvent);

            protected:

                virtual void Default(JobFSMContext& context);
            };

            class SDPA
            {
            public:

                static SDPA_Pending Pending;
                static SDPA_Running Running;
                static SDPA_Cancelling Cancelling;
                static SDPA_Finished Finished;
                static SDPA_Failed Failed;
                static SDPA_Cancelled Cancelled;
            };

            class SDPA_Default :
                public JobFSMState
            {
            public:

                SDPA_Default(const char *name, int stateId)
                : JobFSMState(name, stateId)
                {};

            };

            class SDPA_Pending :
                public SDPA_Default
            {
            public:
                SDPA_Pending(const char *name, int stateId)
                : SDPA_Default(name, stateId)
                {};

                void CancelJob(JobFSMContext& context, const sdpa::events::CancelJobEvent* pEvent);
                void Dispatch(JobFSMContext& context);
                void QueryJobStatus(JobFSMContext& context, const sdpa::events::QueryJobStatusEvent* pEvent);
            };

            class SDPA_Running :
                public SDPA_Default
            {
            public:
                SDPA_Running(const char *name, int stateId)
                : SDPA_Default(name, stateId)
                {};

                void CancelJob(JobFSMContext& context, const sdpa::events::CancelJobEvent* pEvent);
                void CancelJobAck(JobFSMContext& context, const sdpa::events::CancelJobAckEvent* pEvent);
                void JobFailed(JobFSMContext& context, const sdpa::events::JobFailedEvent* pEvent);
                void JobFinished(JobFSMContext& context, const sdpa::events::JobFinishedEvent* pEvent);
                void QueryJobStatus(JobFSMContext& context, const sdpa::events::QueryJobStatusEvent* pEvent);
            };

            class SDPA_Cancelling :
                public SDPA_Default
            {
            public:
                SDPA_Cancelling(const char *name, int stateId)
                : SDPA_Default(name, stateId)
                {};

                void CancelJobAck(JobFSMContext& context, const sdpa::events::CancelJobAckEvent* pEvent);
                void JobFailed(JobFSMContext& context, const sdpa::events::JobFailedEvent* pEvent);
                void JobFinished(JobFSMContext& context, const sdpa::events::JobFinishedEvent* pEvent);
                void QueryJobStatus(JobFSMContext& context, const sdpa::events::QueryJobStatusEvent* pEvent);
            };

            class SDPA_Finished :
                public SDPA_Default
            {
            public:
                SDPA_Finished(const char *name, int stateId)
                : SDPA_Default(name, stateId)
                {};

                void DeleteJob(JobFSMContext& context, const sdpa::events::DeleteJobEvent* pEvent);
                void JobFinished(JobFSMContext& context, const sdpa::events::JobFinishedEvent* pEvent);
                void QueryJobStatus(JobFSMContext& context, const sdpa::events::QueryJobStatusEvent* pEvent);
                void RetrieveJobResults(JobFSMContext& context, const sdpa::events::RetrieveJobResultsEvent* pEvent);
            };

            class SDPA_Failed :
                public SDPA_Default
            {
            public:
                SDPA_Failed(const char *name, int stateId)
                : SDPA_Default(name, stateId)
                {};

                void DeleteJob(JobFSMContext& context, const sdpa::events::DeleteJobEvent* pEvent);
                void JobFailed(JobFSMContext& context, const sdpa::events::JobFailedEvent* pEvent);
                void QueryJobStatus(JobFSMContext& context, const sdpa::events::QueryJobStatusEvent* pEvent);
                void RetrieveJobResults(JobFSMContext& context, const sdpa::events::RetrieveJobResultsEvent* pEvent);
            };

            class SDPA_Cancelled :
                public SDPA_Default
            {
            public:
                SDPA_Cancelled(const char *name, int stateId)
                : SDPA_Default(name, stateId)
                {};

                void CancelJob(JobFSMContext& context, const sdpa::events::CancelJobEvent* pEvent);
                void Default(JobFSMContext& context);
                void DeleteJob(JobFSMContext& context, const sdpa::events::DeleteJobEvent* pEvent);
                void QueryJobStatus(JobFSMContext& context, const sdpa::events::QueryJobStatusEvent* pEvent);
                void RetrieveJobResults(JobFSMContext& context, const sdpa::events::RetrieveJobResultsEvent* pEvent);
            };

            class JobFSMContext :
                public statemap::FSMContext
            {
            public:

                JobFSMContext(JobFSM& owner)
                : _owner(owner)
                {
                    setState(SDPA::Pending);
                    SDPA::Pending.Entry(*this);
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

                void CancelJob(const sdpa::events::CancelJobEvent* pEvent)
                {
                    (getState()).CancelJob(*this, pEvent);
                };

                void CancelJobAck(const sdpa::events::CancelJobAckEvent* pEvent)
                {
                    (getState()).CancelJobAck(*this, pEvent);
                };

                void DeleteJob(const sdpa::events::DeleteJobEvent* pEvent)
                {
                    (getState()).DeleteJob(*this, pEvent);
                };

                void Dispatch()
                {
                    (getState()).Dispatch(*this);
                };

                void JobFailed(const sdpa::events::JobFailedEvent* pEvent)
                {
                    (getState()).JobFailed(*this, pEvent);
                };

                void JobFinished(const sdpa::events::JobFinishedEvent* pEvent)
                {
                    (getState()).JobFinished(*this, pEvent);
                };

                void QueryJobStatus(const sdpa::events::QueryJobStatusEvent* pEvent)
                {
                    (getState()).QueryJobStatus(*this, pEvent);
                };

                void RetrieveJobResults(const sdpa::events::RetrieveJobResultsEvent* pEvent)
                {
                    (getState()).RetrieveJobResults(*this, pEvent);
                };

                static JobFSMState& valueOf(int stateId);

            private:

                JobFSM& _owner;

            private:

                const static int MIN_INDEX;
                const static int MAX_INDEX;
                static JobFSMState* _States[];
            };
        }

    }

}

#endif // _H_JOBFSM_SM
