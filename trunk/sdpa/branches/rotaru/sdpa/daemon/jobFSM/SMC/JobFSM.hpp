#ifndef JOB_FSM_SMC_HPP
#define JOB_FSM_SMC_HPP 1

#include <sdpa/daemon/JobImpl.hpp>
#include <sdpa/daemon/jobFSM/SMC/JobFSM_sm.h>
#include <sdpa/logging.hpp>

namespace sdpa { namespace fsm { namespace smc {
	class JobFSM : public sdpa::daemon::JobImpl {
		public:
			typedef std::tr1::shared_ptr<JobFSM> Ptr;

			JobFSM( const sdpa::job_id_t &id,
					const sdpa::job_desc_t &desc,
				    const sdpa::daemon::ISendEvent* pHandler = NULL,
				    const sdpa::job_id_t &parent = Job::invalid_job_id())
				: JobImpl(id, desc, pHandler, parent), SDPA_INIT_LOGGER("sdpa.fsm.smc.JobFSM"), m_fsmContext(*this) {
				SDPA_LOG_DEBUG("Job state machine created");
			}

			virtual ~JobFSM()  throw () { SDPA_LOG_DEBUG("Job state machine destroyed"); }

			//transitions
			  void CancelJob();
			  void CancelJobAck();
			  void DeleteJob();
			  void JobFailed();
			  void JobFinished();
			  void QueryJobStatus();
			  void RetrieveJobResults();
			  void Dispatch();

			JobFSMContext& GetContext() { return m_fsmContext; }
		private:
			SDPA_DECLARE_LOGGER();
			JobFSMContext m_fsmContext;
	};
}}}

#endif
