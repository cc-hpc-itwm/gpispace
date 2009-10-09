#include "Worker.hpp"
#include <stdexcept>
#include <sdpa/daemon/exceptions.hpp>

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

void Worker::dispatch(const Job::ptr_t &pJob) {
  SDPA_LOG_DEBUG("appending job(" << pJob->id() << ") to the pending queue");

  pJob->put("worker", name());
  pending_.push(pJob);
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

void Worker::delete_job(const sdpa::job_id_t &job_id,  JobQueue& queue_arg )
{
	JobQueue::lock_type lockQ( queue_arg.mutex() );
    for (JobQueue::iterator job( queue_arg.begin()); job != queue_arg.end(); job++ ) {
    if (job_id == (*job)->id()) {
      // remove the job
      queue_arg.erase(job);
      SDPA_LOG_DEBUG("deleted job(" << job_id << ") from the acknowledged_ queue!");
      return;
    }
  }

  //should throw an exception
  SDPA_LOG_ERROR("The job " << job_id << " could not be found into the acknowledged_ queue!");
}


void Worker::delete_job(const sdpa::job_id_t &job_id) {
	JobQueue::lock_type lockAck(acknowledged().mutex());
  for (JobQueue::iterator job(acknowledged().begin()); job != acknowledged().end(); job++) {
    if (job_id == (*job)->id()) {
      // remove the job
      acknowledged().erase(job);
      SDPA_LOG_DEBUG("deleted job(" << job_id << ") from the acknowledged_ queue!");
      return;
    }
  }

  //should throw an exception
  SDPA_LOG_ERROR("The job " << job_id << " could not be found into the acknowledged_ queue!");
}

Job::ptr_t Worker::get_next_job(const sdpa::job_id_t &last_job_id) throw (NoJobScheduledException)
{
	  // acknowledge a previous job
	  if(last_job_id != sdpa::daemon::Job::invalid_job_id())
		  acknowledge(last_job_id);

	  try {
		  // move the job from pending to submitted
		  Job::ptr_t job(pending().pop());
		  submitted().push(job);
		  return job;
	  }
	  catch(QueueEmpty)
	  {
		  throw NoJobScheduledException(name());
	  }
}

