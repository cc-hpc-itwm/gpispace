/*
 * =====================================================================================
 *
 *       Filename:  JobFSM.hpp
 *
 *    Description:  Job meta state machine (boost::msm)
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
#ifndef JOB_FSM_BMSM_HPP
#define JOB_FSM_BMSM_HPP 1

#include <iostream>
// back-end
#include <boost/msm/back/state_machine.hpp>
//front-end
#include <boost/msm/front/state_machine_def.hpp>


#include <sdpa/daemon/IComm.hpp>
#include <sdpa/daemon/JobImpl.hpp>
#include <sdpa/logging.hpp>
#include <sdpa/types.hpp>
#include <boost/serialization/access.hpp>
#include <boost/thread.hpp>

#include <sdpa/events/JobResultsReplyEvent.hpp>
#include <sdpa/events/DeleteJobAckEvent.hpp>

//using namespace sdpa;
//using namespace sdpa::daemon;
//using namespace sdpa::events;

namespace msm = boost::msm;
namespace mpl = boost::mpl;

char const* const state_names[] = { 	"SDPA::Pending"
                                        , "SDPA::Running"
                                        , "SDPA::Finished"
                                        , "SDPA::Failed"
                                        , "SDPA::Cancelling"
                                        , "SDPA::Cancelled"
};

namespace sdpa {
  namespace fsm {
    namespace bmsm {
      struct MSMDispatchEvent{};

      // front-end: define the FSM structure
      struct JobFSM_ : public msm::front::state_machine_def<JobFSM_>
      {
        virtual ~JobFSM_() {}

        // The list of FSM states
        struct Pending : 	public msm::front::state<>{};
        struct Running : 	public msm::front::state<>{};
        struct Finished : 	public msm::front::state<>{};
        struct Failed : 	public msm::front::state<>{};
        struct Cancelling : public msm::front::state<>{};
        struct Cancelled : 	public msm::front::state<>{};

        // the initial state of the JobFSM SM. Must be defined
        typedef Pending initial_state;

        virtual void action_run_job(const sdpa::events::RunJobEvent&) { DLOG(TRACE, "JobFSM_::action_run_job"); }
        virtual void action_cancel_job(const sdpa::events::CancelJobEvent&) { DLOG(TRACE, "JobFSM_::action_cancel_job"); }
        virtual void action_cancel_job_from_pending(const sdpa::events::CancelJobEvent&){ DLOG(TRACE, "JobFSM_::action_cancel_job_from_pending"); }
        virtual void action_cancel_job_ack(const sdpa::events::CancelJobAckEvent&){ DLOG(TRACE, "JobFSM_::action_cancel_job_ack"); }
        virtual void action_delete_job(const sdpa::events::DeleteJobEvent&){ DLOG(TRACE, "JobFSM_::action_delete_job"); }
        virtual void action_job_failed(const sdpa::events::JobFailedEvent&){ DLOG(TRACE, "JobFSM_::action_job_failed"); }
        virtual void action_job_finished(const sdpa::events::JobFinishedEvent&){ DLOG(TRACE, "JobFSM_::action_job_finished"); }
        virtual void action_retrieve_job_results(const sdpa::events::RetrieveJobResultsEvent&){ DLOG(TRACE, "JobFSM_::action_retrieve_job_results\n"); }

        typedef JobFSM_ sm; // makes transition table cleaner

        struct transition_table : mpl::vector
        <
        //      Start       Event         		      Next        Action                Guard
        //      +-----------+--------------------- -+-----------+---------------------+-----
        _row<   Pending,    MSMDispatchEvent, 		Running >,
        a_row<  Pending,    sdpa::events::CancelJobEvent, 	 	Cancelled,  &sm::action_cancel_job_from_pending >,
        //      +-----------+-----------------------+-----------+---------------------+-----
        a_row<  Running,    sdpa::events::JobFinishedEvent,	 	Finished, 	&sm::action_job_finished>,
        a_row<  Running,    sdpa::events::JobFailedEvent, 	 	Failed, 	&sm::action_job_failed >,
        a_row<  Running,    sdpa::events::CancelJobEvent,      	Cancelling, &sm::action_cancel_job >,
        a_row<  Running,    sdpa::events::CancelJobAckEvent, 	Cancelled,  &sm::action_cancel_job_ack >,
        //      +-----------+-----------------------+-----------+---------------------+-----
        a_irow< Finished,   sdpa::events::DeleteJobEvent, 					&sm::action_delete_job >,
        _irow<  Finished,   sdpa::events::JobFinishedEvent >,
        a_irow< Finished,   sdpa::events::RetrieveJobResultsEvent, 			&sm::action_retrieve_job_results >,
        //      +-----------+------------------------+----------+---------------------+-----
        a_irow< Failed, 	sdpa::events::DeleteJobEvent, 		 		 	&sm::action_delete_job >,
        _irow<  Failed, 	sdpa::events::JobFailedEvent >,
        a_irow< Failed, 	sdpa::events::RetrieveJobResultsEvent, 			&sm::action_retrieve_job_results >,
        //      +-----------+------------------------+----------+---------------------+-----
        a_irow< Cancelling, sdpa::events::RetrieveJobResultsEvent, 			&sm::action_retrieve_job_results >,
        a_irow< Cancelling, sdpa::events::DeleteJobEvent, 					&sm::action_delete_job >,
        a_row<  Cancelling, sdpa::events::CancelJobAckEvent, 	 Cancelled, &sm::action_cancel_job_ack>,
        a_row<  Cancelling, sdpa::events::JobFinishedEvent, 	 Cancelled, &sm::action_job_finished>,
        a_row<  Cancelling, sdpa::events::JobFailedEvent, 		 Cancelled, &sm::action_job_failed>,
        //      +-----------+------------------------+----------+---------------------+-----
        a_irow< Cancelled,  sdpa::events::DeleteJobEvent, 		 			&sm::action_delete_job >,
        _irow<  Cancelled,  sdpa::events::CancelJobEvent >,
        a_irow< Cancelled,  sdpa::events::RetrieveJobResultsEvent,			&sm::action_retrieve_job_results >
        >{};

        template <class FSM, class Event>
        void no_transition(Event const& e, FSM&, int state)
        {
          LOG(DEBUG, "no transition from state "<< state << " on event " << typeid(e).name());
        }

        template <class FSM>
        void no_transition(sdpa::events::QueryJobStatusEvent const& e, FSM&, int state)
        {
          LOG(DEBUG, "process event QueryJobStatusEvent");
        }
      };

      // Pick a back-end
      class JobFSM : public msm::back::state_machine<JobFSM_>, public sdpa::daemon::JobImpl
      {
      public:
        typedef sdpa::shared_ptr<JobFSM> Ptr;
        typedef boost::recursive_mutex mutex_type;
        typedef boost::unique_lock<mutex_type> lock_type;

        JobFSM( const sdpa::job_id_t id = sdpa::JobId(""),
              const sdpa::job_desc_t desc = "",
              const sdpa::daemon::IComm* pHandler = NULL,
              const sdpa::job_id_t &parent = sdpa::job_id_t::invalid_job_id())
          : JobImpl(id, desc, pHandler, parent)
          , SDPA_INIT_LOGGER("sdpa.fsm.bmsm.JobFSM")
        {
          SDPA_LOG_DEBUG("State machine created");
        }

        ~JobFSM()
        {
          SDPA_LOG_DEBUG("State machine destroyed");
        }

        void start_fsm() { start(); }

        //transitions
        void CancelJob(const sdpa::events::CancelJobEvent* pEvt) {lock_type lock(mtx_); process_event(*pEvt);}
        void CancelJobAck(const sdpa::events::CancelJobAckEvent* pEvt) {lock_type lock(mtx_); process_event(*pEvt);}

        void DeleteJob(const sdpa::events::DeleteJobEvent* pEvt, sdpa::daemon::IComm*  ptr_comm)
        {
          assert (ptr_comm);
          lock_type lock(mtx_);
          process_event(*pEvt);

          sdpa::events::DeleteJobAckEvent::Ptr pDelJobReply(new sdpa::events::DeleteJobAckEvent(pEvt->to(), pEvt->from(), id(), pEvt->id()) );
          //send ack to master
          ptr_comm->sendEventToMaster(pDelJobReply);
        }

        void JobFailed(const sdpa::events::JobFailedEvent* pEvt) {lock_type lock(mtx_); process_event(*pEvt);}
        void JobFinished(const sdpa::events::JobFinishedEvent* pEvt) {lock_type lock(mtx_); process_event(*pEvt);}

        void QueryJobStatus(const sdpa::events::QueryJobStatusEvent* pEvt, sdpa::daemon::IComm* pDaemon )
        {
          assert (pDaemon);
          // attention, no action called!
          lock_type lock(mtx_);
          process_event(*pEvt);

          //LOG(TRACE, "The status of the job "<<id()<<" is " << getStatus()<<"!!!");
          sdpa::status_t status = getStatus();
          sdpa::events::JobStatusReplyEvent::Ptr
            pStatReply(new sdpa::events::JobStatusReplyEvent( pEvt->to(), pEvt->from(), id(), status));

          pDaemon->sendEventToMaster(pStatReply);
        }

        void RetrieveJobResults(const sdpa::events::RetrieveJobResultsEvent* pEvt, sdpa::daemon::IComm* ptr_comm)
        {
          assert (ptr_comm);
          lock_type lock(mtx_);
          process_event(*pEvt);
          const sdpa::events::JobResultsReplyEvent::Ptr pResReply( new sdpa::events::JobResultsReplyEvent( pEvt->to(), pEvt->from(), id(), result() ));

          // reply the results to master
          ptr_comm->sendEventToMaster(pResReply);
        }

        void Dispatch() { MSMDispatchEvent DispEvt;lock_type lock(mtx_); process_event(DispEvt);}

        // actions
        void action_run_job() {
          sdpa::daemon::JobImpl::action_run_job();
        }

        void action_cancel_job(const sdpa::events::CancelJobEvent& e) {
          sdpa::daemon::JobImpl::action_cancel_job(e);
        }

        void action_cancel_job_from_pending(const sdpa::events::CancelJobEvent& e){
          sdpa::daemon::JobImpl::action_cancel_job_from_pending(e);
        }

        void action_cancel_job_ack(const sdpa::events::CancelJobAckEvent& e) {
          sdpa::daemon::JobImpl::action_cancel_job_ack(e);
        }

        void action_delete_job(const sdpa::events::DeleteJobEvent& e){
          sdpa::daemon::JobImpl::action_delete_job(e);
        }

        void action_job_failed(const sdpa::events::JobFailedEvent& e){
          sdpa::daemon::JobImpl::action_job_failed(e);
        }
        void action_job_finished(const sdpa::events::JobFinishedEvent& e){
          sdpa::daemon::JobImpl::action_job_finished(e);
        }

        void action_retrieve_job_results(const sdpa::events::RetrieveJobResultsEvent& e){
          sdpa::daemon::JobImpl::action_retrieve_job_results(e);
        }

        sdpa::status_t getStatus()
        {
          LOG(DEBUG, "current state of job " << id() << " is " << *current_state());

          if (*current_state() < 0 || *current_state() > sizeof(state_names))
          {
            LOG(ERROR, "state id out of range!");
            return "unknown";
          }
          else
          {
            return state_names[*current_state()];
          }
        }

        template <class Archive>
        void serialize(Archive& ar, const unsigned int)
        {
          //ar.register_type(static_cast<sdpa::daemon::JobImpl*>(NULL));
          //ar.register_type(static_cast<JobFSM_*>(NULL));

          ar & boost::serialization::base_object<JobImpl>(*this);
          ar & boost::serialization::base_object<msm::back::state_machine<JobFSM_> >(*this);
          //ar & job_impl_;
        }

        friend class boost::serialization::access;

      private:
        mutex_type mtx_;

        SDPA_DECLARE_LOGGER();
      };
    }
  }
}

#endif
