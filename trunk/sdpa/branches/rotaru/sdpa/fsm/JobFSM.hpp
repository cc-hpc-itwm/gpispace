#ifndef SDPASMCFSM_HPP
#define SDPASMCFSM_HPP 1


#include <sdpa/events/RunJobEvent.hpp>
#include <sdpa/events/JobFailedEvent.hpp>
#include <sdpa/events/QueryJobStatusEvent.hpp>
#include <sdpa/events/JobStatusAnswerEvent.hpp>
#include <sdpa/events/CancelJobEvent.hpp>
#include <sdpa/events/CancelJobAckEvent.hpp>
#include <sdpa/events/JobFinishedEvent.hpp>
#include <sdpa/events/ErrorEvent.hpp>
#include <sdpa/events/RetriveResultsEvent.hpp>
#include <sdpa/fsm/JobFSM_sm.h>
#include <sdpa/logging.hpp>

#include <list>

namespace sdpa {
	namespace fsm {
		class JobFSM {
			public:
				typedef std::tr1::shared_ptr<JobFSM> Ptr;

				JobFSM();
				virtual ~JobFSM();

				void action_dispatch(const sdpa::events::RunJobEvent& e);
				void action_cancel(const sdpa::events::CancelJobEvent& e);
				void action_query_status(const sdpa::events::QueryJobStatusEvent& e);
				void action_job_failed(const sdpa::events::JobFailedEvent& e);
				void action_job_finished(const sdpa::events::JobFinishedEvent& e );
				void action_retrieve_results(const sdpa::events::RetriveResultsEvent& e );
				void action_cancel_ack(const sdpa::events::CancelJobAckEvent& e);

				void WFE_NotifyNewJob();
				void WFE_GenListNextActiveSubJobs(); //assign unique global IDs!
				void WFE_NotifyJobFailed();


				sdpa::fsm::JobFSMContext& GetContext() { return m_fsmContext; }
			private:
				SDPA_DECLARE_LOGGER();
				sdpa::fsm::JobFSMContext m_fsmContext;
		};
	}
}

#endif
