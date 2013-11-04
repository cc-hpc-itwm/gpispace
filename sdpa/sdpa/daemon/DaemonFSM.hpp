#ifndef AGENT_FSM_BMSM_HPP
#define AGENT_FSM_BMSM_HPP 1

/*
 * =====================================================================================
 *
 *       Filename:  DaemonFSM.hpp
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

#include <iostream>
// back-end
#include <boost/msm/back/state_machine.hpp>
//front-end
#include <boost/msm/front/state_machine_def.hpp>

#include <sdpa/daemon/GenericDaemon.hpp>
#include <boost/serialization/access.hpp>
#include <boost/thread.hpp>

namespace msm = boost::msm;
namespace mpl = boost::mpl;

namespace sdpa {
  namespace fsm {
    namespace bmsm {
      // front-end: define the FSM structure
      struct DaemonFSM_ : public msm::front::state_machine_def<DaemonFSM_>
      {
        virtual ~DaemonFSM_ () {}

        // The list of FSM states
        struct Down : public msm::front::state<>{};
        struct Up : public msm::front::state<>{};

        // events
        struct InterruptEvent {};
        struct ConfigOkEvent {};
        struct ConfigNokEvent {};

        // the initial state of the DaemonFSM SM. Must be defined
        typedef Down initial_state;

        virtual void action_delete_job(const sdpa::events::DeleteJobEvent& );
        virtual void action_request_job(const sdpa::events::RequestJobEvent& );
        virtual void action_submit_job(const sdpa::events::SubmitJobEvent& );
        virtual void action_register_worker(const sdpa::events::WorkerRegistrationEvent& );
        virtual void action_error_event(const sdpa::events::ErrorEvent& );

        typedef DaemonFSM_ agentFSM; // makes transition table cleaner

        struct transition_table : mpl::vector<
        //      Start         Event         		                      Next            Action                Guard
        //      +-------------+---------------------------------------+---------------+---------------------+-----
        _row<   Down,         ConfigOkEvent,                          Up>,
        _irow<  Down,         ConfigNokEvent>,
        _irow<  Down,         sdpa::events::ErrorEvent >,
        //      +------------+-----------------------+----------------+--------------+-----
        _row<   Up,           InterruptEvent,                         Down>,
        a_irow< Up,           sdpa::events::WorkerRegistrationEvent,                  &agentFSM::action_register_worker>,
        a_irow< Up,           sdpa::events::DeleteJobEvent,                           &agentFSM::action_delete_job>,
        a_irow< Up,           sdpa::events::SubmitJobEvent,                           &agentFSM::action_submit_job>,
        a_irow< Up,           sdpa::events::RequestJobEvent,                          &agentFSM::action_request_job>,
        a_irow< Up,           sdpa::events::ErrorEvent,                               &agentFSM::action_error_event>
        >{};

        template <class FSM, class Event>
        void no_transition(Event const& e, FSM&, int state)
        {
          LOG(DEBUG, "no transition from state "<< " on event " << typeid(e).name());
        }
      };

      // Pick a back-end
      class DaemonFSM : public msm::back::state_machine<DaemonFSM_>, public sdpa::daemon::GenericDaemon
      {
      public:
        typedef sdpa::shared_ptr<DaemonFSM> Ptr;
        typedef boost::recursive_mutex mutex_type;
        typedef boost::unique_lock<mutex_type> lock_type;

        DaemonFSM(const std::string &name = "",
                  const sdpa::master_info_list_t& arrMasterNames = sdpa::master_info_list_t(),
                  unsigned int cap = 10000,
                  unsigned int rank = 0
                 , const std::string& guiUrl = ""
                 );

        void perform_ConfigOkEvent();
        void perform_ConfigNokEvent();

        void action_delete_job(const sdpa::events::DeleteJobEvent& );
        void action_request_job(const sdpa::events::RequestJobEvent& );
        void action_submit_job(const sdpa::events::SubmitJobEvent& );
        void action_register_worker(const sdpa::events::WorkerRegistrationEvent& );
        void action_error_event(const sdpa::events::ErrorEvent& );

        // event handlers
        void handleInterruptEvent();
        void handleWorkerRegistrationEvent(const sdpa::events::WorkerRegistrationEvent* );
        void handleDeleteJobEvent(const sdpa::events::DeleteJobEvent* );
        void handleSubmitJobEvent(const sdpa::events::SubmitJobEvent* );
        void handleRequestJobEvent(const sdpa::events::RequestJobEvent* );
        void handleErrorEvent(const sdpa::events::ErrorEvent* );

        template <class Archive>
        void serialize(Archive& ar, const unsigned int)
        {
          ar & boost::serialization::base_object<GenericDaemon>(*this);
          ar & boost::serialization::base_object<msm::back::state_machine<DaemonFSM_> >(*this);
        }

        friend class boost::serialization::access;

      private:
        mutex_type mtx_;
      };
    }
  }
}


#endif
