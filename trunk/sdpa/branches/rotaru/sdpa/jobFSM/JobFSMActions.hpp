#ifndef JOBFSMITF_HPP
#define JOBFSMITF_HPP 1

#include <sdpa/events/RunJobEvent.hpp>
#include <sdpa/events/JobFailedEvent.hpp>
#include <sdpa/events/QueryJobStatusEvent.hpp>
#include <sdpa/events/JobStatusAnswerEvent.hpp>
#include <sdpa/events/CancelJobEvent.hpp>
#include <sdpa/events/CancelJobAckEvent.hpp>
#include <sdpa/events/JobFinishedEvent.hpp>
#include <sdpa/events/ErrorEvent.hpp>
#include <sdpa/events/RetriveResultsEvent.hpp>

namespace sdpa {
	namespace fsm {
		class JobFSMActions
		{
			public:
				virtual void action_run_job(const sdpa::events::RunJobEvent& e);
				virtual void action_cancel_job(const sdpa::events::CancelJobEvent& e);
				virtual void action_cancel_job_ack(const sdpa::events::CancelJobAckEvent& e);
				virtual void action_query_job_status(const sdpa::events::QueryJobStatusEvent& e);
				virtual void action_job_failed(const sdpa::events::JobFailedEvent& e);
				virtual void action_job_finished(const sdpa::events::JobFinishedEvent& e );
				virtual void action_retrieve_job_results(const sdpa::events::RetriveResultsEvent& e );
				virtual void WFE_NotifyNewJob();
				virtual void WFE_GenListNextActiveSubJobs(); //assign unique global IDs!
				virtual void WFE_NotifyJobFailed();
			};
		}
	}

#endif
