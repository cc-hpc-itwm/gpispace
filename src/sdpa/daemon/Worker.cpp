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
                   , boost::optional<intertwine::vmem::cache_id_t> cache_id
                   , boost::optional<intertwine::vmem::rank_t> vmem_rank_
                   , const bool children_allowed
                   )
      : _capabilities (capabilities)
      , capability_names_()
      , vmem_cache_id (cache_id)
      , vmem_rank (vmem_rank_)
      , _children_allowed (children_allowed)
      , _last_time_idle (fhg::util::now())
      , reserved_ (false)
      , backlog_full_ (false)
    {
      for (capability_t const& capability : capabilities)
      {
        capability_names_.emplace (capability.name());
      }
    }

    bool Worker::has_pending_jobs() const
    {
      return !pending_.empty();
    }

    bool Worker::has_running_jobs() const
    {
      return !submitted_.empty() || !acknowledged_.empty();
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

    void Worker::delete_submitted_job(const job_id_t &job_id)
    {
      submitted_.erase (job_id);
      acknowledged_.erase (job_id);
      _last_time_idle = fhg::util::now();
      if (!_children_allowed)
      {
        reserved_ = false;
      }
    }

    void Worker::delete_pending_job (const job_id_t& job_id)
    {
      if (0 == pending_.erase (job_id))
      {
        throw std::runtime_error ( "Could not remove the pending job "
                                 + job_id
                                 );
      }
    }

    bool Worker::addCapabilities( const capabilities_set_t& recvCpbSet )
    {

      bool bModified = false;
      for (Capability const& capability : recvCpbSet)
      {
	capabilities_set_t::iterator const itwcpb (_capabilities.find (capability));
	if (itwcpb == _capabilities.end())
	{
	  _capabilities.insert (capability);
          capability_names_.emplace (capability.name());
	  bModified = true;
	}
	else if (itwcpb->depth() > capability.depth())
	{
	  _capabilities.erase (itwcpb);
	  _capabilities.insert (capability);
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
        removed += _capabilities.erase (capability);
        capability_names_.erase (capability.name());
      }

      return removed != 0;
    }

    bool Worker::hasCapability(const std::string& cpbName) const
    {
      return capability_names_.contains (cpbName);
    }

    bool Worker::isReserved() const
    {
      return reserved_;
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

    bool Worker::backlog_full() const
    {
      return backlog_full_;
    }

    void Worker::set_backlog_full (bool backlog_full)
    {
      backlog_full_ = backlog_full;
    }

    bool Worker::stealing_allowed() const
    {
      return ( (has_pending_jobs() && has_running_jobs())
            || (pending_.size() > 1)
             );
    }
  }
}
