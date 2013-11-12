// tiberiu.rotaru@itwm.fraunhofer.de

#ifndef JOB_FSM_BMSM_HPP
#define JOB_FSM_BMSM_HPP 1

#include <sdpa/daemon/mpl.hpp>

#include <iostream>
// back-end
#include <boost/msm/back/state_machine.hpp>
//front-end
#include <boost/msm/front/state_machine_def.hpp>

#include <fhg/assert.hpp>
#include <sdpa/daemon/IAgent.hpp>
#include <sdpa/daemon/JobImpl.hpp>
#include <sdpa/logging.hpp>
#include <sdpa/types.hpp>
#include <boost/serialization/access.hpp>
#include <boost/thread.hpp>

#include <sdpa/events/JobResultsReplyEvent.hpp>
#include <sdpa/events/DeleteJobAckEvent.hpp>
//#include <boost/msm/back/favor_compile_time.hpp>

//using namespace sdpa;
//using namespace sdpa::daemon;
//using namespace sdpa::events;

namespace msm = boost::msm;
namespace mpl = boost::mpl;

char const* const state_names[] = {     "SDPA::Pending"
                                        , "SDPA::Stalled"
                                        , "SDPA::Running"
                                        , "SDPA::Finished"
                                        , "SDPA::Failed"
                                        , "SDPA::Canceling"
                                        , "SDPA::Canceled"
};

namespace sdpa {
  namespace fsm {
    namespace bmsm {
      struct MSMRescheduleEvent
      {
        MSMRescheduleEvent(sdpa::daemon::IAgent* pAgent, const sdpa::job_id_t& id)
        : m_pAgent(pAgent), m_jobId(id)
        {}
        sdpa::daemon::IAgent& agent() const { return *m_pAgent; }
        sdpa::job_id_t jobId() const { return m_jobId; }
      private:
        sdpa::daemon::IAgent* m_pAgent;
        sdpa::job_id_t m_jobId;
      };

      struct MSMStalledEvent{
        MSMStalledEvent(sdpa::daemon::IAgent* pAgent, const sdpa::job_id_t& jobId)
          : m_pAgent(pAgent), m_jobId(jobId) {}
        sdpa::daemon::IAgent* ptrAgent() const { return m_pAgent; }
        sdpa::job_id_t jobId() const { return m_jobId; }
      private:
        sdpa::daemon::IAgent* m_pAgent;
        sdpa::job_id_t m_jobId;
      };

      struct MSMResumeJobEvent
      {
        MSMResumeJobEvent(sdpa::daemon::IAgent* pAgent, const sdpa::job_id_t& jobId)
          : m_pAgent(pAgent), m_jobId(jobId) {}
        sdpa::daemon::IAgent* ptrAgent() const { return m_pAgent; }
        sdpa::job_id_t jobId() const { return m_jobId; }
      private:
        sdpa::daemon::IAgent* m_pAgent;
        sdpa::job_id_t m_jobId;
      };

      // front-end: define the FSM structure
      struct JobFSM_ : public msm::front::state_machine_def<JobFSM_>
      {
        virtual ~JobFSM_() {}

        // The list of FSM states
        struct Pending :        public msm::front::state<>{};
        struct Stalled :        public msm::front::state<>{};
        struct Running :        public msm::front::state<>{};
        struct Finished :       public msm::front::state<>{};
        struct Failed :         public msm::front::state<>{};
        struct Cancelling : 	public msm::front::state<>{};
        struct Cancelled :      public msm::front::state<>{};

        // the initial state of the JobFSM SM. Must be defined
        typedef Pending initial_state;

        virtual void action_run_job() { DLOG(TRACE, "JobFSM_::action_run_job"); }
        virtual void action_cancel_job(const sdpa::events::CancelJobEvent&) { DLOG(TRACE, "JobFSM_::action_cancel_job"); }
        virtual void action_cancel_job_from_pending(const sdpa::events::CancelJobEvent&){ DLOG(TRACE, "JobFSM_::action_cancel_job_from_pending"); }
        virtual void action_cancel_job_ack(const sdpa::events::CancelJobAckEvent&){ DLOG(TRACE, "JobFSM_::action_cancel_job_ack"); }
        virtual void action_delete_job(const sdpa::events::DeleteJobEvent&){ DLOG(TRACE, "JobFSM_::action_delete_job"); }
        virtual void action_job_failed(const sdpa::events::JobFailedEvent&){ DLOG(TRACE, "JobFSM_::action_job_failed"); }
        virtual void action_job_finished(const sdpa::events::JobFinishedEvent&){ DLOG(TRACE, "JobFSM_::action_job_finished"); }
        virtual void action_retrieve_job_results(const sdpa::events::RetrieveJobResultsEvent&){ DLOG(TRACE, "JobFSM_::action_retrieve_job_results\n"); }
        virtual void action_reschedule_job(const MSMRescheduleEvent& evt)
        {
          DLOG(TRACE, "Reschedule the job "<<evt.jobId());
          evt.agent().schedule(evt.jobId());
        }

