#include "Worker.hpp"
#include <stdexcept>
#include <sdpa/util/util.hpp>
#include <sdpa/daemon/exceptions.hpp>
#include <iostream>
#include <algorithm>

using namespace sdpa;
using namespace sdpa::daemon;

Worker::Worker(	const worker_id_t& name,
				const boost::optional<unsigned int>& cap,
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
  return submitted_.has_item(job_id) || acknowledged_.has_item(job_id);
}

bool Worker::isJobSubmittedOrAcknowleged( const sdpa::job_id_t& job_id )
{
  lock_type lock(mtx_);
  return submitted_.has_item(job_id) || acknowledged_.has_item(job_id);
}

void Worker::update()
{
  lock_type lock(mtx_);
  tstamp_ = sdpa::util::now();
  set_timedout (false);
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
      acknowledged_.push(job_id);
      submitted_.erase(job_id);
      DMLOG(TRACE, "acknowledged job(" << job_id.str() << ")");
      return true;
  }
  catch (const sdpa::daemon::NotFoundItem& ex)
  {
      SDPA_LOG_WARN("The job " << job_id.str() << " could not be acknowledged. It was not found into the worker's submitted queue!");
      return false;
  }
}

void Worker::deleteJob(const sdpa::job_id_t &job_id)
{
  lock_type lock(mtx_);
  submitted_.erase (job_id);
  acknowledged_.erase (job_id);
}

void Worker::print()
{
  lock_type lock(mtx_);
  // print the values of the restored job queue
  if( submitted_.size() ) {
      SDPA_LOG_INFO("There are still "<<submitted_.size()<<" submitted jobs:");
      submitted_.print();
  }

  if(acknowledged_.size()) {
      SDPA_LOG_INFO("There are still "<<acknowledged_.size()<<" acknowledged jobs:");
      acknowledged_.print();
  }
}

unsigned int Worker::nbAllocatedJobs()
{
  lock_type lock(mtx_);
  unsigned int nJobs = /*pending().size() + */ submitted_.size() + acknowledged_.size();
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
  for(sdpa::capabilities_set_t::iterator it = recvCpbSet.begin(); it != recvCpbSet.end(); ++it) {
      sdpa::capabilities_set_t::iterator itwcpb = capabilities_.find(*it);
      if( itwcpb == capabilities_.end() ) {
          capabilities_.insert(*it);
          DMLOG (TRACE, "The worker "<<name()<<" gained the capability:"<<*it);
          bModified = true;
      }
      else
	if( itwcpb->depth()>it->depth() ) {
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
  for(capabilities_set_t::const_iterator it = cpbset.begin(); it != cpbset.end(); ++it ) {
      capabilities_set_t::iterator itwcpb = capabilities_.find(*it);
      if( itwcpb != capabilities_.end() ) {
          capabilities_.erase(itwcpb);

          //LOG( TRACE, "worker " << name() << " lost capability: "
          //<< *it << " (" << std::count(capabilities_.begin(), capabilities_.end(), *it) << ")");
      }
  }
}

bool Worker::hasCapability(const std::string& cpbName)
{
  lock_type lock(mtx_);
  bool bHasCpb = false;
  for( sdpa::capabilities_set_t::iterator it = capabilities_.begin();!bHasCpb && it != capabilities_.end();it++ ) {
      if(false) {
          if( it->name() == cpbName && it->owner() == name() )
            bHasCpb = true;
      }
      else {
          if(it->name() == cpbName)
            bHasCpb = true;
      }
  }

  return bHasCpb;
}

void Worker::reserve()
{
  lock_type lock(mtx_);
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
