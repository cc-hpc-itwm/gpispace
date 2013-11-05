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
#include <sdpa/daemon/Job.hpp>
#include <sdpa/logging.hpp>
#include <sdpa/types.hpp>
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
      struct MSMDispatchEvent{};
      struct MSMRescheduleEvent{};
      struct MSMStalledEvent{};

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

        typedef JobFSM_ sm; // makes transition table cleaner

        struct transition_table : mpl::vector
        <
        //      Start       Event                                       Next        		Action                Guard
        //      +---------------+-------------------------------------------+------------------+---------------------+-----
        _row<   Pending,    	MSMDispatchEvent,           				Running >,
        a_row<  Pending,    	sdpa::events::CancelJobEvent, 				Cancelled,          &sm::action_cancel_job_from_pending >,
        //a_row<  Pending,  	sdpa::events::JobFinishedEvent,             Finished,       	&sm::action_job_finished >,
        //a_row<  Pending,  	sdpa::events::JobFailedEvent,               Failed,         	&sm::action_job_failed >,
        //      +---------------+-------------------------------------------+-------------------+---------------------+-----
        _row<   Stalled,	    MSMDispatchEvent,        					Running >,
        _row<   Stalled,    	MSMRescheduleEvent,                 		Pending >,
        //      +---------------+-------------------------------------------+------------------+---------------------+-----
        a_row<  Running,    	sdpa::events::JobFinishedEvent,             Finished,       	&sm::action_job_finished>,
        a_row<  Running,    	sdpa::events::JobFailedEvent,               Failed,         	&sm::action_job_failed >,
        a_row<  Running,    	sdpa::events::CancelJobEvent,       		Cancelling, 		&sm::action_cancel_job >,
        _row<   Running,    	MSMRescheduleEvent,                 		Pending >,
        _row<   Running,	    MSMStalledEvent,        					Stalled >,
        //      +---------------+-------------------------------------------+-------------------+---------------------+-----
        a_irow< Finished,   	sdpa::events::DeleteJobEvent,                                   &sm::action_delete_job >,
        a_irow< Finished,   	sdpa::events::RetrieveJobResultsEvent,                      	&sm::action_retrieve_job_results >,
        //      +---------------+-------------------------------------------+-------------------+---------------------+-----
        a_irow< Failed,     	sdpa::events::DeleteJobEvent,                                   &sm::action_delete_job >,
        a_irow< Failed,     	sdpa::events::RetrieveJobResultsEvent,                  		&sm::action_retrieve_job_results >,
        //      +---------------+-------------------------------------------+-------------------+---------------------+-----
        a_row<  Cancelling, 	sdpa::events::CancelJobAckEvent,     		Cancelled, 			&sm::action_cancel_job_ack>,
        a_row<  Cancelling, 	sdpa::events::JobFinishedEvent,      		Cancelled, 			&sm::action_job_finished>,
        a_row<  Cancelling, 	sdpa::events::JobFailedEvent,               Cancelled, 			&sm::action_job_failed>,
        //      +---------------+-------------------------------------------+-------------------+---------------------+-----
        a_irow< Cancelled,  	sdpa::events::DeleteJobEvent,                                	&sm::action_delete_job >,
        a_irow< Cancelled,  	sdpa::events::RetrieveJobResultsEvent,                      	&sm::action_retrieve_job_results >
        >{};

        template <class FSM, class Event>
        void no_transition(Event const& e, FSM&, int state)
        {
        	//DLOG(WARN, "no transition from state "<< state << " on event " << typeid(e).name());
        }

        template <class FSM>
        void no_transition(sdpa::events::QueryJobStatusEvent const& e, FSM&, int state)
        {
            //DLOG(DEBUG, "process event QueryJobStatusEvent");
        }
      };

      // Pick a back-end
      class JobFSM : public msm::back::state_machine<JobFSM_>, public sdpa::daemon::Job
      {
      public:
        typedef sdpa::shared_ptr<JobFSM> Ptr;
        typedef boost::recursive_mutex mutex_type;
        typedef boost::unique_lock<mutex_type> lock_type;

        JobFSM( const sdpa::job_id_t id = sdpa::JobId(""),
        		const sdpa::job_desc_t desc = "",
        		const sdpa::daemon::IAgent* pHandler = NULL,
        		const sdpa::job_id_t &parent = sdpa::job_id_t::invalid_job_id())
          : Job(id, desc, pHandler, parent)
          , SDPA_INIT_LOGGER("sdpa.fsm.bmsm.JobFSM")
        {
        	DLOG(TRACE, "State machine created: " << id);
        }

        virtual ~JobFSM()
        {
        	DLOG(TRACE, "State machine destroyed");
        }

        void start_fsm() { start(); }

        //transitions
        void CancelJob(const sdpa::events::CancelJobEvent* pEvt) {lock_type lock(mtx_); process_event(*pEvt);}
        void CancelJobAck(const sdpa::events::CancelJobAckEvent* pEvt)
        {
        	lock_type lock(mtx_);
        	process_event(*pEvt);
        	/*BOOST_FOREACH(sdpa::worker_id_t& workerId, allocation_table[jobId])
        		{
        			lock_type lock_worker;
        			Worker::ptr_t ptrWorker = findWorker(workerId);
        			ptrWorker->free();
        		}

        		allocation_table.erase(jobId);*/
        }

        void DeleteJob(const sdpa::events::DeleteJobEvent* pEvt, sdpa::daemon::IAgent*  ptr_comm)
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

        void QueryJobStatus(const sdpa::events::QueryJobStatusEvent* pEvt, sdpa::daemon::IAgent* pDaemon )
        {
        	assert (pDaemon);
        	// attention, no action called!
        	lock_type const _ (mtx_);
        	process_event (*pEvt);

                sdpa::events::JobStatusReplyEvent::Ptr const pStatReply
                  (new sdpa::events::JobStatusReplyEvent ( pEvt->to()
                                                         , pEvt->from()
                                                         , id()
                                                         , getStatus()
                                                         , error_code()
                                                         , error_message()
                                                         )
                  );

        	pDaemon->sendEventToMaster (pStatReply);
        }

        void RetrieveJobResults(const sdpa::events::RetrieveJobResultsEvent* pEvt, sdpa::daemon::IAgent* ptr_comm)
        {
        	assert (ptr_comm);
        	lock_type lock(mtx_);
        	process_event(*pEvt);
        	const sdpa::events::JobResultsReplyEvent::Ptr pResReply( new sdpa::events::JobResultsReplyEvent( pEvt->to(), pEvt->from(), id(), result() ));

        	// reply the results to master
        	ptr_comm->sendEventToMaster(pResReply);
        }

        void Reschedule(sdpa::daemon::IAgent*  pAgent)
        {
          MSMRescheduleEvent ReschedEvt;
          lock_type lock(mtx_);
          process_event(ReschedEvt);
          pAgent->schedule(id());
        }

        void Dispatch()
        {
            MSMDispatchEvent DispEvt;
            lock_type lock(mtx_);
            process_event(DispEvt);
        }

        void Pause()
        {
        	MSMStalledEvent StalledEvt;
        	lock_type lock(mtx_);
        	process_event(StalledEvt);
        }

        // actions
        void action_run_job() {
          sdpa::daemon::Job::action_run_job();
        }

        void action_cancel_job(const sdpa::events::CancelJobEvent& e) {
          sdpa::daemon::Job::action_cancel_job(e);
        }

        void action_cancel_job_from_pending(const sdpa::events::CancelJobEvent& e){
          sdpa::daemon::Job::action_cancel_job_from_pending(e);
        }

        void action_cancel_job_ack(const sdpa::events::CancelJobAckEvent& e) {
          sdpa::daemon::Job::action_cancel_job_ack(e);
        }

        void action_delete_job(const sdpa::events::DeleteJobEvent& e){
          sdpa::daemon::Job::action_delete_job(e);
        }

        void action_job_failed(const sdpa::events::JobFailedEvent& e){
          sdpa::daemon::Job::action_job_failed(e);
        }
        void action_job_finished(const sdpa::events::JobFinishedEvent& e){
          sdpa::daemon::Job::action_job_finished(e);
        }

        void action_retrieve_job_results(const sdpa::events::RetrieveJobResultsEvent& e){
          sdpa::daemon::Job::action_retrieve_job_results(e);
        }

        sdpa::status_t getStatus()
        {
        	DLOG(TRACE, "current state of job " << id() << " is " << *current_state());

			if (*current_state() < 0 || *current_state() > (int)(sizeof(state_names)))
			{
				LOG(ERROR, "state id out of range!");
				return "unknown";
			}
			else
			{
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
