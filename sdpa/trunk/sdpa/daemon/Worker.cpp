#include "Worker.hpp"
#include <stdexcept>
#include <sdpa/util/util.hpp>
#include <sdpa/daemon/exceptions.hpp>
#include <iostream>
#include <algorithm>

using namespace sdpa;
using namespace sdpa::daemon;

Worker::Worker(	const worker_id_t& name,
				const unsigned int& cap,
				const unsigned int& rank,
				const sdpa::worker_id_t& agent_uuid,
				const location_t &location)
  : SDPA_INIT_LOGGER(std::string("sdpa.daemon.worker.") + name),
    name_(name),
    capacity_(cap),
    rank_(rank),
    agent_uuid_(agent_uuid),
    location_(location),
    tstamp_(sdpa::util::now()),
    last_time_served_(0),
    timedout_(false),
    disconnected_(false)
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

bool Worker::is_job_acknowleged( const sdpa::job_id_t& job_id )
{
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
  //update();

  try
  {
	  acknowledged().push(job_id);
	  submitted().erase(job_id);
	  DMLOG(TRACE, "acknowledged job(" << job_id.str() << ")");
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
	DLOG(TRACE, "deleting job " << job_id << " from worker " << name());

	if( pending().erase(job_id) )
	{
		DLOG(TRACE, "removed the job "<<job_id.str()<<" from pending queue");
	}

	if( submitted().erase(job_id) )
	{
		DLOG(TRACE, "removed the job "<<job_id.str()<<" submitted queue");
	}

	if( acknowledged().erase(job_id) )
	{
		DLOG(TRACE, "removed the job "<<job_id.str()<<" acknowledged queue");
	}
}

sdpa::job_id_t Worker::get_next_job(const sdpa::job_id_t &last_job_id) throw (NoJobScheduledException)
{
	// acknowledge a previous job
	if( !last_job_id.str().empty() && last_job_id != sdpa::job_id_t::invalid_job_id())
		acknowledge(last_job_id);

	try
	{
		// move the job from pending to submitted
		sdpa::job_id_t jobId = pending().pop();
		submitted().push(jobId);
		//update();
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
	std::cout<<"There are still "<<pending().size()<<" pending jobs:"<<std::endl;
	pending().print();
	std::cout<<"There are still "<<submitted().size()<<" submitted jobs:"<<std::endl;
	submitted().print();
	std::cout<<"There are still "<<acknowledged().size()<<" acknowledged jobs:"<<std::endl;
	acknowledged().print();
}

unsigned int Worker::nbAllocatedJobs()
{
	unsigned int nJobs = pending().size() + submitted().size() + acknowledged().size();

	return nJobs;
}

const sdpa::capabilities_set_t& Worker::capabilities() const
{
	lock_type lock(mtx_);
	return capabilities_;
}

bool Worker::addCapabilities( const capabilities_set_t& cpbset )
{
	lock_type lock(mtx_);
	if(cpbset.empty())
		return true;

	bool bIsSubset = true;
	for(sdpa::capabilities_set_t::iterator it = cpbset.begin(); it != cpbset.end(); ++it)
	{
		sdpa::capabilities_set_t::iterator itwcpb = capabilities_.find(*it);
		if( itwcpb == capabilities_.end() )
		{
			capabilities_.insert(*it);
			//LOG( TRACE, "The worker " << name() << " gained capability: "<< *it );
			bIsSubset = false;
		}
	}

	return bIsSubset;
}

void Worker::removeCapabilities( const capabilities_set_t& cpbset )
{
	 lock_type lock(mtx_);
	for(capabilities_set_t::const_iterator it = cpbset.begin(); it != cpbset.end(); ++it )
	{
		capabilities_set_t::iterator itwcpb = capabilities_.find(*it);
		if( itwcpb != capabilities_.end() )
		{
			capabilities_.erase(itwcpb);

			//LOG( TRACE, "worker " << name() << " lost capability: "
    			//<< *it << " (" << std::count(capabilities_.begin(), capabilities_.end(), *it) << ")");
		}
		/*else
		{
			// do nothing
			LOG(WARN, "The worker "<<name()<<" doesn't possess capability: " <<*it);
		}*/
	}
}

bool Worker::hasCapability(const std::string& cpbName, bool bOwn)
{
	lock_type lock(mtx_);
	bool bHasCpb = false;
	for( sdpa::capabilities_set_t::iterator it = capabilities_.begin();
		 !bHasCpb && it != capabilities_.end();
		 it++ )
	{
		if(bOwn)
		{
			if( it->name() == cpbName && it->owner() == name() )
				bHasCpb = true;
		}
		else
		{
			if(it->name() == cpbName)
				bHasCpb = true;
		}
	}

	return bHasCpb;
}


