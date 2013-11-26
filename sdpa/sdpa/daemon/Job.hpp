#ifndef SDPA_JOB_HPP
#define SDPA_JOB_HPP 1

#include <sdpa/common.hpp>
#include <sdpa/events/CancelJobAckEvent.hpp>
#include <sdpa/events/CancelJobEvent.hpp>
#include <sdpa/events/DeleteJobAckEvent.hpp>
#include <sdpa/events/DeleteJobEvent.hpp>
#include <sdpa/events/ErrorEvent.hpp>
#include <sdpa/events/JobFailedEvent.hpp>
#include <sdpa/events/JobFinishedEvent.hpp>
#include <sdpa/events/JobResultsReplyEvent.hpp>
#include <sdpa/events/JobRunningEvent.hpp>
#include <sdpa/events/JobStalledEvent.hpp>
#include <sdpa/events/JobStatusReplyEvent.hpp>
#include <sdpa/events/QueryJobStatusEvent.hpp>
#include <sdpa/events/RetrieveJobResultsEvent.hpp>
#include <sdpa/events/SubmitJobAckEvent.hpp>
#include <sdpa/job_states.hpp>
#include <sdpa/logging.hpp>
#include <sdpa/memory.hpp>
#include <sdpa/types.hpp>

#include <fhg/assert.hpp>

#include <boost/msm/back/state_machine.hpp>
#include <boost/msm/front/state_machine_def.hpp>
#include <boost/thread.hpp>
#include <boost/unordered_map.hpp>

