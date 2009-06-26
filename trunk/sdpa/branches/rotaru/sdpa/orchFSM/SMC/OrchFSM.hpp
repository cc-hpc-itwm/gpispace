#ifndef ORCHFSMSMC_HPP
#define ORCHFSMSMC_HPP 1

#include <sdpa/orchFSM/OrchFSMInterface.hpp>
#include <sdpa/orchFSM/SMC/OrchFSM_sm.h>
#include <sdpa/logging.hpp>

namespace sdpa {
	namespace fsm {
		class OrchFSM : public OrchFSMInterface {
			public:
				typedef std::tr1::shared_ptr<OrchFSM> Ptr;

				OrchFSM() : SDPA_INIT_LOGGER("sdpa.fsm.OrchFSM"), m_fsmContext(*this) {
					SDPA_LOG_DEBUG("State machine created");
				}

				virtual ~OrchFSM() {
					SDPA_LOG_DEBUG("State machine destroyed");
				}

				sdpa::fsm::OrchFSMContext& GetContext() { return m_fsmContext; }
			private:
				SDPA_DECLARE_LOGGER();
				sdpa::fsm::OrchFSMContext m_fsmContext;
		};
	}
}

#endif
