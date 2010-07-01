#include "Worker.hpp"
#include <stdexcept>
#include <sdpa/util/util.hpp>
#include <sdpa/daemon/exceptions.hpp>
#include <iostream>

using namespace sdpa::daemon;

Worker::Worker(const worker_id_t name, const int rank, const location_t &location)
  : SDPA_INIT_LOGGER(std::string("sdpa.daemon.worker.") + name),
    name_(name),
    rank_(rank),
    location_(location),
    tstamp_(sdpa::util::now()) {

}

bool Worker::has_job( const sdpa::job_id_t& job_id )
{
	for( JobQueue::iterator iter=pending_.begin(); iter!=pending_.end(); iter++ )
		if( *iter==job_id )
			return true;

	for( JobQueue::iterator iter=submitted_.begin(); iter!=submitted_.end(); iter++ )
		if( *iter==job_id )
			return true;

	for( JobQueue::iterator iter=acknowledged_.begin(); iter!=acknowledged_.end(); iter++ )
		if( *iter==job_id )
			return true;

	return false;
}

void Worker::update()
{
  tstamp_ = sdpa::util::now();
}

void Worker::dispatch(const sdpa::job_id_t& jobId)
{
  SDPA_LOG_DEBUG("appending job(" << jobId.str() << ") to the pending queue");
  pending_.push(jobId);
}

bool Worker::acknowledge(const sdpa::job_id_t &job_id)
{
  JobQueue::lock_type lockSub(submitted().mutex());
  JobQueue::lock_type lockAck(acknowledged().mutex());

  for (JobQueue::iterator iter = submitted().begin(); iter != submitted().end(); iter++) {
    if (job_id == *iter)
    {
      // remove it and put it to the acknowledged queue
      SDPA_LOG_DEBUG("appending job(" << job_id.str() << ") to the achknowledged queue");
      acknowledged().push(*iter);
      submitted().erase(iter);
      SDPA_LOG_DEBUG("acknowledged job(" << job_id.str() << ")");
      return true;
    }
  }
  SDPA_LOG_DEBUG("not acknowledged job(" << job_id.str() << ")");
  return false;
}

bool delete_(const sdpa::job_id_t &job_id, Worker::JobQueue& queue_arg )
{
	Worker::JobQueue::lock_type lockQ( queue_arg.mutex() );
    for ( Worker::JobQueue::iterator iter = queue_arg.begin(); iter != queue_arg.end(); iter++ )
    {
		if( job_id == *iter )
		{
		  // remove the job
		  queue_arg.erase(iter);
		  return true;
		}
    }

    return false;
}


void Worker::delete_job(const sdpa::job_id_t &job_id)
{

	if( delete_(job_id, pending()))
		SDPA_LOG_DEBUG("Deleted " << job_id.str() << " from the worker's the pending queue!");
	else if( delete_(job_id, submitted()))
		SDPA_LOG_DEBUG("Deleted " << job_id.str() << " from the worker's the submitted queue!");
	else if( delete_(job_id, acknowledged()))
		SDPA_LOG_DEBUG("Deleted " << job_id.str() << " from the worker's the acknowledged queue!");
	else
		SDPA_LOG_ERROR("The job " << job_id.str() << " could not be found into any of the worker's queues!");
}

sdpa::job_id_t Worker::get_next_job(const sdpa::job_id_t &last_job_id) throw (NoJobScheduledException)
{
	  // acknowledge a previous job
	  if(last_job_id != sdpa::job_id_t::invalid_job_id())
		  acknowledge(last_job_id);

	  try {
		  // move the job from pending to submitted
		  sdpa::job_id_t jobId = pending().pop();
		  submitted().push(jobId);
		  return jobId;
	  }
	  catch(const QueueEmpty& )
	  {
		  throw NoJobScheduledException(name());
	  }
}

void Worker::print()
{
	// print the values of the restored job queue
	std::cout<<name()<<"'s queues:"<<std::endl;
	std::cout<<"Pending jobs:"<<std::endl;
	pending().print();
	std::cout<<"Submitted jobs:"<<std::endl;
	submitted().print();
	std::cout<<"Acknowledged jobs:"<<std::endl;
	acknowledged().print();
}
