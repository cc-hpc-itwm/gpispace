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
  // FIXME: lock the jobqueue and the submitted queue!
  JobQueue::lock_type lockSub(submitted_.mutex());
  JobQueue::lock_type lockPen(pending_.mutex());

  for (JobQueue::iterator job(pending_.begin()); job != pending_.end(); job++) {
    if (job_id == (*job)->id()) {
      // remove it and put it to the submitted queue
      submitted_.push(*job);
      pending_.erase(job);
      SDPA_LOG_DEBUG("acknowledged job(" << job_id << ")");
      return true;
    }
  }
  SDPA_LOG_DEBUG("not acknowledged job(" << job_id << ")");
  return false;
}

Job::ptr_t Worker::get_next_job(const sdpa::job_id_t &last_job_id) {
  acknowledge(last_job_id);
  return *pending_.begin();
  /*
  if (pending_.empty()) {
    throw std::runtime_error("pending queue is empty");
  } else {
    SDPA_LOG_DEBUG("next job(" << pending_.front()->id() << ")");
    return pending_.pop();
  }
  */
}

