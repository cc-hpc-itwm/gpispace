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
            class DaemonFSM : public sdpa::daemon::GenericDaemon
            {
                public:
                        typedef  sdpa::shared_ptr<DaemonFSM> ptr_t;
                //typedef boost::recursive_mutex mutex_type;
                //typedef boost::unique_lock<mutex_type> lock_type;

                // obsolete
                DaemonFSM( const std::string &name,
                           seda::Stage* ptrToMasterStage,
                           seda::Stage* ptrToSlaveStage,
                           IWorkflowEngine*  pArgSdpa2Gwes)
                : GenericDaemon(name, ptrToMasterStage, ptrToSlaveStage, pArgSdpa2Gwes),
                  SDPA_INIT_LOGGER(name+"FSM"),
                  m_fsmContext(*this)
                {
                  DLOG(TRACE, "Daemon state machine created");
                }

                // obsolete
                DaemonFSM( const std::string &name,
                           IWorkflowEngine*  pArgSdpa2Gwes,
                           const std::string& toMasterStageName,
                           const std::string& toSlaveStageName = std::string(""))
                : GenericDaemon(name, toMasterStageName, toSlaveStageName, pArgSdpa2Gwes),
                  SDPA_INIT_LOGGER(name+"FSM"),
                  m_fsmContext(*this)
                {
                  DLOG(TRACE, "Daemon state machine created");
                }

                DaemonFSM(  const std::string &name = "",
                            const sdpa::master_list_t& arrMasterNames = sdpa::master_list_t(),
                            unsigned int cap = 10000,
                            IWorkflowEngine* pArgSdpa2Gwes = NULL )
                : GenericDaemon(name, arrMasterNames, cap, pArgSdpa2Gwes),
                  SDPA_INIT_LOGGER(name+"FSM"),
                  m_fsmContext(*this)
                {
                  DLOG(TRACE, "Daemon state machine created");
                }

                virtual ~DaemonFSM()
                {
                  DLOG(TRACE, "Daemon State machine destroyed");
                }

                //void handleDaemonEvent(const seda::IEvent::Ptr& pEvent);void handleStartUpEvent(const StartUpEvent::Ptr& pEvent);
                virtual void handleStartUpEvent(const sdpa::events::StartUpEvent* pEvent);
                virtual void handleConfigOkEvent(const sdpa::events::ConfigOkEvent* pEvent);
                virtual void handleConfigNokEvent(const sdpa::events::ConfigNokEvent* pEvent);
                virtual void handleInterruptEvent(const sdpa::events::InterruptEvent* pEvent);
                virtual void handleWorkerRegistrationEvent(const sdpa::events::WorkerRegistrationEvent* pEvent);
                virtual void handleDeleteJobEvent(const sdpa::events::DeleteJobEvent* pEvent);
                virtual void handleSubmitJobEvent(const sdpa::events::SubmitJobEvent* pEvent);
                //virtual void handleLifeSignEvent(const sdpa::events::LifeSignEvent* pEvent);
                virtual void handleRequestJobEvent(const sdpa::events::RequestJobEvent* pEvent);
                virtual void handleConfigRequestEvent(const sdpa::events::ConfigRequestEvent* pEvent);
                virtual void handleErrorEvent(const sdpa::events::ErrorEvent* pEvent);


                sdpa::status_t getStatus();
                DaemonFSMContext& GetContext() { return m_fsmContext; }

                template<class Archive>
                void save(Archive & ar, const unsigned int) const
                {
                    int stateId(m_fsmContext.getState().getId());

                    // invoke serialization of the base class
                    ar << boost::serialization::base_object<GenericDaemon>(*this);
                    ar << stateId;
                }

                template<class Archive>
                void load(Archive & ar, const unsigned int)
                {
                    int stateId;

                    // invoke serialization of the base class
                    ar >> boost::serialization::base_object<GenericDaemon>(*this);
                    ar >> stateId;

                    m_fsmContext.setState(m_fsmContext.valueOf(stateId));
                }

                BOOST_SERIALIZATION_SPLIT_MEMBER()

                friend class boost::serialization::access;
                //friend class sdpa::tests::WorkerSerializationTest;

        protected:

                SDPA_DECLARE_LOGGER();
                DaemonFSMContext m_fsmContext;
    };
}}}

#endif
