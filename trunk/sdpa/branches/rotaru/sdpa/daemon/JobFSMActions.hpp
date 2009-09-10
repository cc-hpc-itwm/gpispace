#ifndef JOB_FSM_ACTIONS_HPP
#define JOB_FSM_ACTIONS_HPP 1

#include <sdpa/events/SubmitJobEvent.hpp>
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
				virtual void action_run_job()=0;
				virtual void action_cancel_job()=0;
				virtual void action_cancel_job_ack()=0;
				virtual void action_delete_job()=0;
				virtual void action_query_job_status()=0;
				virtual void action_job_failed()=0;
				virtual void action_job_finished()=0;
				virtual void action_retrieve_job_results()=0;
		};
	}
}

#endif
