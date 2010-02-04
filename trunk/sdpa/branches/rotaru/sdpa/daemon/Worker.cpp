#include "Worker.hpp"
#include <stdexcept>
#include <sdpa/util/util.hpp>
#include <sdpa/daemon/exceptions.hpp>

using namespace sdpa::daemon;

Worker::Worker(const worker_id_t &name, const location_t &location)
  : SDPA_INIT_LOGGER(std::string("sdpa.daemon.worker.") + name),
    name_(name),
    location_(location),
    tstamp_(sdpa::util::now()) {

}

void Worker::update(const sdpa::events::SDPAEvent &) {
  tstamp_ = sdpa::util::now();
}

void Worker::dispatch(const Job::ptr_t &pJob) {
  SDPA_LOG_DEBUG("appending job(" << pJob->id() << ") to the pending queue");

  pJob->worker() = name();
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

bool delete_(const sdpa::job_id_t &job_id,  Worker::JobQueue& queue_arg )
{
	Worker::JobQueue::lock_type lockQ( queue_arg.mutex() );
    for ( Worker::JobQueue::iterator job( queue_arg.begin()); job != queue_arg.end(); job++ )
    {
		if (job_id == (*job)->id())
		{
		  // remove the job
		  queue_arg.erase(job);
		  return true;
		}
    }

    return false;
}


void Worker::delete_job(const sdpa::job_id_t &job_id)
{

	if( delete_(job_id, pending()))
		SDPA_LOG_DEBUG("Deleted " << job_id << " from the worker's the pending queue!");
	else if( delete_(job_id, submitted()))
		SDPA_LOG_DEBUG("Deleted " << job_id << " from the worker's the submitted queue!");
	else if( delete_(job_id, acknowledged()))
		SDPA_LOG_DEBUG("Deleted " << job_id << " from the worker's the acknowledged queue!");
	else
		SDPA_LOG_ERROR("The job " << job_id << " could not be found into any of the worker's queues!");
}

Job::ptr_t Worker::get_next_job(const sdpa::job_id_t &last_job_id) throw (NoJobScheduledException)
{
	  // acknowledge a previous job
	  if(last_job_id != sdpa::job_id_t::invalid_job_id())
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

