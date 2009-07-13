#ifndef JOB_FSM_ACTIONS_HPP
#define JOB_FSM_ACTIONS_HPP 1

#include <sdpa/events/RunJobEvent.hpp>
#include <sdpa/events/JobFailedEvent.hpp>
#include <sdpa/events/QueryJobStatusEvent.hpp>
#include <sdpa/events/JobStatusReplyEvent.hpp>
#include <sdpa/events/CancelJobEvent.hpp>
#include <sdpa/events/CancelJobAckEvent.hpp>
#include <sdpa/events/DeleteJobEvent.hpp>
#include <sdpa/events/JobFinishedEvent.hpp>
#include <sdpa/events/ErrorEvent.hpp>
#include <sdpa/events/RetrieveJobResultsEvent.hpp>

namespace sdpa {
	namespace fsm {
		class JobFSMActions {
			public:
				//actions
				virtual void action_run_job(const sdpa::events::RunJobEvent& e)=0;
				virtual void action_cancel_job(const sdpa::events::CancelJobEvent& e)=0;
				virtual void action_cancel_job_ack(const sdpa::events::CancelJobAckEvent& e)=0;
				virtual void action_delete_job(const sdpa::events::DeleteJobEvent& e)=0;
				virtual void action_query_job_status(const sdpa::events::QueryJobStatusEvent& e)=0;
				virtual void action_job_failed(const sdpa::events::JobFailedEvent& e)=0;
				virtual void action_job_finished(const sdpa::events::JobFinishedEvent& e )=0;
				virtual void action_retrieve_job_results(const sdpa::events::RetrieveJobResultsEvent& e )=0;
		};
	}
}

#endif
