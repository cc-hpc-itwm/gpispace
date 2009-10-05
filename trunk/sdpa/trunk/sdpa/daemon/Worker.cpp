#include "Worker.hpp"
#include <fhglog/fhglog.hpp>
#include <stdexcept>

using namespace sdpa::daemon;

Worker::Worker(const worker_id_t &a_name, const location_t &a_location)
  : name_(a_name)
  , location_(a_location)
  , tstamp_(sdpa::util::now())
  , pending_()
  , submitted_()
  , acknowledged_()
{
    
}

void Worker::update(const sdpa::events::SDPAEvent &) {
  tstamp_ = sdpa::util::now();
}

void Worker::dispatch(const Job::ptr_t &job) {
  LOG(DEBUG, "appending job(" << job->id() << ") to the pending queue");
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
      LOG(DEBUG, "acknowledged job(" << job_id << ")");
      return true;
    }
  }
  LOG(DEBUG, "not acknowledged job(" << job_id << ")");
  return false;
}

Job::ptr_t Worker::get_next_job(const sdpa::job_id_t &last_job_id) {
  // acknowledge a previous job
  acknowledge(last_job_id);

  // move the job from pending to submitted
  Job::ptr_t job(pending().pop());
  submitted().push(job);
  return job;
}

