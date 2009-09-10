#include "Job.hpp"

using namespace sdpa::daemon;

const sdpa::job_id_t &Job::invalid_job_id() {
  static sdpa::job_id_t invalid_id("-");
  return invalid_id;
}

void  Job::CancelJob() { }
void  Job::CancelJobAck() { }
void  Job::DeleteJob() { }
void  Job::JobFailed() { }
void  Job::JobFinished() { }
void  Job::QueryJobStatus() { }
void  Job::RetrieveJobResults() { }
void  Job::Dispatch() { }
