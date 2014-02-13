#include "Worker.hpp"
#include <stdexcept>
#include <sdpa/daemon/exceptions.hpp>
#include <fhg/util/now.hpp>
#include <iostream>
#include <algorithm>

#include <boost/foreach.hpp>

using namespace sdpa;
using namespace sdpa::daemon;

Worker::Worker(	const worker_id_t& name,
				const boost::optional<unsigned int>& cap,
				const unsigned int& rank,
				const location_t &location)
  : _logger (fhg::log::Logger::get ("sdpa.daemon.worker." + name)),
    name_(name),
    capacity_(cap),
    rank_(rank),
    location_(location),
    tstamp_(fhg::util::now()),
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
  tstamp_ = fhg::util::now();
  set_timedout (false);
}

void Worker::submit(const sdpa::job_id_t& jobId)
{
  lock_type lock(mtx_);
  DLLOG (TRACE, _logger, "appending job(" << jobId << ") to the submitted queue");
  setLastTimeServed(fhg::util::now());
  submitted_.push(jobId);
}

bool Worker::acknowledge(const sdpa::job_id_t &job_id)
{
  lock_type lock(mtx_);
  try
  {
      acknowledged_.push(job_id);
      submitted_.erase(job_id);
      DLLOG (TRACE, _logger, "acknowledged job(" << job_id << ")");
      return true;
  }
  catch (const sdpa::daemon::NotFoundItem& ex)
  {
    LLOG (WARN, _logger, "The job " << job_id << " could not be acknowledged. It was not found into the worker's submitted queue!");
      return false;
  }
}

void Worker::deleteJob(const sdpa::job_id_t &job_id)
{
  lock_type lock(mtx_);
  submitted_.erase (job_id);
  acknowledged_.erase (job_id);
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
  if(recvCpbSet.empty())
    return false;

  lock_type const _ (mtx_);

  bool bModified = false;
  BOOST_FOREACH (sdpa::Capability const& capability, recvCpbSet)
  {
      sdpa::capabilities_set_t::iterator itwcpb = capabilities_.find(capability);
      if( itwcpb == capabilities_.end() ) {
          capabilities_.insert (capability);
          bModified = true;
      }
      else
	if( itwcpb->depth()>capability.depth() ) {
	    const_cast<sdpa::capability_t&>(*itwcpb).setDepth(capability.depth());
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

          //LLOG (TRACE, _logger, "worker " << name() << " lost capability: "
          //<< *it << " (" << std::count(capabilities_.begin(), capabilities_.end(), *it) << ")");
      }
  }
}

bool Worker::hasCapability(const std::string& cpbName)
{
  lock_type lock(mtx_);

  return std::find_if ( capabilities_.begin(), capabilities_.end()
                      , boost::bind (&capability_t::name, _1) == cpbName
                      ) != capabilities_.end();
}

void Worker::reserve()
{
  lock_type lock(mtx_);
  reserved_ = true;
  last_schedule_time_ = fhg::util::now();
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
