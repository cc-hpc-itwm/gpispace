/*
 * =====================================================================================
 *
 *       Filename:  WorkerManager.cpp
 *
 *    Description:  Worker manager implementation
 *
 *        Version:  1.0
 *        Created:
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Dr. Tiberiu Rotaru, tiberiu.rotaru@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#include <sdpa/daemon/WorkerManager.hpp>
#include <algorithm>
#include <sdpa/types.hpp>

#include <algorithm>
#include <limits>

#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/range/algorithm/count_if.hpp>
#include <boost/range/algorithm/copy.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>

#include <unordered_map>

namespace sdpa
{
  namespace daemon
  {
    std::string WorkerManager::host (const sdpa::worker_id_t& worker) const
    {
      return worker_map_.at(worker)->hostname();
    }

    bool WorkerManager::hasWorker(const worker_id_t& worker_id) const
    {
      boost::mutex::scoped_lock const _ (mtx_);
      return worker_map_.find(worker_id) != worker_map_.end();
    }

    const boost::optional<worker_id_t> WorkerManager::findSubmOrAckWorker (const sdpa::job_id_t& job_id) const
    {
      boost::mutex::scoped_lock const _ (mtx_);

      for ( Worker::ptr_t worker
          : worker_map_
          | boost::adaptors::map_values
          | boost::adaptors::filtered
            ([&job_id] (boost::shared_ptr<Worker> w) { return w->has_job (job_id); })
          )
      {
        return worker->name();
      }

      return boost::none;
    }

    void WorkerManager::addWorker ( const worker_id_t& workerId
                                  , boost::optional<unsigned int> capacity
                                  , const capabilities_set_t& cpbSet
                                  , unsigned long allocated_shared_memory_size
                                  , const bool children_allowed
                                  , const std::string& hostname
                                  , const fhg::com::p2p::address_t& address
                                  )
    {
      boost::mutex::scoped_lock const _ (mtx_);

      if (worker_map_.count (workerId) != 0)
      {
        throw std::runtime_error ("worker '" + workerId + "' already exists");
      }
      worker_connections_.left.insert ({workerId, address});
      Worker::ptr_t pWorker ( new Worker ( workerId
                                         , capacity
                                         , cpbSet
                                         , allocated_shared_memory_size
                                         , children_allowed
                                         , hostname
                                         , address
                                         )
                            );
      worker_map_.insert(worker_map_t::value_type(pWorker->name(), pWorker));
    }


    void WorkerManager::deleteWorker (const worker_id_t& workerId)
    {
      boost::mutex::scoped_lock const _ (mtx_);

      worker_map_.erase (workerId);
      worker_connections_.left.erase (workerId);
    }

    std::set<worker_id_t> WorkerManager::getAllNonReservedWorkers() const
    {
      boost::mutex::scoped_lock const _ (mtx_);
      std::set<worker_id_t> free_workers;
      for ( Worker::ptr_t ptr_worker
          : worker_map_
          | boost::adaptors::map_values
          | boost::adaptors::filtered
             ([](const Worker::ptr_t& ptr_worker)
               {return !ptr_worker->isReserved();}
             )
          )
      {
        free_workers.insert (ptr_worker->name());
      }

      return free_workers;
    }

    void WorkerManager::getCapabilities (sdpa::capabilities_set_t& agentCpbSet) const
    {
      boost::mutex::scoped_lock const _ (mtx_);

      for (Worker::ptr_t worker : worker_map_ | boost::adaptors::map_values)
      {
        for (sdpa::capability_t capability : worker->capabilities())
        {
          const sdpa::capabilities_set_t::iterator itag_cpbs
            (agentCpbSet.find (capability));
          if (itag_cpbs == agentCpbSet.end())
          {
            agentCpbSet.insert (capability);
          }
          else if (itag_cpbs->depth() > capability.depth())
          {
            agentCpbSet.erase (itag_cpbs);
            agentCpbSet.insert (capability);
          }
        }
      }
    }

    boost::optional<double> WorkerManager::matchRequirements
      ( const worker_id_t& worker
      , const job_requirements_t& job_req_set
      ) const
    {
      std::size_t matchingDeg (0);
      if (job_req_set.numWorkers()>1 && worker_map_.at (worker)->children_allowed())
      {
        return boost::none;
      }
      for (we::type::requirement_t req : job_req_set.getReqList())
      {
        if (worker_map_.at (worker)->hasCapability (req.value()))
        {
          ++matchingDeg;
        }
        else if (req.is_mandatory())
        {
          return boost::none;
        }
      }

      return matchingDeg/(1.0*(worker_map_.at (worker)->capabilities().size() + 1));
    }

    mmap_match_deg_worker_id_t WorkerManager::getMatchingDegreesAndWorkers
      ( const job_requirements_t& job_reqs
      ) const
    {
      boost::mutex::scoped_lock const lock_worker_map (mtx_);
      std::vector<worker_id_t> workers;
      boost::copy ( worker_map_ | boost::adaptors::map_keys
                  , std::back_inserter (workers)
                  );

      if (workers.size() < job_reqs.numWorkers())
      {
        return {};
      }

      mmap_match_deg_worker_id_t mmap_match_deg_worker_id;

      // note: the multimap container maintains the elements
      // sorted according to the specified comparison criteria
      // (here std::greater<int>, i.e. in the descending order of the matching degrees).
      // Searching and insertion operations have logarithmic complexity, as the
      // multimaps are implemented as binary search trees

      for (const sdpa::worker_id_t& worker_id : workers)
      {
        const worker_map_t::const_iterator it (worker_map_.find (worker_id));
        if (it == worker_map_.end())
          continue;

        if ( job_reqs.shared_memory_amount_required()
           > it->second->allocated_shared_memory_size()
           )
          continue;

        if (it->second->backlog_full())
          continue;

        const boost::optional<double>
          matchingDeg (matchRequirements (it->second->name(), job_reqs));

        if (matchingDeg)
        {
          mmap_match_deg_worker_id.emplace ( matchingDeg.get()
                                           , worker_id_host_info_t ( worker_id
                                                                   , it->second->hostname()
                                                                   , it->second->allocated_shared_memory_size()
                                                                   , it->second->lastTimeServed()
                                                                   )
                                           );
        }
      }

      return mmap_match_deg_worker_id;
    }

    double WorkerManager::cost_assigned_jobs
      ( const worker_id_t worker_id
      , std::function<double (job_id_t job_id)> cost_reservation
      )
    {
      boost::mutex::scoped_lock const _(mtx_);
      return worker_map_.at (worker_id)->cost_assigned_jobs (cost_reservation);
    }

    bool WorkerManager::can_start_job (std::set<worker_id_t> workers) const
    {
      boost::mutex::scoped_lock const _(mtx_);
      return std::all_of ( std::begin(workers)
                         , std::end(workers)
                         , [this] (const worker_id_t& worker_id)
                           {return !worker_map_.at (worker_id)->isReserved();}
                         );
    }

    std::set<job_id_t>  WorkerManager::remove_all_matching_pending_jobs
      (const job_id_list_t& matching_jobs)
    {
      boost::mutex::scoped_lock const _(mtx_);
      std::set<job_id_t> pending_jobs;
      for (Worker::ptr_t ptr_worker : worker_map_ | boost::adaptors::map_values )
      {
        for (const job_id_t& job_id : matching_jobs)
        {
          if (ptr_worker->remove_job_if_pending (job_id))
          {
            pending_jobs.insert (job_id);
          }
        }
      }

      return pending_jobs;
    }

    bool WorkerManager::all_workers_busy_and_have_pending_jobs() const
    {
      boost::mutex::scoped_lock const _(mtx_);
      return std::all_of ( worker_map_.begin()
                         , worker_map_.end()
                         , [](const worker_map_t::value_type& p)
                             {return p.second->isReserved() && p.second->has_pending_jobs();}
                         );
    }

    void WorkerManager::assign_job_to_worker (const job_id_t& job_id, const worker_id_t& worker_id)
    {
      boost::mutex::scoped_lock const _(mtx_);
      worker_map_.at (worker_id)->assign (job_id);
    }

    void WorkerManager::submit_job_to_worker (const job_id_t& job_id, const worker_id_t& worker_id)
    {
      boost::mutex::scoped_lock const _(mtx_);
      worker_map_.at (worker_id)->submit (job_id);
    }

    void WorkerManager::acknowledge_job_sent_to_worker ( const job_id_t& job_id
                                                       , const worker_id_t& worker_id
                                                       )
    {
      boost::mutex::scoped_lock const _(mtx_);
      worker_map_.at (worker_id)->acknowledge (job_id);
    }

    void WorkerManager::delete_job_from_worker ( const job_id_t &job_id
                                               , const worker_id_t& worker_id
                                               )
    {
      boost::mutex::scoped_lock const _(mtx_);
      worker_map_.at (worker_id)->deleteJob (job_id);
    }

    const capabilities_set_t& WorkerManager::worker_capabilities (const worker_id_t& worker) const
    {
      boost::mutex::scoped_lock const _(mtx_);
      return worker_map_.at (worker)->capabilities();
    }

    const std::set<job_id_t> WorkerManager::get_worker_jobs_and_clean_queues (const worker_id_t& worker) const
    {
      boost::mutex::scoped_lock const _(mtx_);
      return worker_map_.at (worker)->getJobListAndCleanQueues();
    }

    bool WorkerManager::add_worker_capabilities ( const worker_id_t& worker_id
                                                , const capabilities_set_t& cpb_set
                                                )
    {
      boost::mutex::scoped_lock const _(mtx_);
      return worker_map_.at (worker_id)->addCapabilities (cpb_set);
    }

    bool WorkerManager::remove_worker_capabilities ( const worker_id_t& worker_id
                                                   , const capabilities_set_t& cpb_set
                                                   )
    {
      boost::mutex::scoped_lock const _(mtx_);
      return worker_map_.at (worker_id)->removeCapabilities (cpb_set);
    }

    void WorkerManager::set_worker_backlog_full ( const worker_id_t& worker_id
                                                , bool val
                                                )
    {
      boost::mutex::scoped_lock const _ (mtx_);
      return worker_map_.at (worker_id)->set_backlog_full (val);
    }

    boost::optional<WorkerManager::worker_connections_t::right_iterator>
      WorkerManager::worker_by_address (fhg::com::p2p::address_t const& address)
    {
      WorkerManager::worker_connections_t::right_iterator it
        (worker_connections_.right.find (address));
      return boost::make_optional (it != worker_connections_.right.end(), it);
    }

    boost::optional<WorkerManager::worker_connections_t::left_iterator>
      WorkerManager::address_by_worker (std::string const& worker)
    {
      WorkerManager::worker_connections_t::left_iterator it
        (worker_connections_.left.find (worker));
      return boost::make_optional (it != worker_connections_.left.end(), it);
    }
  }
}
