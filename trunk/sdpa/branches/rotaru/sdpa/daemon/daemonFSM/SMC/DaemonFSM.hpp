#ifndef DAEMON_FSM_SMC_HPP
#define DAEMON_FSM_SMC_HPP 1

#include <sdpa/daemon/GenericDaemon.hpp>
#include <sdpa/daemon/daemonFSM/SMC/DaemonFSM_sm.h>
#include <sdpa/logging.hpp>
#include <sdpa/memory.hpp>

namespace sdpa {
	namespace fsm {
		namespace smc {
			class DaemonFSM : public sdpa::daemon::GenericDaemon {
				public:
					typedef  sdpa::shared_ptr<DaemonFSM> ptr_t;

					DaemonFSM(const std::string& name, const std::string& outputStage, gwes::Sdpa2Gwes* pGwes = NULL) :
						SDPA_INIT_LOGGER("sdpa.fsm.smc.DaemonFSM"),
						GenericDaemon(name, outputStage, pGwes),
						m_fsmContext(*this)
						{
							SDPA_LOG_DEBUG("Daemon state machine created");
						}

					virtual ~DaemonFSM() {
						SDPA_LOG_DEBUG("Daemon State machine destroyed");
					}

					void handleDaemonEvent(const seda::IEvent::Ptr& pEvent);

					DaemonFSMContext& GetContext() { return m_fsmContext; }
				protected:
					SDPA_DECLARE_LOGGER();
					DaemonFSMContext m_fsmContext;
			};
}}}

#endif
