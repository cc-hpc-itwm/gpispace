#include "Worker.hpp"
#include <stdexcept>
#include <sdpa/util/util.hpp>
#include <sdpa/daemon/exceptions.hpp>
#include <iostream>

using namespace sdpa::daemon;

Worker::Worker(const worker_id_t name, const unsigned int rank, const location_t &location)
  : SDPA_INIT_LOGGER(std::string("sdpa.daemon.worker.") + name),
    name_(name),
    rank_(rank),
    location_(location),
    tstamp_(sdpa::util::now()),
    timedout_(false)
{

}

bool Worker::has_job( const sdpa::job_id_t& job_id )
{
	if( pending_.find(job_id) != pending_.end() )
		return true;

	if( submitted_.find(job_id) != submitted_.end() )
		return true;

	if( acknowledged_.find(job_id) != acknowledged_.end() )
		return true;

	return false;
}

void Worker::update()
{
  tstamp_ = sdpa::util::now();
  set_timedout (false);
}

void Worker::dispatch(const sdpa::job_id_t& jobId)
{
  SDPA_LOG_DEBUG("appending job(" << jobId.str() << ") to the pending queue");
  pending_.push(jobId);
}

bool Worker::acknowledge(const sdpa::job_id_t &job_id)
{
  update();

  try
  {
	  submitted().erase(job_id);
	  acknowledged().push(job_id);
	  SDPA_LOG_DEBUG("acknowledged job(" << job_id.str() << ")");
	  return true;
  }
  catch (const sdpa::daemon::NotFoundItem& ex)
  {
	  SDPA_LOG_WARN("The job " << job_id.str() << " could not be acknowledged. It was not found into the worker's submitted queue!");
	  return false;
  }
}


void Worker::delete_job(const sdpa::job_id_t &job_id)
{
	try {
		pending().erase(job_id);
		SDPA_LOG_DEBUG("Deleted " << job_id.str() << " from the submitted queue!");
	}
	catch( const sdpa::daemon::NotFoundItem& ex )
	{
		try {
			submitted().erase(job_id);
			SDPA_LOG_DEBUG("Deleted " << job_id.str() << " from the submitted queue!");
		}
		catch( const sdpa::daemon::NotFoundItem& ex )
		{
			try {
				acknowledged().erase(job_id);
				SDPA_LOG_DEBUG("Deleted " << job_id.str() << " from the acknowledged queue!");
			}
			catch( const sdpa::daemon::NotFoundItem& ex )
			{
				SDPA_LOG_ERROR("The job " << job_id.str() << " could not be found into any of the worker's queues!");
			}
		}
	}
}

sdpa::job_id_t Worker::get_next_job(const sdpa::job_id_t &last_job_id) throw (NoJobScheduledException)
{
	// acknowledge a previous job
	if(last_job_id != sdpa::job_id_t::invalid_job_id())
		acknowledge(last_job_id);

	try
	{
		// move the job from pending to submitted
		sdpa::job_id_t jobId = pending().pop();
		submitted().push(jobId);
		update();
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

void Worker::add_to_affinity_list(const pref_deg_t& pref_deg, const sdpa::job_id_t& jobId )
{
	// lock it first
	mi_affinity_list.insert( Worker::scheduling_preference_t( pref_deg, jobId ) );
}

void Worker::delete_from_affinity_list(const sdpa::job_id_t& job_id)
{
	// lock it first
	Worker::mi_ordered_jobIds& mi_jobIds = get<1>(mi_affinity_list);

	// delete the entry corresponding to job_id in jobs_preferring_this_
	mi_jobIds.erase(job_id);
}
