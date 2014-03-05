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
  return std::find (submitted_.begin(), submitted_.end(), job_id) != submitted_.end()
    || std::find (acknowledged_.begin(), acknowledged_.end(), job_id) != acknowledged_.end();
}

void Worker::submit(const job_id_t& jobId)
{
  lock_type const _ (mtx_);
  submitted_.push_back (jobId);
}

void Worker::acknowledge(const job_id_t &job_id)
{
  lock_type const _ (mtx_);
  const job_id_list_t::iterator it
    (std::find (submitted_.begin(), submitted_.end(), job_id));
  if (it == submitted_.end())
  {
    throw JobNotFoundException();
  }
  submitted_.erase (it);
  acknowledged_.push_back (job_id);
}

void Worker::deleteJob(const job_id_t &job_id)
{
  lock_type const _ (mtx_);
  {
    const job_id_list_t::iterator it
      (std::find (submitted_.begin(), submitted_.end(), job_id));
    if (it != submitted_.end())
    {
      submitted_.erase (it);
    }
  }
  {
    const job_id_list_t::iterator it
      (std::find (acknowledged_.begin(), acknowledged_.end(), job_id));
    if (it != acknowledged_.end())
    {
      acknowledged_.erase (it);
    }
  }
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

job_id_list_t Worker::getJobListAndCleanQueues()
{
  lock_type const _ (mtx_);
  job_id_list_t listAssignedJobs;

  BOOST_FOREACH (job_id_t const& id, submitted_)
  {
    listAssignedJobs.push_back (id);
  }
  BOOST_FOREACH (job_id_t const& id, acknowledged_)
  {
    listAssignedJobs.push_back (id);
  }
  submitted_.clear();
  acknowledged_.clear();

  return listAssignedJobs;
}
  }
}
