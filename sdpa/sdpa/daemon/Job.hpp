#ifndef SDPA_JOB_HPP
#define SDPA_JOB_HPP 1

#include <sdpa/events/CancelJobAckEvent.hpp>
#include <sdpa/events/CancelJobEvent.hpp>
#include <sdpa/events/ErrorEvent.hpp>
#include <sdpa/events/JobFailedEvent.hpp>
#include <sdpa/events/JobFinishedEvent.hpp>
#include <sdpa/events/JobResultsReplyEvent.hpp>
#include <sdpa/events/JobStatusReplyEvent.hpp>
#include <sdpa/events/QueryJobStatusEvent.hpp>
#include <sdpa/events/SubmitJobAckEvent.hpp>
#include <sdpa/job_states.hpp>
#include <sdpa/types.hpp>

#include <fhg/assert.hpp>

#include <fhglog/fhglog.hpp>

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
      JobFSM_() : _logger (fhg::log::Logger::get ("JobFSM")) {}
      fhg::log::Logger::ptr_t _logger;
      virtual ~JobFSM_() {}

      // The list of FSM states
      struct Pending : public boost::msm::front::state<>{};
      struct Running : public boost::msm::front::state<>{};
      struct Finished : public boost::msm::front::state<>{};
      struct Failed : public boost::msm::front::state<>{};
      struct Canceling : public boost::msm::front::state<>{};
      struct Canceled : public boost::msm::front::state<>{};

      struct MSMRescheduleEvent {};

      struct MSMDispatchJobEvent
      {
        MSMDispatchJobEvent(GenericDaemon* pAgent)
          : m_pAgent(pAgent)
        {}
        GenericDaemon* ptrAgent() const { return m_pAgent; }
      private:
        GenericDaemon* m_pAgent;
      };

      // the initial state of the JobFSM SM. Must be defined
      typedef Pending initial_state;

      typedef JobFSM_ sm; // makes transition table cleaner

      struct transition_table : boost::mpl::vector
        <
        //      Start           Event                                   Next           Action                Guard
        //      +---------------+---------------------------------------+--------------+---------------------+-----
        _row<   Pending,        MSMDispatchJobEvent,                      Running >,
        _row<   Pending,        events::CancelJobEvent, 		Canceling>,
        _row<   Pending,        events::JobFinishedEvent,               Finished>,
        _row<   Pending,        events::JobFailedEvent,                 Failed>,
        _irow<  Pending,        MSMRescheduleEvent>,
        //      +---------------+-------------------------------------------+------------------+---------------------+-----
        _row<   Running,        events::JobFinishedEvent,               Finished>,
        _row<   Running,        events::JobFailedEvent,                 Failed>,
        _row<   Running,        events::CancelJobEvent,                 Canceling>,
        _row<   Running,        MSMRescheduleEvent,                 	Pending>,
        _irow<  Running,        MSMDispatchJobEvent>,
        //      +---------------+---------------------------------------+-------------------+---------------------+-----
        _irow<  Finished,       events::JobFinishedEvent>,
        //      +---------------+---------------------------------------+-------------------+---------------------+-----
        _irow<  Failed,         events::JobFailedEvent>,
        //      +---------------+---------------------------------------+-------------------+---------------------+-----
        _row<   Canceling, 	sdpa::events::CancelJobAckEvent,     	Canceled>,
        _row<   Canceling, 	sdpa::events::JobFinishedEvent,      	Canceled>,
        _row<   Canceling, 	sdpa::events::JobFailedEvent,           Canceled>,
        _irow<  Canceling,      sdpa::events::CancelJobEvent>,
        _irow<  Canceling,      MSMDispatchJobEvent>,
        //      +---------------+-------------------------------------------+-------------------+---------------------+-----
        _irow<  Canceled,       events::CancelJobAckEvent>,
        _irow<  Canceled,       events::JobFinishedEvent>,
        _irow<  Canceled,       events::JobFailedEvent>
        >{};

      //! \note This table refers to the order in which states are
      //! first seen in the state machine definition. This is hacky
      //! and should be removed / done via visitors.
      static status::code state_code (size_t state)
      {
        static status::code const state_codes[] =
          { status::PENDING
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
          , bool is_master_job
          , const worker_id_t& owner
          );

      const job_id_t& id() const;
      const job_desc_t& description() const;
      const job_result_t& result() const;

      std::string error_message () const;

      bool isMasterJob() const;

      worker_id_t owner() const;

      status::code getStatus() const;

      //transitions
      void CancelJob(const events::CancelJobEvent*);
      void CancelJobAck(const events::CancelJobAckEvent*);
      void JobFailed(const events::JobFailedEvent*);
      void JobFinished(const events::JobFinishedEvent*);

      void Reschedule();

      void Dispatch();

    private:
      mutable mutex_type mtx_;
      job_id_t id_;
      job_desc_t desc_;

      bool _is_master_job;
      job_result_t result_;
      std::string m_error_message;

      worker_id_t m_owner;
    };
}}

#endif
