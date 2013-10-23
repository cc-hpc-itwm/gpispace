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
    last_schedule_time_(0),
    timedout_(false),
    disconnected_(false),
    reserved_(false)
{

}

bool Worker::has_job( const sdpa::job_id_t& job_id )
{
	lock_type lock(mtx_);
	return pending_.has_item(job_id) ||  submitted_.has_item(job_id) || acknowledged_.has_item(job_id);
}

bool Worker::isJobSubmittedOrAcknowleged( const sdpa::job_id_t& job_id )
{
  lock_type lock(mtx_);
  return submitted_.has_item(job_id) || acknowledged_.has_item(job_id);
  return false;
}

void Worker::update()
{
	lock_type lock(mtx_);
	tstamp_ = sdpa::util::now();
	set_timedout (false);
}

void Worker::dispatch(const sdpa::job_id_t& jobId)
{
	lock_type lock(mtx_);
	DMLOG (TRACE, "appending job(" << jobId.str() << ") to the pending queue");
	setLastScheduleTime(sdpa::util::now());
	pending_.push(jobId);
}

void Worker::submit(const sdpa::job_id_t& jobId)
{
	lock_type lock(mtx_);
	DMLOG (TRACE, "appending job(" << jobId.str() << ") to the submitted queue");
	setLastTimeServed(sdpa::util::now());
	submitted_.push(jobId);
}

bool Worker::acknowledge(const sdpa::job_id_t &job_id)
{
	lock_type lock(mtx_);
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
	lock_type lock(mtx_);
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

void Worker::print()
{
	lock_type lock(mtx_);
	// print the values of the restored job queue
	if(pending().size())
	{
		SDPA_LOG_INFO("There are still "<<pending().size()<<" pending jobs:");
		pending().print();
	}

	if( submitted().size() )
	{
		SDPA_LOG_INFO("There are still "<<submitted().size()<<" submitted jobs:");
		submitted().print();
	}

	if(acknowledged().size())
	{
		SDPA_LOG_INFO("There are still "<<acknowledged().size()<<" acknowledged jobs:");
		acknowledged().print();
	}
}

unsigned int Worker::nbAllocatedJobs()
{
	lock_type lock(mtx_);
	unsigned int nJobs = /*pending().size() + */ submitted().size() + acknowledged().size();
	return nJobs;
}

const sdpa::capabilities_set_t& Worker::capabilities() const
{
	lock_type lock(mtx_);
	return capabilities_;
}

bool Worker::addCapabilities( const capabilities_set_t& recvCpbSet )
{
	lock_type lock(mtx_);
	if(recvCpbSet.empty())
		return false;

	bool bModified = false;
	for(sdpa::capabilities_set_t::iterator it = recvCpbSet.begin(); it != recvCpbSet.end(); ++it)
	{
		sdpa::capabilities_set_t::iterator itwcpb = capabilities_.find(*it);
		if( itwcpb == capabilities_.end() )
		{
			capabilities_.insert(*it);
			DMLOG (TRACE, "The worker "<<name()<<" gained the capability:"<<*it);
			bModified = true;
		}
		else
			if( itwcpb->depth()>it->depth() )
			{
				SDPA_LOG_INFO("Worker " << name() << ": updated the depth of the capability:\n   "<<*it<<" from "<<itwcpb->depth()<<" to "<<it->depth() );
				const_cast<sdpa::capability_t&>(*itwcpb).setDepth(it->depth());
				bModified = true;
			}
	}

	return bModified;
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

void Worker::reserve()
{
	lock_type lock(mtx_);
	//DMLOG(TRACE, "Marked the worker "<<name()<<" as reserved ");
	reserved_ = true;
	last_schedule_time_ = sdpa::util::now();
}

bool Worker::isReserved()
{
	lock_type lock(mtx_);
	return reserved_;
}

void Worker::free()
{
	lock_type lock(mtx_);
	reserved_ = false;
}

