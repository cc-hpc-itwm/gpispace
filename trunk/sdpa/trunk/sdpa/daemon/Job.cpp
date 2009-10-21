#include "Job.hpp"

using namespace sdpa::daemon;


void Job::CancelJob(const sdpa::events::CancelJobEvent*) { }
void Job::CancelJobAck(const sdpa::events::CancelJobAckEvent*) { }
void Job::DeleteJob(const sdpa::events::DeleteJobEvent*) { }
void Job::JobFailed(const sdpa::events::JobFailedEvent*) { }
void Job::JobFinished(const sdpa::events::JobFinishedEvent*) { }
void Job::QueryJobStatus(const sdpa::events::QueryJobStatusEvent*) { }
void Job::RetrieveJobResults(const sdpa::events::RetrieveJobResultsEvent*) { }
void Job::Dispatch() { }
