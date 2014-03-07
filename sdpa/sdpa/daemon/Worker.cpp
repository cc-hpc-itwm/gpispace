#include <sdpa/daemon/Worker.hpp>
#include <stdexcept>
#include <sdpa/daemon/exceptions.hpp>
#include <fhg/util/now.hpp>
#include <iostream>
#include <algorithm>

#include <boost/foreach.hpp>

namespace sdpa
{
  namespace daemon
  {
Worker::Worker(	const worker_id_t& name,
				const boost::optional<unsigned int>& cap
              , const capabilities_set_t& capabilities
              )
  : name_(name),
    capacity_(cap)
  , capabilities_ (capabilities)
  , last_schedule_time_(0),
    reserved_(false)
{

}

bool Worker::has_job( const job_id_t& job_id )
{
  lock_type const _ (mtx_);
  return submitted_.count (job_id) || acknowledged_.count (job_id);
}

void Worker::submit(const job_id_t& jobId)
{
  lock_type const _ (mtx_);
  submitted_.insert (jobId);
  reserve();
}

void Worker::acknowledge(const job_id_t &job_id)
{
  lock_type const _ (mtx_);
  if (submitted_.erase (job_id) == 0)
  {
    throw JobNotFoundException();
  }
  acknowledged_.insert (job_id);
}

void Worker::deleteJob(const job_id_t &job_id)
{
  lock_type const _ (mtx_);
  submitted_.erase (job_id);
  acknowledged_.erase (job_id);
  free();
}

const capabilities_set_t& Worker::capabilities() const
{
  lock_type const _ (mtx_);
  return capabilities_;
}

bool Worker::addCapabilities( const capabilities_set_t& recvCpbSet )
{
  lock_type const _ (mtx_);

  bool bModified = false;
  BOOST_FOREACH (Capability const& capability, recvCpbSet)
  {
    capabilities_set_t::iterator itwcpb (capabilities_.find (capability));
    if (itwcpb == capabilities_.end())
    {
      capabilities_.insert (capability);
      bModified = true;
    }
    else if (itwcpb->depth() > capability.depth())
    {
      capabilities_.erase (itwcpb);
      capabilities_.insert (capability);
      bModified = true;
    }
  }

  return bModified;
}

void Worker::removeCapabilities( const capabilities_set_t& cpbset )
{
  lock_type const _ (mtx_);
  BOOST_FOREACH (Capability const& capability, cpbset)
  {
    capabilities_.erase (capability);
  }
}

bool Worker::hasCapability(const std::string& cpbName)
{
  lock_type const _ (mtx_);

  return std::find_if ( capabilities_.begin(), capabilities_.end()
                      , boost::bind (&capability_t::name, _1) == cpbName
                      ) != capabilities_.end();
}

void Worker::reserve()
{
  lock_type const _ (mtx_);
  reserved_ = true;
  last_schedule_time_ = fhg::util::now();
}

bool Worker::isReserved()
{
  lock_type const _ (mtx_);
  return reserved_;
}

void Worker::free()
{
  lock_type const _ (mtx_);
  reserved_ = false;
}

std::set<job_id_t> Worker::getJobListAndCleanQueues()
{
  lock_type const _ (mtx_);
  std::set<job_id_t> listAssignedJobs;

  listAssignedJobs.insert (submitted_.begin(), submitted_.end());
  listAssignedJobs.insert (acknowledged_.begin(), acknowledged_.end());
  submitted_.clear();
  acknowledged_.clear();

  return listAssignedJobs;
}
  }
}
