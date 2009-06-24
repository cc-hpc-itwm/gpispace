#include "Worker.hpp"
#include <stdexcept>

using namespace sdpa::daemon;

Worker::Worker(const std::string &name, const location_t &location)
  : SDPA_INIT_LOGGER(std::string("sdpa.daemon.worker.") + name_),
    name_(name),
    location_(location),
    tstamp_(sdpa::util::now()) {
    
}

void Worker::update(const sdpa::events::SDPAEvent &event) {
  tstamp_ = sdpa::util::now();
}

void Worker::dispatch(Job::ptr_t &job) {
  SDPA_LOG_DEBUG("appending job(" << job->id() << ") to the pending queue");
  pending_.push_back(job);
}

bool Worker::acknowledge(const Job::job_id_t &job_id) {
  // FIXME: lock the jobqueue and the submitted queue!
  for (JobQueue::iterator job(pending_.begin()); job != pending_.end(); job++) {
    if (job_id == (*job)->id()) {
      // remove it and put it to the submitted queue
      submitted_.push_back(*job);
      pending_.erase(job);
      SDPA_LOG_DEBUG("acknowledged job(" << job_id << ")");
      return true;
    }
  }
  SDPA_LOG_DEBUG("not acknowledged job(" << job_id << ")");
  return false;
}

Job::ptr_t Worker::get_next_job(const Job::job_id_t &last_job_id) {
  acknowledge(last_job_id);
  if (pending_.empty()) {
    throw std::runtime_error("pending queue is empty");
  } else {
    SDPA_LOG_DEBUG("next job(" << pending_.front()->id() << ")");
    return pending_.front();
  }
}

