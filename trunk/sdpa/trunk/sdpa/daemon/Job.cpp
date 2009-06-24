#include "Job.hpp"

using namespace sdpa::daemon;

const Job::job_id_t &Job::invalid_job_id() {
  static Job::job_id_t invalid_id("-");
  return invalid_id;
}
