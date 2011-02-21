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
#define BOOST_MPL_CFG_NO_PREPROCESSED_HEADERS
#define BOOST_MPL_LIMIT_VECTOR_SIZE 30 //or whatever you need
#define BOOST_MPL_LIMIT_MAP_SIZE 30 //or whatever you need

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

char const* const agent_state_names[] = { "Down", "Configuring", "Up" };

namespace sdpa {
	namespace fsm {
		namespace bmsm {

			// front-end: define the FSM structure
			struct DaemonFSM_ : public msm::front::state_machine_def<DaemonFSM_>
			{
				// The list of FSM states
				struct Down : public msm::front::state<>{};
				struct Configuring : public msm::front::state<>{};
				struct Up : public msm::front::state<>{};

				// the initial state of the DaemonFSM SM. Must be defined
				typedef Down initial_state;

				virtual void action_configure(const sdpa::events::StartUpEvent&);
				virtual void action_config_ok(const sdpa::events::ConfigOkEvent&);
				virtual void action_config_nok(const sdpa::events::ConfigNokEvent&);
				virtual void action_interrupt(const sdpa::events::InterruptEvent& );
				virtual void action_delete_job(const sdpa::events::DeleteJobEvent& );
				virtual void action_request_job(const sdpa::events::RequestJobEvent& );
				virtual void action_submit_job(const sdpa::events::SubmitJobEvent& );
				virtual void action_config_request(const sdpa::events::ConfigRequestEvent& );
				virtual void action_register_worker(const sdpa::events::WorkerRegistrationEvent& );
				virtual void action_error_event(const sdpa::events::ErrorEvent& );

				typedef DaemonFSM_ agentFSM; // makes transition table cleaner

				struct transition_table : mpl::vector<
				//      Start        Event         		                    Next           Action                Guard
				//      +------------+--------------------------------------+--------------+---------------------+-----
				a_row<  Down,    	 sdpa::events::StartUpEvent, 			Configuring,&agentFSM::action_configure>,
				_irow<  Down,    	 sdpa::events::ErrorEvent >,
				//      +------------+-----------------------+--------------+---------------------+-----
				a_row<  Configuring, sdpa::events::ConfigOkEvent,			Up, 		&agentFSM::action_config_ok>,
				a_row<  Configuring, sdpa::events::ConfigNokEvent, 	 	 	Down, 		&agentFSM::action_config_nok >,
				_irow<  Configuring, sdpa::events::ErrorEvent >,
				//      +------------+-----------------------+--------------+---------------------+-----
				a_row<  Up,   		 sdpa::events::InterruptEvent, 		 	Down,		&agentFSM::action_interrupt >,
				a_irow< Up,    		 sdpa::events::WorkerRegistrationEvent,			 	&agentFSM::action_register_worker>,
				a_irow< Up,    		 sdpa::events::DeleteJobEvent,					 	&agentFSM::action_delete_job>,
				a_irow< Up,    		 sdpa::events::SubmitJobEvent,					 	&agentFSM::action_submit_job>,
				a_irow< Up,    		 sdpa::events::RequestJobEvent, 				 	&agentFSM::action_request_job>,
				a_irow< Up,    		 sdpa::events::ConfigRequestEvent,               	&agentFSM::action_config_request>,
				a_irow< Up,    		 sdpa::events::ErrorEvent,						 	&agentFSM::action_error_event>
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

				// obsolete
				DaemonFSM(	const std::string &name,
							seda::Stage* ptrToMasterStage,
							seda::Stage* ptrToSlaveStage,
							IWorkflowEngine*  pArgSdpa2Gwes
							);
				// obsolete
				DaemonFSM(  const std::string &name,
							IWorkflowEngine*  pArgSdpa2Gwes,
							const std::string& toMasterStageName,
							const std::string& toSlaveStageName = std::string("") );

				DaemonFSM( const std::string &name = "", IWorkflowEngine* pArgSdpa2Gwes = NULL );

				virtual ~DaemonFSM();

				void start_fsm() { start(); }

				virtual void action_configure(const sdpa::events::StartUpEvent& );
				virtual void action_config_ok(const sdpa::events::ConfigOkEvent& );
				virtual void action_config_nok(const sdpa::events::ConfigNokEvent& );
				virtual void action_interrupt(const sdpa::events::InterruptEvent& );
				virtual void action_delete_job(const sdpa::events::DeleteJobEvent& );
				virtual void action_request_job(const sdpa::events::RequestJobEvent& );
				virtual void action_submit_job(const sdpa::events::SubmitJobEvent& );
				virtual void action_config_request(const sdpa::events::ConfigRequestEvent& );
				virtual void action_register_worker(const sdpa::events::WorkerRegistrationEvent& );
				virtual void action_error_event(const sdpa::events::ErrorEvent& );

				// event handlers
				void handleStartUpEvent(const sdpa::events::StartUpEvent* );
				void handleConfigOkEvent(const sdpa::events::ConfigOkEvent* );
				void handleConfigNokEvent(const sdpa::events::ConfigNokEvent* );
				void handleInterruptEvent(const sdpa::events::InterruptEvent* );
				void handleWorkerRegistrationEvent(const sdpa::events::WorkerRegistrationEvent* );
				void handleDeleteJobEvent(const sdpa::events::DeleteJobEvent* );
				void handleSubmitJobEvent(const sdpa::events::SubmitJobEvent* );
				void handleRequestJobEvent(const sdpa::events::RequestJobEvent* );
				void handleConfigRequestEvent(const sdpa::events::ConfigRequestEvent* );
				void handleErrorEvent(const sdpa::events::ErrorEvent* );

				sdpa::status_t getStatus();

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
