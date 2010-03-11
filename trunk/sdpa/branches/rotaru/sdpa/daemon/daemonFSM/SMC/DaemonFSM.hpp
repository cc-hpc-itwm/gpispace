/*
 * =====================================================================================
 *
 *       Filename:  DaemonFSM.hpp
 *
 *    Description:  Daemon state machine (state machine compiler)
 *
 *        Version:  1.0
 *        Created:
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Dr. Tiberiu Rotaru, tiberiu.rotaru@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */
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

			      	DaemonFSM(	const std::string &name,
								seda::Stage* ptrToMasterStage,
								seda::Stage* ptrToSlaveStage,
								IWorkflowEngine*  pArgSdpa2Gwes)
						:GenericDaemon(name, ptrToMasterStage, ptrToSlaveStage, pArgSdpa2Gwes),
						 SDPA_INIT_LOGGER(name+"FSM"),
						 m_fsmContext(*this)
					{
						SDPA_LOG_DEBUG("Daemon state machine created");
					}

			      	DaemonFSM(  const std::string &name,
								IWorkflowEngine*  pArgSdpa2Gwes,
								const std::string& toMasterStageName,
								const std::string& toSlaveStageName = std::string(""))
						: GenericDaemon(name, toMasterStageName, toSlaveStageName, pArgSdpa2Gwes),
						  SDPA_INIT_LOGGER(name+"FSM"),
						  m_fsmContext(*this)
					{
						SDPA_LOG_DEBUG("Daemon state machine created");
					}

			     	DaemonFSM( const std::string &name = "", IWorkflowEngine* pArgSdpa2Gwes = NULL )
						: GenericDaemon(name, pArgSdpa2Gwes),
						  SDPA_INIT_LOGGER(name+"FSM"),
						  m_fsmContext(*this)
					{
						SDPA_LOG_DEBUG("Daemon state machine created");
					}

					virtual ~DaemonFSM() {
						SDPA_LOG_DEBUG("Daemon State machine destroyed");
					}

					void handleDaemonEvent(const seda::IEvent::Ptr& pEvent);

					DaemonFSMContext& GetContext() { return m_fsmContext; }

					template<class Archive>
					void save(Archive & ar, const unsigned int version) const
					{
						int stateId(m_fsmContext.getState().getId());

						// invoke serialization of the base class
						ar << boost::serialization::base_object<GenericDaemon>(*this);
						ar << stateId;
					}

					template<class Archive>
					void load(Archive & ar, const unsigned int version)
					{
						int stateId;

						// invoke serialization of the base class
						ar >> boost::serialization::base_object<GenericDaemon>(*this);
						ar >> stateId;

						m_fsmContext.setState(m_fsmContext.valueOf(stateId));
					}

					template<class Archive>
					void serialize( Archive & ar, const unsigned int file_version )
					{
						boost::serialization::split_member(ar, *this, file_version);
					}

					friend class boost::serialization::access;
					//friend class sdpa::tests::WorkerSerializationTest;

				protected:

					SDPA_DECLARE_LOGGER();
					DaemonFSMContext m_fsmContext;
					mutex_type mtx_;
			};
}}}

#endif
