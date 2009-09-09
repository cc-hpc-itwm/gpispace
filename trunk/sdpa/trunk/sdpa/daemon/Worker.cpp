#include "Worker.hpp"
#include <stdexcept>

using namespace sdpa::daemon;

Worker::Worker(const worker_id_t &name, const location_t &location)
  : SDPA_INIT_LOGGER(std::string("sdpa.daemon.worker.") + name),
    name_(name),
    location_(location),
    tstamp_(sdpa::util::now()) {
    
}

void Worker::update(const sdpa::events::SDPAEvent &event) {
  tstamp_ = sdpa::util::now();
}

void Worker::dispatch(const Job::ptr_t &job) {
  SDPA_LOG_DEBUG("appending job(" << job->id() << ") to the pending queue");
  pending_.push(job);
}

bool Worker::acknowledge(const sdpa::job_id_t &job_id) {
  JobQueue::lock_type lockSub(submitted().mutex());
  JobQueue::lock_type lockAck(acknowledged().mutex());

  for (JobQueue::iterator job(submitted().begin()); job != submitted().end(); job++) {
    if (job_id == (*job)->id()) {
      // remove it and put it to the acknowledged queue
      acknowledged().push(*job);
      submitted().erase(job);
      SDPA_LOG_DEBUG("acknowledged job(" << job_id << ")");
      return true;
    }
  }
  SDPA_LOG_DEBUG("not acknowledged job(" << job_id << ")");
  return false;
}

Job::ptr_t Worker::get_next_job(const sdpa::job_id_t &last_job_id) {
  // acknowledge a previous job
  acknowledge(last_job_id);

  // move the job from pending to submitted
  Job::ptr_t job(pending().try_pop());
  submitted().push(job);
  return job;
}

