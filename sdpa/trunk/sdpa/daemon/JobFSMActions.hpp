/*
 * =====================================================================================
 *
 *       Filename:  JobFSMActions.hpp
 *
 *    Description:  Defines interface for the job's state machine actions
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
#ifndef JOB_FSM_ACTIONS_HPP
#define JOB_FSM_ACTIONS_HPP 1

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

#include <boost/serialization/access.hpp>

namespace sdpa {
	namespace fsm {
		class JobFSMActions {
			public:
                  virtual ~JobFSMActions() {}

				//actions
				virtual void action_run_job()=0;
				virtual void action_cancel_job_from_pending(const sdpa::events::CancelJobEvent&)=0;
				virtual void action_cancel_job(const sdpa::events::CancelJobEvent&)=0;
				virtual void action_cancel_job_ack(const sdpa::events::CancelJobAckEvent&)=0;
				virtual void action_delete_job(const sdpa::events::DeleteJobEvent&)=0;
				//virtual void action_query_job_status(const sdpa::events::QueryJobStatusEvent&)=0;
				virtual void action_job_failed(const sdpa::events::JobFailedEvent&)=0;
				virtual void action_job_finished(const sdpa::events::JobFinishedEvent&)=0;
				virtual void action_retrieve_job_results(const sdpa::events::RetrieveJobResultsEvent&)=0;

				friend class boost::serialization::access;
				template<class Archive>
					void serialize(Archive& , const unsigned int /* file version */){}

		};
	}
}

#endif
