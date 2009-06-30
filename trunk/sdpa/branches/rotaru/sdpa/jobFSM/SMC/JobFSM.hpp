#ifndef JOBFSMSMC_HPP
#define JOBFSMSMC_HPP 1

#include <sdpa/daemon/JobImpl.hpp>
#include <sdpa/jobFSM/SMC/JobFSM_sm.h>
#include <sdpa/logging.hpp>

namespace sdpa { namespace fsm { namespace smc {
	class JobFSM : public sdpa::daemon::JobImpl {
		public:
			typedef std::tr1::shared_ptr<JobFSM> Ptr;

			JobFSM(const Job::job_id_t &id, const Job::job_desc_t &desc, const Job::job_id_t &parent = Job::invalid_job_id())
				: JobImpl(id, desc, parent), SDPA_INIT_LOGGER("sdpa.fsm.JobFSM"), m_fsmContext(*this) {
				SDPA_LOG_DEBUG("State machine created");
			}

			virtual ~JobFSM()  throw () { SDPA_LOG_DEBUG("State machine destroyed"); }

			JobFSMContext& GetContext() { return m_fsmContext; }
		private:
			SDPA_DECLARE_LOGGER();
			JobFSMContext m_fsmContext;
	};
}}}

#endif
