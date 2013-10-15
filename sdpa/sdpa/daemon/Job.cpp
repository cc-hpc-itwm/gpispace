#include "Job.hpp"

using namespace sdpa::daemon;

void Job::CancelJob(const sdpa::events::CancelJobEvent*) { }
void Job::CancelJobAck(const sdpa::events::CancelJobAckEvent*) { }
void Job::DeleteJob(const sdpa::events::DeleteJobEvent*, sdpa::daemon::IAgent*) { }
void Job::JobFailed(const sdpa::events::JobFailedEvent*) { }
void Job::JobFinished(const sdpa::events::JobFinishedEvent*) { }
void Job::QueryJobStatus(const sdpa::events::QueryJobStatusEvent*, sdpa::daemon::IAgent* ) { }
void Job::RetrieveJobResults(const sdpa::events::RetrieveJobResultsEvent*, sdpa::daemon::IAgent*) { }
void Job::Dispatch() { }
void Job::Reschedule() {};
void Job::WaitForResources() {};
