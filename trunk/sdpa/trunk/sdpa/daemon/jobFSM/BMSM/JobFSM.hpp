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

#define BOOST_MPL_CFG_NO_PREPROCESSED_HEADERS
#define BOOST_MPL_LIMIT_VECTOR_SIZE 30 //or whatever you need
#define BOOST_MPL_LIMIT_MAP_SIZE 30 //or whatever you need

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
				// The list of FSM states
				struct Pending : 	public msm::front::state<>{};
				struct Running : 	public msm::front::state<>{};
				struct Finished : 	public msm::front::state<>{};
				struct Failed : 	public msm::front::state<>{};
				struct Cancelling : public msm::front::state<>{};
				struct Cancelled : 	public msm::front::state<>{};

				// the initial state of the JobFSM SM. Must be defined
				typedef Pending initial_state;

				virtual void action_run_job(const sdpa::events::RunJobEvent&) { std::cout << "JobFSM_::action_run_job\n"; }
				virtual void action_cancel_job(const sdpa::events::CancelJobEvent&) { std::cout << "JobFSM_::action_cancel_job\n"; }
				virtual void action_cancel_job_from_pending(const sdpa::events::CancelJobEvent&){ std::cout << "JobFSM_::action_cancel_job_from_pending\n"; }
				virtual void action_cancel_job_ack(const sdpa::events::CancelJobAckEvent&){ std::cout << "JobFSM_::action_cancel_job_ack\n"; }
				virtual void action_delete_job(const sdpa::events::DeleteJobEvent&){ std::cout << "JobFSM_::action_delete_job\n"; }
				virtual void action_job_failed(const sdpa::events::JobFailedEvent&){ std::cout << "JobFSM_::action_job_failed\n"; }
				virtual void action_job_finished(const sdpa::events::JobFinishedEvent&){ std::cout << "JobFSM_::action_job_finished\n"; }
				virtual void action_retrieve_job_results(const sdpa::events::RetrieveJobResultsEvent&){ std::cout << "JobFSM_::action_retrieve_job_results\n"; }

				typedef JobFSM_ sm; // makes transition table cleaner

				struct transition_table : mpl::vector<
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
					LOG(DEBUG, "no transition from state "<< " on event " << typeid(e).name());
				}

				template <class FSM>
				void no_transition(sdpa::events::QueryJobStatusEvent const& e, FSM&, int state)
				{
					LOG(DEBUG, "process event QueryJobStatusEvent");
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
						const sdpa::daemon::IComm* pHandler = NULL,
						const sdpa::job_id_t &parent = sdpa::job_id_t::invalid_job_id())
						: job_impl_(id, desc, pHandler, parent)
						  , SDPA_INIT_LOGGER("sdpa.fsm.bmsm.JobFSM")
				{
					SDPA_LOG_DEBUG("State machine created");
				}

				~JobFSM()
				{
					SDPA_LOG_DEBUG("State machine destroyed");
				}

				const job_id_t& id() const { return job_impl_.id(); }
				const job_id_t& parent() const { return job_impl_.parent(); }
				const job_desc_t& description() const { return job_impl_.description(); }
				const job_result_t& result() const { return job_impl_.result(); }
				void set_icomm(sdpa::daemon::IComm* pArgComm) { job_impl_.set_icomm(pArgComm); }

				bool is_marked_for_deletion() { return job_impl_.is_marked_for_deletion(); }
				bool mark_for_deletion() { return job_impl_.mark_for_deletion(); }

				bool is_local(){ return job_impl_.is_local(); }
				void set_local(bool b){ job_impl_.set_local(b); }

				std::string print_info() { return job_impl_.print_info(); }

				unsigned long& walltime() { return job_impl_.walltime(); }
				void setResult(const sdpa::job_result_t& res) { return job_impl_.setResult(res); }

				//transitions
				void CancelJob(const sdpa::events::CancelJobEvent* pEvt) {lock_type lock(mtx_); process_event(*pEvt);}
				void CancelJobAck(const sdpa::events::CancelJobAckEvent* pEvt) {lock_type lock(mtx_); process_event(*pEvt);}
				void DeleteJob(const sdpa::events::DeleteJobEvent* pEvt) {lock_type lock(mtx_); process_event(*pEvt);}
				void JobFailed(const sdpa::events::JobFailedEvent* pEvt) {lock_type lock(mtx_); process_event(*pEvt);}
				void JobFinished(const sdpa::events::JobFinishedEvent* pEvt) {lock_type lock(mtx_); process_event(*pEvt);}
				void QueryJobStatus(const sdpa::events::QueryJobStatusEvent* pEvt)
				{
					// attention, no action called!
					lock_type lock(mtx_);
					process_event(*pEvt);

					//LOG(TRACE, "The status of the job "<<id()<<" is " << getStatus()<<"!!!");
					sdpa::status_t status = getStatus();
					if(job_impl_.icomm())
					{
						sdpa::events::JobStatusReplyEvent::Ptr
						        pStatReply(new sdpa::events::JobStatusReplyEvent( pEvt->to(), pEvt->from(), id(), status));

						job_impl_.icomm()->sendEventToMaster(pStatReply);
					}
					else
						SDPA_LOG_WARN("Could not send back job status reply. Invalid communication handler!");

				}
				void RetrieveJobResults(const sdpa::events::RetrieveJobResultsEvent* pEvt) {lock_type lock(mtx_); process_event(*pEvt);}
				void Dispatch() { MSMDispatchEvent DispEvt;lock_type lock(mtx_); process_event(DispEvt);}

				// actions
				void action_run_job() { job_impl_.action_run_job(); }
				void action_cancel_job(const sdpa::events::CancelJobEvent& e) { job_impl_.action_cancel_job(e); }
				void action_cancel_job_from_pending(const sdpa::events::CancelJobEvent& e){ job_impl_.action_cancel_job_from_pending(e); }
				void action_cancel_job_ack(const sdpa::events::CancelJobAckEvent& e){ job_impl_.action_cancel_job_ack(e); }
				void action_delete_job(const sdpa::events::DeleteJobEvent& e){ job_impl_.action_delete_job(e); }

				void action_job_failed(const sdpa::events::JobFailedEvent& e){ job_impl_.action_job_failed(e); }
				void action_job_finished(const sdpa::events::JobFinishedEvent& e){ job_impl_.action_job_finished(e); }
				void action_retrieve_job_results(const sdpa::events::RetrieveJobResultsEvent& e){ job_impl_.action_retrieve_job_results(e); }

				sdpa::status_t getStatus()
				{
					//SDPA_LOG_INFO("Look for the status of the job "<<id());
					sdpa::status_t status(state_names[*current_state()]);
					//SDPA_LOG_INFO("The status of the job "<<id()<<" is "<<status);
					return status;
				}

				template <class Archive>
				void serialize(Archive& ar, const unsigned int)
				{
					ar.register_type(static_cast<sdpa::daemon::JobImpl*>(NULL));
					//ar.register_type(static_cast<JobFSM*>(NULL));

					ar & boost::serialization::base_object<Job>(*this);
					ar & boost::serialization::base_object<msm::back::state_machine<JobFSM_> >(*this);
					ar & job_impl_;
				}

				friend class boost::serialization::access;

			private:
				mutex_type mtx_;
				sdpa::daemon::JobImpl job_impl_;
				SDPA_DECLARE_LOGGER();
			};
		}
	}
}

#endif