        virtual void action_stalled_job(const MSMStalledEvent& evt)
        {
          LOG(INFO, "The job "<<evt.jobId()<<" changed its status from RUNNING to STALLED");
          if(evt.ptrAgent()) {
              // notify the job owner that the job has subtasks that are stalling
          }
        }

        virtual void action_resume_job(const MSMResumeJobEvent& evt)
        {
          LOG(INFO, "The job "<<evt.jobId()<<" changed its status from STALLED to RUNNING");
          if(evt.ptrAgent()) {
              // notify the job owner that the job makes progress
           }
        }

        typedef JobFSM_ sm; // makes transition table cleaner

        struct transition_table : mpl::vector
        <
        //      Start           Event                                   Next        		Action                Guard
        //      +---------------+---------------------------------------+----------------------+---------------------+-----
        _row<   Pending,    	MSMResumeJobEvent,                      Running >,              // no action
        a_row<  Pending,    	sdpa::events::CancelJobEvent, 		Cancelled,              &sm::action_cancel_job_from_pending >,
        a_row<  Pending,  	sdpa::events::JobFinishedEvent,         Finished,       	&sm::action_job_finished >,
        a_row<  Pending,  	sdpa::events::JobFailedEvent,           Failed,         	&sm::action_job_failed >,
        a_row<  Pending,        MSMStalledEvent,                        Stalled,                &sm::action_stalled_job >,
        //      +---------------+-------------------------------------------+-------------------+---------------------+-----
        a_row<  Stalled,	MSMResumeJobEvent,        		Running,                &sm::action_resume_job >,
        a_row<  Stalled,    	MSMRescheduleEvent,                 	Pending,                &sm::action_reschedule_job >,
        //      +---------------+-------------------------------------------+------------------+---------------------+-----
        a_row<  Running,    	sdpa::events::JobFinishedEvent,         Finished,       	&sm::action_job_finished>,
        a_row<  Running,    	sdpa::events::JobFailedEvent,           Failed,         	&sm::action_job_failed >,
        a_row<  Running,    	sdpa::events::CancelJobEvent,       	Cancelling, 		&sm::action_cancel_job >,
        a_row<  Running,    	MSMRescheduleEvent,                 	Pending,                &sm::action_reschedule_job >,
        a_row<  Running,	MSMStalledEvent,        		Stalled,                &sm::action_stalled_job >,
        //      +---------------+-------------------------------------------+-------------------+---------------------+-----
        a_irow< Finished,   	sdpa::events::DeleteJobEvent,                                   &sm::action_delete_job >,
        a_irow< Finished,   	sdpa::events::RetrieveJobResultsEvent,                      	&sm::action_retrieve_job_results >,
        //      +---------------+-------------------------------------------+-------------------+---------------------+-----
        a_irow< Failed,     	sdpa::events::DeleteJobEvent,                                   &sm::action_delete_job >,
        a_irow< Failed,     	sdpa::events::RetrieveJobResultsEvent,                  	&sm::action_retrieve_job_results >,
        //      +---------------+-------------------------------------------+-------------------+---------------------+-----
        a_row<  Cancelling, 	sdpa::events::CancelJobAckEvent,     	Cancelled, 		&sm::action_cancel_job_ack>,
        a_row<  Cancelling, 	sdpa::events::JobFinishedEvent,      	Cancelled, 		&sm::action_job_finished>,
        a_row<  Cancelling, 	sdpa::events::JobFailedEvent,           Cancelled, 		&sm::action_job_failed>,
        //      +---------------+-------------------------------------------+-------------------+---------------------+-----
        a_irow< Cancelled,  	sdpa::events::DeleteJobEvent,                                	&sm::action_delete_job >,
        a_irow< Cancelled,  	sdpa::events::RetrieveJobResultsEvent,                      	&sm::action_retrieve_job_results >
        >{};

        template <class FSM, class Event>
        void no_transition(Event const& e, FSM&, int state)
        {
          DLOG(TRACE, "no transition from state "<< state << " on event " << typeid(e).name());
        }

        typedef std::pair<sdpa::daemon::IAgent*, const sdpa::events::JobStatusReplyEvent::Ptr> FSMStatusQueryEvent;

