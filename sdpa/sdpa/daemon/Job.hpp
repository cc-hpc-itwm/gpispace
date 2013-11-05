#ifndef SDPA_JOB_HPP
#define SDPA_JOB_HPP 1

#include <sdpa/daemon/mpl.hpp>
#include <string>
#include <vector>
#include <utility>

#include <sdpa/memory.hpp>
#include <sdpa/types.hpp>
#include <sdpa/util/Properties.hpp>

#include <sdpa/events/SubmitJobAckEvent.hpp>
#include <sdpa/events/JobFailedEvent.hpp>
#include <sdpa/events/QueryJobStatusEvent.hpp>
#include <sdpa/events/JobStatusReplyEvent.hpp>
#include <sdpa/events/CancelJobEvent.hpp>
#include <sdpa/events/CancelJobAckEvent.hpp>
#include <sdpa/events/DeleteJobEvent.hpp>
#include <sdpa/events/JobFinishedEvent.hpp>
#include <sdpa/events/ErrorEvent.hpp>
#include <sdpa/events/RetrieveJobResultsEvent.hpp>
#include <sdpa/types.hpp>

#include <sdpa/common.hpp>
#include <boost/thread.hpp>

#include <boost/unordered_map.hpp>

#include <sdpa/daemon/mpl.hpp>

#include <iostream>

#include <boost/msm/back/state_machine.hpp>
#include <boost/msm/front/state_machine_def.hpp>

#include <fhg/assert.hpp>
#include <sdpa/daemon/Job.hpp>
#include <sdpa/logging.hpp>
#include <sdpa/types.hpp>
#include <boost/thread.hpp>

#include <sdpa/events/JobResultsReplyEvent.hpp>
#include <sdpa/events/DeleteJobAckEvent.hpp>

namespace sdpa {
  namespace daemon {

    // front-end: define the FSM structure
    struct JobFSM_ : public boost::msm::front::state_machine_def<JobFSM_>
    {
      virtual ~JobFSM_() {}

      // The list of FSM states
      struct Pending :        public boost::msm::front::state<>{};
      struct Stalled :        public boost::msm::front::state<>{};
      struct Running :        public boost::msm::front::state<>{};
      struct Finished :       public boost::msm::front::state<>{};
      struct Failed :         public boost::msm::front::state<>{};
      struct Cancelling : 	  public boost::msm::front::state<>{};
      struct Cancelled :      public boost::msm::front::state<>{};

      struct MSMDispatchEvent {};
      struct MSMRescheduleEvent {};
      struct MSMStalledEvent {};

      // the initial state of the JobFSM SM. Must be defined
      typedef Pending initial_state;

      virtual void action_delete_job(const sdpa::events::DeleteJobEvent&){ DLOG(TRACE, "JobFSM_::action_delete_job"); }
      virtual void action_job_failed(const sdpa::events::JobFailedEvent&){ DLOG(TRACE, "JobFSM_::action_job_failed"); }
      virtual void action_job_finished(const sdpa::events::JobFinishedEvent&){ DLOG(TRACE, "JobFSM_::action_job_finished"); }

      typedef JobFSM_ sm; // makes transition table cleaner

