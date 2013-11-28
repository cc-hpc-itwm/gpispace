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
    class GenericDaemon;
    class SchedulerBase;

    // front-end: define the FSM structure
    struct JobFSM_ : public boost::msm::front::state_machine_def<JobFSM_>
    {
      virtual ~JobFSM_() {}

      // The list of FSM states
      struct Pending : public boost::msm::front::state<>{};
      struct Stalled : public boost::msm::front::state<>{};
      struct Running : public boost::msm::front::state<>{};
      struct Finished : public boost::msm::front::state<>{};
      struct Failed : public boost::msm::front::state<>{};
      struct Canceling : public boost::msm::front::state<>{};
      struct Canceled : public boost::msm::front::state<>{};

      struct MSMRescheduleEvent
      {
        MSMRescheduleEvent(SchedulerBase* pSched)
        : m_pScheduler(pSched)
        {}
        SchedulerBase* ptrScheduler() const { return m_pScheduler; }
      private:
        SchedulerBase* m_pScheduler;
      };
      struct MSMStalledEvent{
        MSMStalledEvent(GenericDaemon* pAgent)
          : m_pAgent(pAgent)
        {}
        GenericDaemon* ptrAgent() const { return m_pAgent; }
      private:
        GenericDaemon* m_pAgent;
      };
      struct MSMResumeJobEvent
      {
        MSMResumeJobEvent(GenericDaemon* pAgent)
          : m_pAgent(pAgent)
        {}
        GenericDaemon* ptrAgent() const { return m_pAgent; }
      private:
        GenericDaemon* m_pAgent;
      };

      struct MSMDeleteJobEvent
      {
        MSMDeleteJobEvent(const events::DeleteJobEvent* pEvt, GenericDaemon* pAgent)
        : m_pDelEvt(pEvt), m_pAgent(pAgent)
        {}
        events::SDPAEvent::address_t to() const { return m_pDelEvt->from();}
        events::SDPAEvent::address_t from() const { return m_pDelEvt->to();}
        GenericDaemon* ptrAgent() const { return m_pAgent; }
      private:
        const events::DeleteJobEvent* m_pDelEvt;
        GenericDaemon* m_pAgent;
      };

      struct MSMRetrieveJobResultsEvent
      {
        MSMRetrieveJobResultsEvent(const events::RetrieveJobResultsEvent* pEvt, GenericDaemon* pAgent)
        : m_pRetResEvt(pEvt), m_pAgent(pAgent)
        {}
        events::SDPAEvent::address_t to() const { return m_pRetResEvt->from();}
        events::SDPAEvent::address_t from() const { return m_pRetResEvt->to();}
        GenericDaemon* ptrAgent() const { return m_pAgent; }
      private:
        const events::RetrieveJobResultsEvent* m_pRetResEvt;
        GenericDaemon* m_pAgent;
      };


      // the initial state of the JobFSM SM. Must be defined
      typedef Pending initial_state;

      virtual void action_job_failed(const events::JobFailedEvent&) = 0;
      virtual void action_job_finished(const events::JobFinishedEvent&) = 0;
      virtual void action_reschedule_job(const MSMRescheduleEvent&) = 0;
      virtual void action_job_stalled(const MSMStalledEvent&) = 0;
      virtual void action_resume_job(const MSMResumeJobEvent&) = 0;
      virtual void action_delete_job(const MSMDeleteJobEvent&) = 0;
      virtual void action_retrieve_job_results(const MSMRetrieveJobResultsEvent&) = 0;

      typedef JobFSM_ sm; // makes transition table cleaner

      struct transition_table : boost::mpl::vector
        <
        //      Start           Event                                   Next           Action                Guard
        //      +---------------+---------------------------------------+--------------+---------------------+-----
        _row<   Pending,        MSMResumeJobEvent,                      Running >,
        _row<   Pending,        events::CancelJobEvent, 		Canceled>,
        a_row<  Pending,        events::JobFinishedEvent,         Finished,       &sm::action_job_finished >,
        a_row<  Pending,        events::JobFailedEvent,           Failed,         &sm::action_job_failed >,
        a_row<  Pending,        MSMStalledEvent,                        Stalled,        &sm::action_job_stalled >,
        //      +---------------+-------------------------------------------+-------------------+---------------------+-----
        a_row<  Stalled,        MSMResumeJobEvent,        		Running,        &sm::action_resume_job >,
        a_row<  Stalled,        MSMRescheduleEvent,                 	Pending,        &sm::action_reschedule_job >,
        _irow<  Stalled,        MSMStalledEvent>,
        _row<   Stalled,        sdpa::events::CancelJobEvent,           Canceling>,
        //      +---------------+-------------------------------------------+------------------+---------------------+-----
        a_row<  Running,        events::JobFinishedEvent,         Finished,       &sm::action_job_finished>,
        a_row<  Running,        events::JobFailedEvent,           Failed,         &sm::action_job_failed >,
        _row<   Running,        events::CancelJobEvent,           Canceling>,
        a_row<  Running,        MSMRescheduleEvent,                 	Pending,        &sm::action_reschedule_job >,
        a_row<  Running,        MSMStalledEvent,        		Stalled,        &sm::action_job_stalled >,
        _irow<  Running,        MSMResumeJobEvent>,
        //      +---------------+---------------------------------------+-------------------+---------------------+-----
        a_irow< Finished,       MSMDeleteJobEvent,                                      &sm::action_delete_job >,
        a_irow< Finished,   	MSMRetrieveJobResultsEvent,                             &sm::action_retrieve_job_results>,
        _irow<  Finished,       events::JobFinishedEvent>,
        //      +---------------+---------------------------------------+-------------------+---------------------+-----
        a_irow< Failed,     	MSMDeleteJobEvent,                                      &sm::action_delete_job>,
        a_irow< Failed,     	MSMRetrieveJobResultsEvent,                             &sm::action_retrieve_job_results>,
        _irow<  Failed,         events::JobFailedEvent>,
        //      +---------------+---------------------------------------+-------------------+---------------------+-----
        _row<   Canceling, 	sdpa::events::CancelJobAckEvent,     	Canceled>,
        a_row<  Canceling, 	sdpa::events::JobFinishedEvent,      	Canceled,       &sm::action_job_finished>,
        a_row<  Canceling, 	sdpa::events::JobFailedEvent,           Canceled,       &sm::action_job_failed>,
        _irow<  Canceling,      sdpa::events::CancelJobEvent>,
        _irow<  Canceling,      MSMResumeJobEvent>,
        _irow<  Canceling,      MSMStalledEvent>,
        //      +---------------+-------------------------------------------+-------------------+---------------------+-----
        a_irow< Canceled,       MSMDeleteJobEvent,                                      &sm::action_delete_job>,
        a_irow< Canceled,       MSMRetrieveJobResultsEvent,                             &sm::action_retrieve_job_results>,
        _irow<  Canceled,       events::CancelJobAckEvent>
        >{};

      //! \note This table refers to the order in which states are
      //! first seen in the state machine definition. This is hacky
      //! and should be removed / done via visitors.
      static status::code state_code (size_t state)
      {
        static status::code const state_codes[] =
          { status::PENDING
          , status::STALLED
          , status::RUNNING
          , status::FINISHED
          , status::FAILED
          , status::CANCELING
          , status::CANCELED
          };
        fhg_assert ( state < sizeof (state_codes) / sizeof (*state_codes)
                   , "index shall be valid"
                   );
        return state_codes[state];
      }

      template <class FSM, class Event>
      void no_transition(Event const& e, FSM&, int state)
      {
        DMLOG(ERROR,  "No transition from state "
                                         + sdpa::status::show(state_code(state))
                                         + " on event "
                                         + typeid(e).name()
                                         );

        throw std::runtime_error ( "no transition from state "
                                 + status::show(state_code(state))
                                 + " on event "
                                 + typeid(e).name()
                                 );
      }
    };

    class Job : public boost::msm::back::state_machine<JobFSM_>
    {
    public:
      typedef boost::unordered_map<job_id_t, Job*> job_list_t;
      typedef boost::recursive_mutex mutex_type;
      typedef boost::unique_lock<mutex_type> lock_type;

      Job ( const job_id_t id
          , const job_desc_t desc
          , const job_id_t &parent
          , bool is_master_job
          );

      const job_id_t& id() const;
      const job_id_t& parent() const;
      const job_desc_t& description() const;
      const job_result_t& result() const;

      int error_code() const;
      std::string error_message () const;

      bool isMasterJob() const;

      void set_owner(const worker_id_t& owner);
      worker_id_t owner() const;

      status::code getStatus() const;

      bool completed() const;
      bool is_running() const;

      // job FSM actions
      virtual void action_job_failed(const events::JobFailedEvent&);
      virtual void action_job_finished(const events::JobFinishedEvent&);
      virtual void action_reschedule_job(const MSMRescheduleEvent&);
      virtual void action_job_stalled(const MSMStalledEvent&);
      virtual void action_resume_job(const MSMResumeJobEvent&);
      virtual void action_delete_job(const MSMDeleteJobEvent&);
      virtual void action_retrieve_job_results(const MSMRetrieveJobResultsEvent&);

      //transitions
      void CancelJob(const events::CancelJobEvent*);
      void CancelJobAck(const events::CancelJobAckEvent*);
      void JobFailed(const events::JobFailedEvent*);
      void JobFinished(const events::JobFinishedEvent*);

      void DeleteJob(const events::DeleteJobEvent*, GenericDaemon*);
      void RetrieveJobResults(const events::RetrieveJobResultsEvent* pEvt, GenericDaemon*);
      void Reschedule(SchedulerBase*);

      void Dispatch();
      void Pause(GenericDaemon*);
      void Resume(GenericDaemon*);

    private:
      mutable mutex_type mtx_;
      job_id_t id_;
      job_desc_t desc_;
      job_id_t parent_;

      bool _is_master_job;
      job_result_t result_;
      int m_error_code;
      std::string m_error_message;

      worker_id_t m_owner;
    };
}}

#endif
