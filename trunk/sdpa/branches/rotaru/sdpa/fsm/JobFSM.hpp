#ifndef SDPASMCFSM_HPP
#define SDPASMCFSM_HPP 1


#include <sdpa/events/RunJobEvent.hpp>
#include <sdpa/events/JobFailedEvent.hpp>
#include <sdpa/events/QueryJobStatusEvent.hpp>
#include <sdpa/events/JobStatusAnswerEvent.hpp>
#include <sdpa/events/CancelJobEvent.hpp>
#include <sdpa/events/CancelJobAckEvent.hpp>
#include <sdpa/events/JobFinishedEvent.hpp>
#include <sdpa/events/ErrorEvent.hpp>
#include <sdpa/fsm/JobFSM_sm.h>
#include <sdpa/logging.hpp>

#include <list>

namespace sdpa {
	namespace fsm {
		class JobFSM {
			public:
				typedef std::tr1::shared_ptr<JobFSM> Ptr;

				JobFSM();
				virtual ~JobFSM();

				int InformWFEJobFailed( sdpa::Job::job_id_t JobID );
				int GetNextActiveSubJobsListFromWFE( sdpa::Job::job_id_t JobID );  //assign unique global IDs!
				int ScheduleJobs();
				int DoCancelJob( sdpa::Job::job_id_t JobID );
				int PostCancelJobAckEventForMaster( sdpa::events::CancelJobEvent& event );
				int PostJobStatusAnswerEventForMaster( sdpa::events::QueryJobStatusEvent& event );
				int PostJobFinishedEventForMaster( sdpa::events::JobFinishedEvent& event );
				int PostJobFailedEventForMaster( sdpa::events::JobFailedEvent& event);
				int PostCancelJobAckEventForMaster( sdpa::events::CancelJobAckEvent& event );

				int HandleJobFailure( sdpa::Job::job_id_t JobID );
				int DoCancelSubJobs( sdpa::Job::job_id_t JobID );// Attention!: some of SubJobs may already have finished!

				bool IsSubJob( sdpa::Job::job_id_t JobID );
				int IncGetCancelAckCounter();
				int GetCancelAckCounter();
				int GetNumberSubJobs();

				sdpa::fsm::JobFSMContext& GetContext() { return m_fsmContext; }
			private:
				SDPA_DECLARE_LOGGER();
				sdpa::fsm::JobFSMContext m_fsmContext;

				int m_nNumberSubJobs;
				int m_nCancelAckCounter;
				std::list<std::string> m_listNextActiveSubJobs;
				//boost::recursive_mutex _mtx;
		};
	}
}

#endif
