#include "Job.hpp"

using namespace sdpa::daemon;

const sdpa::job_id_t &Job::invalid_job_id() {
  static sdpa::job_id_t invalid_id("-");
  return invalid_id;
}

void Job::CancelJob(const sdpa::events::CancelJobEvent* pEvt) { }
void Job::CancelJobAck(const sdpa::events::CancelJobAckEvent* pEvt) { }
void Job::DeleteJob(const sdpa::events::DeleteJobEvent* pEvt) { }
void Job::JobFailed(const sdpa::events::JobFailedEvent* pEvt) { }
void Job::JobFinished(const sdpa::events::JobFinishedEvent* pEvt) { }
void Job::QueryJobStatus(const sdpa::events::QueryJobStatusEvent* pEvt) { }
void Job::RetrieveJobResults(const sdpa::events::RetrieveJobResultsEvent* pEvt) { }
void Job::Dispatch(const sdpa::events::SubmitJobAckEvent*) { }
