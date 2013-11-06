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

namespace sdpa
{
  namespace fsm
  {
    namespace bmsm
    {
      DaemonFSM::DaemonFSM ( const std::string &name
                           , const sdpa::master_info_list_t& arrMasterNames
                           , unsigned int cap
                           , unsigned int rank
                           , std::string const &guiUrl
                           )
        : GenericDaemon (name, arrMasterNames, cap, rank, guiUrl)
      {}

#define DFLT_IMPL(METHOD,EVENT_TYPE)                                  \
      void DaemonFSM_::METHOD(const EVENT_TYPE&)                      \
      {                                                               \
        throw "NO IMPLEMENTATION FOR ACTION PROVIDED. PURE VIRTUAL";  \
      }

      DFLT_IMPL (action_delete_job, DeleteJobEvent)
      DFLT_IMPL (action_request_job, RequestJobEvent)
      DFLT_IMPL (action_submit_job, SubmitJobEvent)
      DFLT_IMPL (action_register_worker, WorkerRegistrationEvent)
      DFLT_IMPL (action_error_event, ErrorEvent)

#undef DFLT_IMPL

#define FORWARD(METHOD,EVENT_TYPE)                \
      void DaemonFSM::METHOD(const EVENT_TYPE& e) \
      {                                           \
    	  GenericDaemon::METHOD (e);                \
      }

      FORWARD (action_delete_job, DeleteJobEvent)
      FORWARD (action_request_job, RequestJobEvent)
      FORWARD (action_submit_job, SubmitJobEvent)
      FORWARD (action_register_worker, WorkerRegistrationEvent)
      FORWARD (action_error_event, ErrorEvent)

#undef FORWARD

#define PERFORM(METHOD,EVENT_TYPE)                \
      void DaemonFSM::METHOD()                    \
      {                                           \
        lock_type lock (mtx_);                    \
        process_event (EVENT_TYPE());             \
      }

      PERFORM (perform_ConfigOkEvent, ConfigOkEvent)
      PERFORM (perform_ConfigNokEvent, ConfigNokEvent)
      PERFORM (handleInterruptEvent, InterruptEvent)

#undef PERFORM

#define PERFORM_FORWARD(METHOD,EVENT_TYPE)          \
      void DaemonFSM::METHOD(const EVENT_TYPE* evt) \
      {                                             \
        lock_type lock (mtx_);                      \
        process_event (*evt);                       \
      }

      PERFORM_FORWARD (handleWorkerRegistrationEvent, WorkerRegistrationEvent)
      PERFORM_FORWARD (handleDeleteJobEvent, DeleteJobEvent)
      PERFORM_FORWARD (handleSubmitJobEvent, SubmitJobEvent)
      PERFORM_FORWARD (handleRequestJobEvent, RequestJobEvent)
      PERFORM_FORWARD (handleErrorEvent, ErrorEvent)

#undef PERFORM_FORWARD
    }
  }
}
