#include <sdpa/daemon/Worker.hpp>
#include <stdexcept>
#include <sdpa/daemon/exceptions.hpp>
#include <fhg/util/now.hpp>
#include <iostream>
#include <algorithm>

#include <boost/foreach.hpp>

using namespace sdpa;
using namespace sdpa::daemon;

Worker::Worker(	const worker_id_t& name,
				const boost::optional<unsigned int>& cap)
  : name_(name),
    capacity_(cap),
    tstamp_(fhg::util::now()),
    last_schedule_time_(0),
    disconnected_(false),
    reserved_(false)
{

}

bool Worker::has_job( const sdpa::job_id_t& job_id )
{
  lock_type lock(mtx_);
  return submitted_.has_item(job_id) || acknowledged_.has_item(job_id);
}

void Worker::update()
{
  lock_type lock(mtx_);
  tstamp_ = fhg::util::now();
}

void Worker::submit(const sdpa::job_id_t& jobId)
{
  lock_type lock(mtx_);
  submitted_.push(jobId);
}

bool Worker::acknowledge(const sdpa::job_id_t &job_id)
{
  lock_type lock(mtx_);
      acknowledged_.push(job_id);
      return submitted_.erase(job_id) > 0;
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
  return submitted_.size() + acknowledged_.size();
}

const sdpa::capabilities_set_t& Worker::capabilities() const
{
  lock_type lock(mtx_);
  return capabilities_;
}

bool Worker::addCapabilities( const capabilities_set_t& recvCpbSet )
{
  lock_type const _ (mtx_);

  bool bModified = false;
  BOOST_FOREACH (sdpa::Capability const& capability, recvCpbSet)
  {
    sdpa::capabilities_set_t::iterator itwcpb (capabilities_.find (capability));
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
  lock_type lock(mtx_);
  for(capabilities_set_t::const_iterator it = cpbset.begin(); it != cpbset.end(); ++it ) {
      capabilities_set_t::iterator itwcpb = capabilities_.find(*it);
      if( itwcpb != capabilities_.end() ) {
          capabilities_.erase(itwcpb);

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

namespace
{
  void addToList (Worker::JobQueue* pQueue, sdpa::job_id_list_t& jobList)
  {
    while (!pQueue->empty())
    {
      jobList.push_back (pQueue->pop());
    }
  }
}

sdpa::job_id_list_t Worker::getJobListAndCleanQueues()
{
  lock_type const _ (mtx_);
  sdpa::job_id_list_t listAssignedJobs;

  addToList (&submitted_, listAssignedJobs);
  addToList (&acknowledged_, listAssignedJobs);

  return listAssignedJobs;
}
