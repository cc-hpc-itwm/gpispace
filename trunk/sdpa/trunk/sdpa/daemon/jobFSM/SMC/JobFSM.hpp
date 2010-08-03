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

namespace sdpa { namespace fsm { namespace smc {
	class JobFSM : public sdpa::daemon::JobImpl {
		public:
			typedef sdpa::shared_ptr<JobFSM> Ptr;

			JobFSM( const sdpa::job_id_t id = JobId(""),
					const sdpa::job_desc_t desc = "",
				    const sdpa::daemon::IComm* pHandler = NULL,
				    const sdpa::job_id_t &parent = sdpa::job_id_t::invalid_job_id())
			: JobImpl(id, desc, pHandler, parent), m_fsmContext(*this)
			{
                          SDPA_LOG_DEBUG("Job state machine created: " << id);
			}

          virtual ~JobFSM() { SDPA_LOG_DEBUG("Job state machine destroyed: " << id()); }

			//transitions
			void CancelJob(const sdpa::events::CancelJobEvent*);
			void CancelJobAck(const sdpa::events::CancelJobAckEvent*);
			void DeleteJob(const sdpa::events::DeleteJobEvent*);
			void JobFailed(const sdpa::events::JobFailedEvent*);
			void JobFinished(const sdpa::events::JobFinishedEvent*);
			void QueryJobStatus(const sdpa::events::QueryJobStatusEvent*);
			void RetrieveJobResults(const sdpa::events::RetrieveJobResultsEvent*);
			void Dispatch();

			sdpa::status_t getStatus() { return m_status_; }
			JobFSMContext& GetContext() { return m_fsmContext; }

			template<class Archive>
			void save(Archive & ar, const unsigned int) const
			{
				int stateId(m_fsmContext.getState().getId());

			    // invoke serialization of the base class
			    ar << boost::serialization::base_object<JobImpl>(*this);
			    ar << stateId;
			}

			template<class Archive>
			void load(Archive & ar, const unsigned int)
			{
				int stateId;

			    // invoke serialization of the base class
			    ar >> boost::serialization::base_object<JobImpl>(*this);
			    ar >> stateId;

			    m_fsmContext.setState(m_fsmContext.valueOf(stateId));
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
			sdpa::status_t m_status_;
	};
}}}

#endif