        template <class FSM>
        void no_transition(FSMStatusQueryEvent const& evt, FSM&, int state)
        {
           DLOG(TRACE, "process event StatusQueryEvent");
           evt.first->sendEventToMaster (evt.second);
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
        		const sdpa::daemon::IAgent* pHandler = NULL,
        		const sdpa::job_id_t &parent = sdpa::job_id_t::invalid_job_id())
          : JobImpl(id, desc, pHandler, parent)
          , SDPA_INIT_LOGGER("sdpa.fsm.bmsm.JobFSM")
        {
          DLOG(TRACE, "State machine created: " << id);
        }

        virtual ~JobFSM()
        {
          DLOG(TRACE, "State machine destroyed");
        }

        void start_fsm() { start(); }

        void CancelJob(const sdpa::events::CancelJobEvent* pEvt) {lock_type lock(mtx_); process_event(*pEvt);}
        void CancelJobAck(const sdpa::events::CancelJobAckEvent* pEvt)
        {
          lock_type lock(mtx_);
          process_event(*pEvt);
        }

        void DeleteJob(const sdpa::events::DeleteJobEvent* pEvt, sdpa::daemon::IAgent* ptr_comm)
        {
          lock_type lock(mtx_);
          process_event(*pEvt);

          sdpa::events::DeleteJobAckEvent::Ptr pDelJobReply(new sdpa::events::DeleteJobAckEvent(pEvt->to(), pEvt->from(), id(), pEvt->id()) );
          ptr_comm->sendEventToMaster(pDelJobReply);
        }

        void JobFailed(const sdpa::events::JobFailedEvent* pEvt) {lock_type lock(mtx_); process_event(*pEvt);}
        void JobFinished(const sdpa::events::JobFinishedEvent* pEvt) {lock_type lock(mtx_); process_event(*pEvt);}

        void Pause(sdpa::daemon::IAgent* pAgent)
        {
          MSMStalledEvent stalledEvt(pAgent, id());
          lock_type lock(mtx_);
          process_event(stalledEvt);
        }

        void Resume(sdpa::daemon::IAgent* pAgent)
        {
          MSMResumeJobEvent resumeEvt(pAgent, id());
          lock_type lock(mtx_);
          process_event(resumeEvt);
        }

        void QueryJobStatus(const sdpa::events::QueryJobStatusEvent* pEvt, sdpa::daemon::IAgent* pDaemon )
        {
          lock_type const _ (mtx_);
          sdpa::events::JobStatusReplyEvent::Ptr const pStatReply
                                               (new sdpa::events::JobStatusReplyEvent ( pEvt->to()
                                                                                      , pEvt->from()
                                                                                      , id()
                                                                                      , getStatus()
                                                                                      , error_code()
                                                                                      , error_message()
                                                                                      )
                                               );

          process_event(FSMStatusQueryEvent(pDaemon, pStatReply));
        }

        void RetrieveJobResults(const sdpa::events::RetrieveJobResultsEvent* pEvt, sdpa::daemon::IAgent* ptr_comm)
        {
          lock_type lock(mtx_);
          process_event(*pEvt);
          const sdpa::events::JobResultsReplyEvent::Ptr pResReply( new sdpa::events::JobResultsReplyEvent( pEvt->to(), pEvt->from(), id(), result() ));
          ptr_comm->sendEventToMaster(pResReply);
        }

        void Reschedule(sdpa::daemon::IAgent*  pAgent)
        {
          MSMRescheduleEvent ReschedEvt(pAgent, id());
          lock_type lock(mtx_);
          process_event(ReschedEvt);
        }

        void Dispatch()
        {
          MSMResumeJobEvent evt(NULL, id());
          lock_type lock(mtx_);
          process_event(evt);
        }

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
          DLOG(TRACE, "current state of job " << id() << " is " << *current_state());

          if (*current_state() < 0 || *current_state() > (int)(sizeof(state_names))) {
              LOG(ERROR, "state id out of range!");
              return "unknown";
          }
          else {
              return state_names[*current_state()];
          }
        }

        bool completed()
        {
          sdpa::status_t status = getStatus();
          return status=="SDPA::Finished" || status=="SDPA::Failed" || status=="SDPA::Canceled";
        }


        bool is_running()
        {
          sdpa::status_t status = getStatus();
          return status=="SDPA::Running";
        }

        template <class Archive>
        void serialize(Archive& ar, const unsigned int)
        {
          ar & boost::serialization::base_object<JobImpl>(*this);
          ar & boost::serialization::base_object<msm::back::state_machine<JobFSM_> >(*this);
        }

        friend class boost::serialization::access;

      private:
        mutex_type mtx_;
        SDPA_DECLARE_LOGGER();
      };
    }
  }
}

//! \todo Remove this using directive. sdpa::fsm::bmsm::JobFSM is used
//! directly in multiple locations.
using namespace sdpa::fsm::bmsm;

#endif
