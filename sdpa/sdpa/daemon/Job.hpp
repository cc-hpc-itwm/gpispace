#pragma once

#include <sdpa/job_states.hpp>
#include <sdpa/types.hpp>
#include <sdpa/job_requirements.hpp>

#include <fhg/assert.hpp>

#include <boost/msm/back/state_machine.hpp>
#include <boost/msm/front/state_machine_def.hpp>
#include <boost/thread.hpp>

namespace sdpa
{
  namespace daemon
  {
    // front-end: define the FSM structure
    struct JobFSM_ : public boost::msm::front::state_machine_def<JobFSM_>
    {
      virtual ~JobFSM_() = default;

      struct s_pending : public boost::msm::front::state<>{};
      struct s_running : public boost::msm::front::state<>{};
      struct s_finished : public boost::msm::front::state<>{};
      struct s_failed : public boost::msm::front::state<>{};
      struct s_canceling : public boost::msm::front::state<>{};
      struct s_canceled : public boost::msm::front::state<>{};

      struct e_begin_cancel {};
      struct e_canceled {};
      struct e_dispatch {};
      struct e_failed {};
      struct e_finished {};
      struct e_reschedule {};

      typedef s_pending initial_state;

      struct transition_table : boost::mpl::vector
        <
        //     Start         Event              Next
        //   +-------------+------------------+-------------+
         _row< s_pending   , e_begin_cancel   , s_canceling >,
         _row< s_pending   , e_dispatch       , s_running   >,
         _row< s_pending   , e_failed         , s_failed    >,
         _row< s_pending   , e_finished       , s_finished  >,
        _irow< s_pending   , e_reschedule   /*, ignore */   >,
        //   +-------------+------------------+-------------+
         _row< s_running   , e_begin_cancel   , s_canceling >,
        _irow< s_running   , e_dispatch     /*, ignore */   >,
         _row< s_running   , e_failed         , s_failed    >,
         _row< s_running   , e_finished       , s_finished  >,
         _row< s_running   , e_reschedule     , s_pending   >,
        //   +-------------+------------------+-------------+
        _irow< s_finished  , e_finished     /*, ignore */   >,
        //   +-------------+------------------+-------------+
        _irow< s_failed    , e_failed       /*, ignore */   >,
        //   +-------------+------------------+-------------+
        _irow< s_canceling , e_begin_cancel /*, ignore */   >,
         _row< s_canceling , e_canceled       , s_canceled  >,
        _irow< s_canceling , e_dispatch     /*, ignore */   >,
         _row< s_canceling , e_failed         , s_canceled  >,
         _row< s_canceling , e_finished       , s_canceled  >,
        //   +-------------+------------------+-------------+
        _irow< s_canceled  , e_canceled     /*, ignore */   >,
        _irow< s_canceled  , e_failed       /*, ignore */   >,
        _irow< s_canceled  , e_finished     /*, ignore */   >
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
      Job ( const job_id_t id
          , we::type::activity_t
          , opaque_job_master_t owner
          , job_requirements_t
          );

      we::type::activity_t const& activity() const
      {
        return _activity;
      }
      const job_id_t& id() const;
      opaque_job_master_t const& owner() const;
      job_requirements_t requirements() const;

      std::string error_message () const;
      const we::type::activity_t& result() const;

      status::code getStatus() const;

      void CancelJob();
      void CancelJobAck();
      void Dispatch();
      void JobFailed (std::string error_message);
      void JobFinished (we::type::activity_t);
      void Reschedule();

    private:
      mutable boost::mutex mtx_;
      we::type::activity_t _activity;
      job_id_t id_;
      opaque_job_master_t m_owner;
      job_requirements_t _requirements;

      std::string m_error_message;
      we::type::activity_t result_;
    };
}}
