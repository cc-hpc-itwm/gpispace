#ifndef JOB_FSM_SMC_HPP
#define JOB_FSM_SMC_HPP 1

#include <sdpa/daemon/IComm.hpp>
#include <sdpa/daemon/JobImpl.hpp>
#include <sdpa/daemon/jobFSM/SMC/JobFSM_sm.h>
#include <sdpa/logging.hpp>
#include <sdpa/types.hpp>

namespace sdpa { namespace fsm { namespace smc {
	class JobFSM : public sdpa::daemon::JobImpl {
		public:
			typedef std::tr1::shared_ptr<JobFSM> Ptr;

			JobFSM( const sdpa::job_id_t &id,
					const sdpa::job_desc_t &desc,
				    const sdpa::daemon::IComm* pHandler = NULL,
				    const sdpa::job_id_t &parent = sdpa::job_id_t::invalid_job_id())
				: JobImpl(id, desc, pHandler, parent), SDPA_INIT_LOGGER("sdpa.fsm.smc.JobFSM"), m_fsmContext(*this) {
				SDPA_LOG_DEBUG("Job state machine created");
			}

			virtual ~JobFSM()  throw () { SDPA_LOG_DEBUG("Job state machine destroyed"); }

			//transitions
			void CancelJob(const sdpa::events::CancelJobEvent*);
			void CancelJobAck(const sdpa::events::CancelJobAckEvent*);
			void DeleteJob(const sdpa::events::DeleteJobEvent*);
			void JobFailed(const sdpa::events::JobFailedEvent*);
			void JobFinished(const sdpa::events::JobFinishedEvent*);
			void QueryJobStatus(const sdpa::events::QueryJobStatusEvent*);
			void RetrieveJobResults(const sdpa::events::RetrieveJobResultsEvent*);
			void Dispatch();

			sdpa::status_t getStatus() { return m_status_; }

			JobFSMContext& GetContext() { return m_fsmContext; }
		private:
			SDPA_DECLARE_LOGGER();
			JobFSMContext m_fsmContext;
			sdpa::status_t m_status_;
	};
}}}

#endif