      struct transition_table : boost::mpl::vector
        <
        //      Start       Event                                       Next        		Action                Guard
        //      +---------------+-------------------------------------------+------------------+---------------------+-----
        _row<   Pending,    	MSMDispatchEvent,           				Running >,
        _row<   Pending,    	sdpa::events::CancelJobEvent, 				Cancelled>,
        //a_row<  Pending,  	sdpa::events::JobFinishedEvent,             Finished,       	&sm::action_job_finished >,
        //a_row<  Pending,  	sdpa::events::JobFailedEvent,               Failed,         	&sm::action_job_failed >,
        //      +---------------+-------------------------------------------+-------------------+---------------------+-----
        _row<   Stalled,	    MSMDispatchEvent,        					Running >,
        _row<   Stalled,    	MSMRescheduleEvent,                 		Pending >,
        //      +---------------+-------------------------------------------+------------------+---------------------+-----
        a_row<  Running,    	sdpa::events::JobFinishedEvent,             Finished,       	&sm::action_job_finished>,
        a_row<  Running,    	sdpa::events::JobFailedEvent,               Failed,         	&sm::action_job_failed >,
        _row<   Running,    	sdpa::events::CancelJobEvent,       		Cancelling>,
        _row<   Running,    	MSMRescheduleEvent,                 		Pending >,
        _row<   Running,	    MSMStalledEvent,        					Stalled >,
        //      +---------------+-------------------------------------------+-------------------+---------------------+-----
        a_irow< Finished,   	sdpa::events::DeleteJobEvent,                                   &sm::action_delete_job >,
        _irow<  Finished,   	sdpa::events::RetrieveJobResultsEvent>,
        //      +---------------+-------------------------------------------+-------------------+---------------------+-----
        a_irow< Failed,     	sdpa::events::DeleteJobEvent,                                   &sm::action_delete_job >,
        _irow<  Failed,     	sdpa::events::RetrieveJobResultsEvent>,
        //      +---------------+-------------------------------------------+-------------------+---------------------+-----
        _row<   Cancelling, 	sdpa::events::CancelJobAckEvent,     		Cancelled>,
        a_row<  Cancelling, 	sdpa::events::JobFinishedEvent,      		Cancelled, 			&sm::action_job_finished>,
        a_row<  Cancelling, 	sdpa::events::JobFailedEvent,               Cancelled, 			&sm::action_job_failed>,
        //      +---------------+-------------------------------------------+-------------------+---------------------+-----
        a_irow< Cancelled,  	sdpa::events::DeleteJobEvent,                                	&sm::action_delete_job >,
        _irow<  Cancelled,  	sdpa::events::RetrieveJobResultsEvent>
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


    class IAgent;
    class Job
    {
    public:
      typedef Job* ptr_t;

      enum job_type {MASTER, LOCAL, WORKER, TMP};

      typedef boost::unordered_map<sdpa::job_id_t, Job::ptr_t> job_list_t;
      typedef boost::recursive_mutex mutex_type;
      typedef boost::unique_lock<mutex_type> lock_type;

      Job ( const sdpa::job_id_t id
          , const sdpa::job_desc_t desc
          , const sdpa::job_id_t &parent
          );

      virtual ~Job() {}

      virtual const sdpa::job_id_t& id() const;
      virtual const sdpa::job_id_t& parent() const;
      virtual const sdpa::job_desc_t& description() const;
      virtual const sdpa::job_result_t& result() const { return result_; }

      int error_code() const {return m_error_code;}
      std::string const & error_message () const { return m_error_message;}

      Job& error_code(int ec)
      {
        m_error_code = ec;
        return *this;
      }

      Job& error_message(std::string const &msg)
      {
        m_error_message = msg;
        return *this;
      }

      virtual bool is_marked_for_deletion();
      virtual bool mark_for_deletion();

      bool isMasterJob();
      void setType(const job_type& );
      virtual job_type type() { return type_;}

      virtual void set_owner(const sdpa::worker_id_t& owner) { m_owner = owner; }
      virtual sdpa::worker_id_t owner() { return m_owner; }

      virtual bool completed() = 0;
      virtual bool is_running() = 0;

      virtual unsigned long &walltime() { return walltime_;}

      // job FSM actions
      virtual void action_delete_job(const sdpa::events::DeleteJobEvent&);
      virtual void action_job_failed(const sdpa::events::JobFailedEvent&);
      virtual void action_job_finished(const sdpa::events::JobFinishedEvent&);

      virtual void setResult(const sdpa::job_result_t& arg_results) { result_ = arg_results; }

