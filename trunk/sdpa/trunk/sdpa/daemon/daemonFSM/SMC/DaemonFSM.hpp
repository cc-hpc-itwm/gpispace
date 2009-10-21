#ifndef DAEMON_FSM_SMC_HPP
#define DAEMON_FSM_SMC_HPP 1

#include <sdpa/daemon/GenericDaemon.hpp>
#include <sdpa/daemon/daemonFSM/SMC/DaemonFSM_sm.h>
#include <sdpa/logging.hpp>
#include <sdpa/memory.hpp>
#include <boost/thread.hpp>

namespace sdpa {
	namespace fsm {
		namespace smc {
			class DaemonFSM : public sdpa::daemon::GenericDaemon {
				public:
					typedef  sdpa::shared_ptr<DaemonFSM> ptr_t;

			        typedef boost::recursive_mutex mutex_type;
			      	typedef boost::unique_lock<mutex_type> lock_type;

					DaemonFSM(const std::string& name, seda::Stage* ptrOutStage, sdpa::Sdpa2Gwes* pGwes = NULL) :
						GenericDaemon(name, ptrOutStage, pGwes),
						SDPA_INIT_LOGGER("sdpa.fsm.smc.DaemonFSM"),
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
					mutex_type mtx_;
			};
}}}

#endif
