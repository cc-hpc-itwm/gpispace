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
				SDPA_LOG_DEBUG("State machine created");
			}

			virtual ~JobFSM()  throw () { SDPA_LOG_DEBUG("State machine destroyed"); }

			//transitions
			void process_event(const sdpa::events::CancelJobEvent& event);
			void process_event(const sdpa::events::CancelJobAckEvent& event);
			void process_event(const sdpa::events::DeleteJobEvent& event);
			void process_event(const sdpa::events::JobFailedEvent& event);
			void process_event(const sdpa::events::JobFinishedEvent& event);
			void process_event(const sdpa::events::QueryJobStatusEvent& event);
			void process_event(const sdpa::events::RetrieveJobResultsEvent& event);
			void process_event(const sdpa::events::SubmitJobEvent& event);

			JobFSMContext& GetContext() { return m_fsmContext; }
		private:
			SDPA_DECLARE_LOGGER();
			JobFSMContext m_fsmContext;
	};
}}}

#endif