      virtual std::string print_info()
      {
        std::ostringstream os;
        os<<std::endl;
        os<<"id: "<<id_<<std::endl;
        os<<"type: "<<type_<<std::endl;
        os<<"status: "<<getStatus()<<std::endl;
        os<<"parent: "<<parent_<<std::endl;
        os<<"error-code: " << m_error_code << std::endl;
        os<<"error-message: \"" << m_error_message << "\"" << std::endl;
        //os<<"description: "<<desc_<<std::endl;

        return os.str();
      }

      //transitions (implemented in JobFSM)
      virtual void CancelJob(const sdpa::events::CancelJobEvent*) = 0;
      virtual void CancelJobAck(const sdpa::events::CancelJobAckEvent*) = 0;
      virtual void DeleteJob(const sdpa::events::DeleteJobEvent*, sdpa::daemon::IAgent*) = 0;
      virtual void JobFailed(const sdpa::events::JobFailedEvent*) = 0;
      virtual void JobFinished(const sdpa::events::JobFinishedEvent*) = 0;
      virtual void QueryJobStatus(const sdpa::events::QueryJobStatusEvent*, sdpa::daemon::IAgent* ) = 0;
      virtual void RetrieveJobResults(const sdpa::events::RetrieveJobResultsEvent*, sdpa::daemon::IAgent*) = 0;
      virtual void Dispatch() = 0;
      virtual void Reschedule(sdpa::daemon::IAgent*) = 0;
      virtual void Pause() = 0;

      virtual sdpa::status_t getStatus() = 0;

    protected:
      SDPA_DECLARE_LOGGER();

    private:
      sdpa::job_id_t id_;
      sdpa::job_desc_t desc_;
      sdpa::job_id_t parent_;

      bool b_marked_for_del_;
      job_type type_;
      sdpa::job_result_t result_;
      int m_error_code;
      std::string m_error_message;
      unsigned long walltime_;

      sdpa::worker_id_t m_owner;
    };

      // Pick a back-end
      class JobFSM : public boost::msm::back::state_machine<JobFSM_>, public sdpa::daemon::Job
      {
      public:
        typedef sdpa::shared_ptr<JobFSM> Ptr;
        typedef boost::recursive_mutex mutex_type;
        typedef boost::unique_lock<mutex_type> lock_type;

        JobFSM ( const sdpa::job_id_t id
               , const sdpa::job_desc_t desc
               , const sdpa::job_id_t &parent
               )
          : Job(id, desc, parent)
          , SDPA_INIT_LOGGER("sdpa.fsm.bmsm.JobFSM")
        {
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

        void DeleteJob(const sdpa::events::DeleteJobEvent* pEvt, sdpa::daemon::IAgent*  ptr_comm);
        void JobFailed(const sdpa::events::JobFailedEvent* pEvt) {lock_type lock(mtx_); process_event(*pEvt);}
        void JobFinished(const sdpa::events::JobFinishedEvent* pEvt) {lock_type lock(mtx_); process_event(*pEvt);}
        void QueryJobStatus(const sdpa::events::QueryJobStatusEvent* pEvt, sdpa::daemon::IAgent* pDaemon );
        void RetrieveJobResults(const sdpa::events::RetrieveJobResultsEvent* pEvt, sdpa::daemon::IAgent* ptr_comm);
        void Reschedule(sdpa::daemon::IAgent*  pAgent);

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
        void action_delete_job(const sdpa::events::DeleteJobEvent& e){
          sdpa::daemon::Job::action_delete_job(e);
        }

        void action_job_failed(const sdpa::events::JobFailedEvent& e){
          sdpa::daemon::Job::action_job_failed(e);
        }
        void action_job_finished(const sdpa::events::JobFinishedEvent& e){
          sdpa::daemon::Job::action_job_finished(e);
        }

        sdpa::status_t getStatus()
        {
        	DLOG(TRACE, "current state of job " << id() << " is " << *current_state());

static char const* const state_names[] = {     "SDPA::Pending"
										, "SDPA::Stalled"
                                        , "SDPA::Running"
                                        , "SDPA::Finished"
                                        , "SDPA::Failed"
                                        , "SDPA::Canceling"
                                        , "SDPA::Canceled"
};

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

}}

#endif
