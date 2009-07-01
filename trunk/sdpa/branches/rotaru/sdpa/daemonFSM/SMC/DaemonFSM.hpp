#ifndef DAEMON_FSM_SMC_HPP
#define DAEMON_FSM_SMC_HPP 1

#include <sdpa/daemon/GenericDaemon.hpp>
#include <sdpa/daemonFSM/SMC/DaemonFSM_sm.h>
#include <sdpa/logging.hpp>

namespace sdpa {
	namespace fsm {
		namespace smc {
			class DaemonFSM : public sdpa::daemon::GenericDaemon {
				public:
					typedef std::tr1::shared_ptr<DaemonFSM> Ptr;

					DaemonFSM(const std::string& name, const std::string& outputStage) :
						SDPA_INIT_LOGGER("sdpa.fsm.smc.DaemonFSM"), GenericDaemon(name, outputStage), m_fsmContext(*this) {
							SDPA_LOG_DEBUG("State machine created");
						}

					virtual ~DaemonFSM() {
						SDPA_LOG_DEBUG("State machine destroyed");
					}

					DaemonFSMContext& GetContext() { return m_fsmContext; }
				private:
					SDPA_DECLARE_LOGGER();
					DaemonFSMContext m_fsmContext;
			};
}}}

#endif
