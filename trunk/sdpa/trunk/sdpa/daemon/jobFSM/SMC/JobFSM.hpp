/*
 * =====================================================================================
 *
 *       Filename:  JobFSM.hpp
 *
 *    Description:  Job state machine (state machine compiler)
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
#ifndef JOB_FSM_SMC_HPP
#define JOB_FSM_SMC_HPP 1

#include <sdpa/daemon/IComm.hpp>
#include <sdpa/daemon/JobImpl.hpp>
#include <sdpa/daemon/jobFSM/SMC/JobFSM_sm.h>
#include <sdpa/logging.hpp>
#include <sdpa/types.hpp>
#include <boost/serialization/access.hpp>
#include <boost/thread.hpp>

namespace sdpa { namespace fsm { namespace smc {
	class JobFSM : public sdpa::daemon::JobImpl {
		public:
			typedef sdpa::shared_ptr<JobFSM> Ptr;
			typedef boost::recursive_mutex mutex_type;
			typedef boost::unique_lock<mutex_type> lock_type;

			JobFSM( const sdpa::job_id_t id = JobId(""),
					const sdpa::job_desc_t desc = "",
				    const sdpa::daemon::IComm* pHandler = NULL,
				    const sdpa::job_id_t &parent = sdpa::job_id_t::invalid_job_id())
			: JobImpl(id, desc, pHandler, parent), m_fsmContext(*this)
			{
				DLOG(TRACE, "Job state machine created: " << id);
			}

            virtual ~JobFSM() { DLOG(TRACE, "Job state machine destroyed: " << id()); }

			//transitions
			void CancelJob(const sdpa::events::CancelJobEvent*);
			void CancelJobAck(const sdpa::events::CancelJobAckEvent*);
			void DeleteJob(const sdpa::events::DeleteJobEvent* pEvt, sdpa::daemon::IComm*  ptr_comm);
			void JobFailed(const sdpa::events::JobFailedEvent*);
			void JobFinished(const sdpa::events::JobFinishedEvent*);
			void QueryJobStatus(const sdpa::events::QueryJobStatusEvent*, sdpa::daemon::IComm* );
			//void RetrieveJobResults(const sdpa::events::RetrieveJobResultsEvent*);
			void RetrieveJobResults(const sdpa::events::RetrieveJobResultsEvent* pEvt, sdpa::daemon::IComm* ptr_comm)
			void Dispatch();

			sdpa::status_t getStatus();

			JobFSMContext& GetContext() { return m_fsmContext; }

			template<class Archive>
			void save(Archive & ar, const unsigned int) const
			{
				//lock_type lock(mtx_);
				int stateId(m_fsmContext.getState().getId());

			    // invoke serialization of the base class
			    ar << boost::serialization::base_object<JobImpl>(*this);
			    ar << stateId;
			}

			template<class Archive>
			void load(Archive & ar, const unsigned int)
			{
				lock_type lock(mtx_);
				int stateId;

			    // invoke serialization of the base class
			    ar >> boost::serialization::base_object<JobImpl>(*this);
			    ar >> stateId;

			    LOG(TRACE, "setState("<<id()<<") to "<<m_fsmContext.valueOf(stateId).getName()<<"!!!");
			    JobFSMState& state = m_fsmContext.valueOf(stateId);
			    m_fsmContext.setState(state);
			    state.Entry(m_fsmContext);

			   try {
				   LOG(TRACE, "Current state is: "<<m_fsmContext.getState().getName());
			   }
			   catch( const statemap::StateUndefinedException& ex )
			   {
				   LOG(TRACE, "Current state is: UNDEFINED");
			   }

			}

			template<class Archive>
			void serialize( Archive & ar, const unsigned int file_version )
			{
			    boost::serialization::split_member(ar, *this, file_version);
			}

			friend class boost::serialization::access;
			//friend class sdpa::tests::WorkerSerializationTest;

		private:
			JobFSMContext m_fsmContext;
			//sdpa::status_t m_status_;
			mutex_type mtx_;
	};
}}}

#endif
