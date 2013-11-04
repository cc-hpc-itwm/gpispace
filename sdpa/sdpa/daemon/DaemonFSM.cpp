/*
 * =====================================================================================
 *
 *       Filename:  DaemonFSM.cpp
 *
 *    Description:  Daemon meta state machine (boost::msm)
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

#include <sdpa/daemon/DaemonFSM.hpp>

using namespace sdpa;
using namespace sdpa::daemon;
using namespace sdpa::events;

namespace msm = boost::msm;
namespace mpl = boost::mpl;

namespace sdpa {
  namespace fsm {
    namespace bmsm {

      void DaemonFSM_::action_delete_job(const DeleteJobEvent& )
      {
    	  DLOG(TRACE, "DaemonFSM_::action_delete_job");
      }

      void DaemonFSM_::action_request_job(const RequestJobEvent& )
      {
    	  DLOG(TRACE, "DaemonFSM_::action_request_job");
      }

      void DaemonFSM_::action_submit_job(const SubmitJobEvent& )
      {
    	  DLOG(TRACE, "DaemonFSM_::action_submit_job");
      }

      void DaemonFSM_::action_config_request(const ConfigRequestEvent& )
      {
    	  DLOG(TRACE, "DaemonFSM_::action_config_request");
      }

      void DaemonFSM_::action_register_worker(const WorkerRegistrationEvent& )
      {
    	  DLOG(TRACE, "DaemonFSM_::action_register_worker");
      }

      void DaemonFSM_::action_error_event(const ErrorEvent& )
      {
    	  DLOG(TRACE, "DaemonFSM_::action_error_event");
      }

      DaemonFSM::DaemonFSM( const std::string &name,
                            const sdpa::master_info_list_t& arrMasterNames,
                            unsigned int cap,
                            unsigned int rank
                          , std::string const &guiUrl
                          )
        : GenericDaemon(name, arrMasterNames, cap, rank, guiUrl)
      {
    	  DLOG(TRACE, "Daemon state machine created");
      }

      DaemonFSM::~DaemonFSM()
      {
    	  DLOG(TRACE, "Daemon State machine destroyed");
      }

      void DaemonFSM::action_delete_job(const DeleteJobEvent& e)
      {
    	  DLOG(TRACE, "DaemonFSM::action_delete_job");
    	  GenericDaemon::action_delete_job(e);
      }
      void DaemonFSM::action_request_job(const RequestJobEvent& e)
      {
    	  DLOG(TRACE, "DaemonFSM::action_request_job");
    	  GenericDaemon::action_request_job(e);
      }

      void DaemonFSM::action_submit_job(const SubmitJobEvent& e)
      {
    	  DLOG(TRACE, "DaemonFSM::action_submit_job");
    	  GenericDaemon::action_submit_job(e);
      }

      void DaemonFSM::action_config_request(const ConfigRequestEvent& e)
      {
    	  DLOG(TRACE, "DaemonFSM::action_config_request");
    	  GenericDaemon::action_config_request(e);
      }

      void DaemonFSM::action_register_worker(const WorkerRegistrationEvent& e)
      {
    	  DLOG(TRACE, "DaemonFSM::action_register_worker");
    	  GenericDaemon::action_register_worker(e);
      }

      void DaemonFSM::action_error_event(const ErrorEvent& e)
      {
    	  DLOG(TRACE, "DaemonFSM::action_error_event");
    	  GenericDaemon::action_error_event(e);
      }

      void DaemonFSM::perform_StartUpEvent()
      {
        lock_type lock (mtx_);
        process_event (StartUpEvent());
      }
      void DaemonFSM::perform_ConfigOkEvent()
      {
        lock_type lock (mtx_);
        process_event (ConfigOkEvent());
      }
      void DaemonFSM::perform_ConfigNokEvent()
      {
        lock_type lock (mtx_);
        process_event (ConfigNokEvent());
      }

      void DaemonFSM::handleInterruptEvent()
      {
        lock_type lock(mtx_);
        process_event(InterruptEvent());
      }

      void DaemonFSM::handleWorkerRegistrationEvent(const WorkerRegistrationEvent* pEvent)
      {
    	  lock_type lock(mtx_);
    	  //SDPA_LOG_DEBUG("Process WorkerRegistrationEvent");
    	  process_event(*pEvent);
      }

      void DaemonFSM::handleDeleteJobEvent(const DeleteJobEvent* pEvent)
      {
    	  lock_type lock(mtx_);
    	  //SDPA_LOG_DEBUG("Process DeleteJobEvent");
    	  process_event(*pEvent);
      }

      void DaemonFSM::handleSubmitJobEvent(const SubmitJobEvent* pEvent)
      {
    	  lock_type lock(mtx_);
    	  //SDPA_LOG_DEBUG("Process SubmitJobEvent");
    	  process_event(*pEvent);
      }

      void DaemonFSM::handleRequestJobEvent(const RequestJobEvent* pEvent)
      {
    	  lock_type lock(mtx_);
    	  //SDPA_LOG_DEBUG("Process RequestJobEvent");
    	  process_event(*pEvent);
      }

      void DaemonFSM::handleConfigRequestEvent(const ConfigRequestEvent* pEvent)
      {
    	  lock_type lock(mtx_);
    	  //SDPA_LOG_DEBUG("Process ConfigRequestEvent");
    	  process_event(*pEvent);
      }

      void DaemonFSM::handleErrorEvent(const ErrorEvent* pEvent)
      {
    	  lock_type lock(mtx_);
    	  //SDPA_LOG_DEBUG("Process ErrorEvent");
    	  process_event(*pEvent);
      }
    }
  }
}
