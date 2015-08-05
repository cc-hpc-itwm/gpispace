#include <sdpa/daemon/Worker.hpp>
#include <stdexcept>
#include <fhg/util/now.hpp>
#include <iostream>
#include <algorithm>
#include <numeric>

namespace sdpa
{
  namespace daemon
  {
    Worker::Worker ( const capabilities_set_t& capabilities
                   , unsigned long allocated_shared_memory_size
                   , const bool children_allowed
                   , const std::string& hostname
                   )
      : capabilities_ (capabilities)
      , _allocated_shared_memory_size (allocated_shared_memory_size)
      , _children_allowed (children_allowed)
      , _hostname (hostname)
      , last_time_served_ (0)
      , reserved_ (false)
      , backlog_full_ (false)
    {

    }

    bool Worker::has_job( const job_id_t& job_id ) const
    {
      return pending_.count (job_id)
        || submitted_.count (job_id)
        || acknowledged_.count (job_id);
    }

    bool Worker::has_pending_jobs() const
    {
      return !pending_.empty();
    }

    void Worker::assign (const job_id_t& jobId)
    {
      pending_.insert (jobId);
    }

    void Worker::submit (const job_id_t& jobId)
    {
      if (!pending_.erase (jobId))
      {
        throw std::runtime_error ("subnmit: no pending job with the id " + jobId + " was found!");
      }
      submitted_.insert (jobId);
      if (!_children_allowed)
      {
        reserved_ = true;
        last_time_served_ = fhg::util::now();
      }
    }

    void Worker::acknowledge (const job_id_t &job_id)
    {
      if (submitted_.erase (job_id) == 0)
      {
	throw std::runtime_error ("acknowledge: job not in submitted queue");
      }
      acknowledged_.insert (job_id);
    }

    void Worker::deleteJob(const job_id_t &job_id)
    {
      pending_.erase (job_id);
      submitted_.erase (job_id);
      acknowledged_.erase (job_id);
      if (!_children_allowed)
      {
        reserved_ = false;
      }
    }

    const capabilities_set_t& Worker::capabilities() const
    {
      return capabilities_;
    }

    bool Worker::addCapabilities( const capabilities_set_t& recvCpbSet )
    {

      bool bModified = false;
      for (Capability const& capability : recvCpbSet)
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

    bool Worker::removeCapabilities( const capabilities_set_t& cpbset )
    {
      capabilities_set_t::size_type removed (0);
      for (Capability const& capability : cpbset)
      {
        removed += capabilities_.erase (capability);
      }
      return removed != 0;
    }

    bool Worker::hasCapability(const std::string& cpbName) const
    {
      return std::find_if ( capabilities_.begin(), capabilities_.end()
                          , [&cpbName] (capability_t const& cap)
                          {
                            return cap.name() == cpbName;
                          }
                          ) != capabilities_.end();
    }

    bool Worker::isReserved() const
    {
      return reserved_;
    }

    std::set<job_id_t> Worker::getJobListAndCleanQueues()
    {
      std::set<job_id_t> listAssignedJobs;

      listAssignedJobs.insert (pending_.begin(), pending_.end());
      listAssignedJobs.insert (submitted_.begin(), submitted_.end());
      listAssignedJobs.insert (acknowledged_.begin(), acknowledged_.end());
      pending_.clear();
      submitted_.clear();
      acknowledged_.clear();

      return listAssignedJobs;
    }

    double Worker::cost_assigned_jobs
      (std::function<double (job_id_t job_id)> cost_reservation) const
    {
      return ( std::accumulate ( pending_.begin()
                               , pending_.end()
                               , 0.0
                               , [&cost_reservation] (double total_cost, job_id_t job_id)
                                 {
                                   return total_cost + cost_reservation (job_id);
                                 }
                               )
             + std::accumulate ( submitted_.begin()
                               , submitted_.end()
                               , 0.0
                               , [&cost_reservation] (double total_cost, job_id_t job_id)
                                 {
                                   return total_cost + cost_reservation (job_id);
                                 }
                               )
             + std::accumulate ( acknowledged_.begin()
                               , acknowledged_.end()
                               , 0.0
                               , [&cost_reservation] (double total_cost, job_id_t job_id)
                                 {
                                   return total_cost + cost_reservation (job_id);
                                 }
                               )
             );
    }

    bool Worker::remove_job_if_pending (const job_id_t& job_id)
    {
      return pending_.erase (job_id);
    }

    bool Worker::backlog_full() const
    {
      return backlog_full_;
    }

    void Worker::set_backlog_full (bool backlog_full)
    {
      backlog_full_ = backlog_full;
    }

  }
}
