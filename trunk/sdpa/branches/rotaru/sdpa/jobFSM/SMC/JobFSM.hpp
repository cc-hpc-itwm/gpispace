#ifndef JOBFSMSMC_HPP
#define JOBFSMSMC_HPP 1

#include <sdpa/jobFSM/JobFSMActions.hpp>
#include <sdpa/jobFSM/SMC/JobFSM_sm.h>
#include <sdpa/logging.hpp>

namespace sdpa { namespace fsm { namespace smc {
	class JobFSM : public sdpa::fsm::JobFSMActions {
		public:
			typedef std::tr1::shared_ptr<JobFSM> Ptr;

			JobFSM() : SDPA_INIT_LOGGER("sdpa.fsm.JobFSM"), m_fsmContext(*this) {
				SDPA_LOG_DEBUG("State machine created");
			}

			virtual ~JobFSM() { SDPA_LOG_DEBUG("State machine destroyed"); }

			JobFSMContext& GetContext() { return m_fsmContext; }
		private:
			SDPA_DECLARE_LOGGER();
			JobFSMContext m_fsmContext;
	};
}}}

#endif