#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace sdpa {
  namespace daemon {
    class IAgent;
    class SchedulerBase;

    // front-end: define the FSM structure
    struct JobFSM_ : public boost::msm::front::state_machine_def<JobFSM_>
    {
      virtual ~JobFSM_() {}

      // The list of FSM states
      struct Pending :  public boost::msm::front::state<>{};
      struct Stalled :  public boost::msm::front::state<>{};
      struct Running :  public boost::msm::front::state<>{};
      struct Finished : public boost::msm::front::state<>{};
      struct Failed :   public boost::msm::front::state<>{};
      struct Canceling :public boost::msm::front::state<>{};
      struct Canceled : public boost::msm::front::state<>{};

      struct MSMRescheduleEvent
      {
        MSMRescheduleEvent(sdpa::daemon::SchedulerBase* pSched, const sdpa::job_id_t& id)
        : m_pScheduler(pSched), m_jobId(id)
        {}
        sdpa::daemon::SchedulerBase* ptrScheduler() const { return m_pScheduler; }
        sdpa::job_id_t jobId() const { return m_jobId; }
      private:
        sdpa::daemon::SchedulerBase* m_pScheduler;
        sdpa::job_id_t m_jobId;
      };
      struct MSMStalledEvent{
        MSMStalledEvent(sdpa::daemon::IAgent* pAgent, const sdpa::job_id_t& jobId, const sdpa::worker_id_t& jobOwner)
          : m_pAgent(pAgent), m_jobId(jobId), m_jobOwner (jobOwner) {}
        sdpa::daemon::IAgent* ptrAgent() const { return m_pAgent; }
        sdpa::job_id_t jobId() const { return m_jobId; }
        sdpa::worker_id_t jobOwner() const { return m_jobOwner; }
      private:
        sdpa::daemon::IAgent* m_pAgent;
        sdpa::job_id_t m_jobId;
        sdpa::worker_id_t m_jobOwner;
      };
      struct MSMResumeJobEvent
      {
        MSMResumeJobEvent(sdpa::daemon::IAgent* pAgent, const sdpa::job_id_t& jobId, const sdpa::worker_id_t& jobOwner)
          : m_pAgent(pAgent), m_jobId(jobId), m_jobOwner (jobOwner)  {}
        sdpa::daemon::IAgent* ptrAgent() const { return m_pAgent; }
        sdpa::job_id_t jobId() const { return m_jobId; }
        sdpa::worker_id_t jobOwner() const { return m_jobOwner; }
      private:
        sdpa::daemon::IAgent* m_pAgent;
        sdpa::job_id_t m_jobId;
        sdpa::worker_id_t m_jobOwner;
      };

      struct MSMDeleteJobEvent
      {
        MSMDeleteJobEvent(const sdpa::events::DeleteJobEvent* pEvt, sdpa::daemon::IAgent* pAgent)
        : m_pDelEvt(pEvt), m_pAgent(pAgent)
        {}
        sdpa::events::SDPAEvent::address_t to() const { return m_pDelEvt->from();}
        sdpa::events::SDPAEvent::address_t from() const { return m_pDelEvt->to();}
        job_id_t jobId() const { return m_pDelEvt->job_id(); }
        sdpa::daemon::IAgent* ptrAgent() const { return m_pAgent; }
      private:
        const sdpa::events::DeleteJobEvent* m_pDelEvt;
        sdpa::daemon::IAgent* m_pAgent;
      };

      struct MSMRetrieveJobResultsEvent
      {
        MSMRetrieveJobResultsEvent(const sdpa::events::RetrieveJobResultsEvent* pEvt, sdpa::daemon::IAgent* pAgent, const sdpa::job_result_t& result)
        : m_pRetResEvt(pEvt), m_pAgent(pAgent), m_strResult(result)
        {}
        sdpa::events::SDPAEvent::address_t to() const { return m_pRetResEvt->from();}
        sdpa::events::SDPAEvent::address_t from() const { return m_pRetResEvt->to();}
        job_id_t jobId() const { return m_pRetResEvt->job_id(); }
        sdpa::daemon::IAgent* ptrAgent() const { return m_pAgent; }
        const sdpa::job_result_t result() const { return m_strResult; }
      private:
        const sdpa::events::RetrieveJobResultsEvent* m_pRetResEvt;
        sdpa::daemon::IAgent* m_pAgent;
        const sdpa::job_result_t m_strResult;
      };


      // the initial state of the JobFSM SM. Must be defined
      typedef Pending initial_state;

      virtual void action_job_failed(const sdpa::events::JobFailedEvent&) = 0;
      virtual void action_job_finished(const sdpa::events::JobFinishedEvent&) = 0;
      virtual void action_reschedule_job(const MSMRescheduleEvent& evt);
      virtual void action_job_stalled(const MSMStalledEvent& evt);
      virtual void action_resume_job(const MSMResumeJobEvent& evt);
      virtual void action_delete_job(const MSMDeleteJobEvent&);
      virtual void action_retrieve_job_results(const MSMRetrieveJobResultsEvent& evt);

      typedef JobFSM_ sm; // makes transition table cleaner

      struct transition_table : boost::mpl::vector
        <
        //      Start           Event                                   Next           Action                Guard
        //      +---------------+---------------------------------------+--------------+---------------------+-----
        _row<   Pending,        MSMResumeJobEvent,                      Running >,
        _row<   Pending,        sdpa::events::CancelJobEvent, 		Canceled>,
        a_row<  Pending,        sdpa::events::JobFinishedEvent,         Finished,       &sm::action_job_finished >,
        a_row<  Pending,        sdpa::events::JobFailedEvent,           Failed,         &sm::action_job_failed >,
        a_row<  Pending,        MSMStalledEvent,                        Stalled,        &sm::action_job_stalled >,
        //      +---------------+-------------------------------------------+-------------------+---------------------+-----
        a_row<  Stalled,        MSMResumeJobEvent,        		Running,        &sm::action_resume_job >,
        a_row<  Stalled,        MSMRescheduleEvent,                 	Pending,        &sm::action_reschedule_job >,
        _irow<  Stalled,        MSMStalledEvent>,
        _row<   Stalled,        sdpa::events::CancelJobEvent,           Canceling>,
        //      +---------------+-------------------------------------------+------------------+---------------------+-----
        a_row<  Running,        sdpa::events::JobFinishedEvent,         Finished,       &sm::action_job_finished>,
        a_row<  Running,        sdpa::events::JobFailedEvent,           Failed,         &sm::action_job_failed >,
        _row<   Running,        sdpa::events::CancelJobEvent,           Canceling>,
        a_row<  Running,        MSMRescheduleEvent,                 	Pending,        &sm::action_reschedule_job >,
        a_row<  Running,        MSMStalledEvent,        		Stalled,        &sm::action_job_stalled >,
        _irow<  Running,        MSMResumeJobEvent>,
        //      +---------------+---------------------------------------+-------------------+---------------------+-----
        a_irow< Finished,       MSMDeleteJobEvent,                                      &sm::action_delete_job >,
        a_irow< Finished,   	MSMRetrieveJobResultsEvent,                             &sm::action_retrieve_job_results>,
        _irow<  Finished,       sdpa::events::JobFinishedEvent>,
        //      +---------------+---------------------------------------+-------------------+---------------------+-----
        a_irow< Failed,     	MSMDeleteJobEvent,                                      &sm::action_delete_job>,
        a_irow< Failed,     	MSMRetrieveJobResultsEvent,                             &sm::action_retrieve_job_results>,
        _irow<  Failed,         sdpa::events::JobFailedEvent>,
        //      +---------------+---------------------------------------+-------------------+---------------------+-----
        _row<   Canceling, 	sdpa::events::CancelJobAckEvent,     	Canceled>,
        a_row<  Canceling, 	sdpa::events::JobFinishedEvent,      	Canceled,       &sm::action_job_finished>,
        a_row<  Canceling, 	sdpa::events::JobFailedEvent,           Canceled,       &sm::action_job_failed>,
        _irow<  Canceling,      sdpa::events::CancelJobEvent>,
        //      +---------------+-------------------------------------------+-------------------+---------------------+-----
        a_irow< Canceled,       MSMDeleteJobEvent,                                      &sm::action_delete_job>,
        a_irow< Canceled,       MSMRetrieveJobResultsEvent,                             &sm::action_retrieve_job_results>,
        _irow<  Canceled,       sdpa::events::CancelJobAckEvent>
        >{};

      //! \note This table refers to the order in which states are
      //! first seen in the state machine definition. This is hacky
      //! and should be removed / done via visitors.
      static sdpa::status::code state_code (size_t state)
      {
        static sdpa::status::code const state_codes[] =
          { sdpa::status::PENDING
          , sdpa::status::STALLED
          , sdpa::status::RUNNING
          , sdpa::status::FINISHED
          , sdpa::status::FAILED
          , sdpa::status::CANCELING
          , sdpa::status::CANCELED
          };
        fhg_assert ( state < sizeof (state_codes) / sizeof (*state_codes)
                   , "index shall be valid"
                   );
        return state_codes[state];
      }

      template <class FSM, class Event>
      void no_transition(Event const& e, FSM&, int state)
      {
        DLOG(WARN, "no transition from state "<< sdpa::status::show(state_code(state)) << " on event " << typeid(e).name());
      }
    };

    class Job : public boost::msm::back::state_machine<JobFSM_>
    {
    public:
      typedef Job* ptr_t;

      typedef boost::unordered_map<sdpa::job_id_t, Job::ptr_t> job_list_t;
      typedef boost::recursive_mutex mutex_type;
      typedef boost::unique_lock<mutex_type> lock_type;

      Job ( const sdpa::job_id_t id
          , const sdpa::job_desc_t desc
          , const sdpa::job_id_t &parent
          , bool is_master_job
          );

      const sdpa::job_id_t& id() const;
      const sdpa::job_id_t& parent() const;
      const sdpa::job_desc_t& description() const;
      const sdpa::job_result_t& result() const;

      int error_code() const;
      std::string error_message () const;

      Job& error_code(int ec);
      Job& error_message(std::string const &msg);

      bool isMasterJob() const;

      void set_owner(const sdpa::worker_id_t& owner);
      sdpa::worker_id_t owner() const;

      sdpa::status::code getStatus() const;

      bool completed() const;
      bool is_running() const;

      // job FSM actions
      virtual void action_job_failed(const sdpa::events::JobFailedEvent&);
      virtual void action_job_finished(const sdpa::events::JobFinishedEvent&);

      void setResult(const sdpa::job_result_t& arg_results);

      //transitions
      void CancelJob(const sdpa::events::CancelJobEvent*);
      void CancelJobAck(const sdpa::events::CancelJobAckEvent*);
      void JobFailed(const sdpa::events::JobFailedEvent*);
      void JobFinished(const sdpa::events::JobFinishedEvent*);

      void DeleteJob(const sdpa::events::DeleteJobEvent*, sdpa::daemon::IAgent*);
      void RetrieveJobResults(const sdpa::events::RetrieveJobResultsEvent* pEvt, sdpa::daemon::IAgent*);
      void Reschedule(sdpa::daemon::SchedulerBase*);

      void Dispatch();
      void Pause(sdpa::daemon::IAgent*);
      void Resume(sdpa::daemon::IAgent*);

    private:
      mutable mutex_type mtx_;
      sdpa::job_id_t id_;
      sdpa::job_desc_t desc_;
      sdpa::job_id_t parent_;

      bool _is_master_job;
      sdpa::job_result_t result_;
      int m_error_code;
      std::string m_error_message;

      sdpa::worker_id_t m_owner;
    };
}}

#endif
