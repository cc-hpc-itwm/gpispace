#ifndef JOB_FSM_SMC_HPP
#define JOB_FSM_SMC_HPP 1

#include <sdpa/daemon/JobImpl.hpp>
#include <sdpa/jobFSM/SMC/JobFSM_sm.h>
#include <sdpa/logging.hpp>

namespace sdpa { namespace fsm { namespace smc {
	class JobFSM : public sdpa::daemon::JobImpl {
		public:
			typedef std::tr1::shared_ptr<JobFSM> Ptr;

			JobFSM(const sdpa::job_id_t &id, const sdpa::job_desc_t &desc, const sdpa::job_id_t &parent = Job::invalid_job_id())
				: JobImpl(id, desc, parent), SDPA_INIT_LOGGER("sdpa.fsm.smc.JobFSM"), m_fsmContext(*this) {
				SDPA_LOG_DEBUG("State machine created");
			}

			virtual ~JobFSM()  throw () { SDPA_LOG_DEBUG("State machine destroyed"); }

			virtual void process_event( const boost::statechart::event_base & e) {
					std::cout <<":call 'sdpa::fsm::smc::JobFSM::process_event'"<< std::endl;
			}

			//transitions
			void CancelJob(const sdpa::events::CancelJobEvent& event);
			void CancelJobAck(const sdpa::events::CancelJobAckEvent& event);
			void DeleteJob(const sdpa::events::DeleteJobEvent& event);
			void JobFailed(const sdpa::events::JobFailedEvent& event);
			void JobFinished(const sdpa::events::JobFinishedEvent& event);
			void QueryJobStatus(const sdpa::events::QueryJobStatusEvent& event);
			void RetrieveResults(const sdpa::events::RetrieveResultsEvent& event);
			void RunJob(const sdpa::events::RunJobEvent& event);

			JobFSMContext& GetContext() { return m_fsmContext; }
		private:
			SDPA_DECLARE_LOGGER();
			JobFSMContext m_fsmContext;
	};
}}}

#endif
