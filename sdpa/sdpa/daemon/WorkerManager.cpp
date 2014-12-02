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

    Worker::ptr_t WorkerManager::findWorker (const worker_id_t& worker_id)
    {
      boost::mutex::scoped_lock const _ (mtx_);
      worker_map_t::iterator it = worker_map_.find(worker_id);
      if( it != worker_map_.end() )
        return it->second;
      else
        throw WorkerNotFoundException();
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

    bool WorkerManager::addWorker ( const worker_id_t& workerId
                                  , boost::optional<unsigned int> capacity
                                  , const capabilities_set_t& cpbSet
                                  , const bool children_allowed
                                  , const std::string& hostname
                                  )
    {
      boost::mutex::scoped_lock const _ (mtx_);

      if (worker_map_.count (workerId) != 0)
      {
        return false;
      }

      Worker::ptr_t pWorker( new Worker( workerId, capacity, cpbSet,  children_allowed, hostname) );
      worker_map_.insert(worker_map_t::value_type(pWorker->name(), pWorker));

      return true;
    }


    void WorkerManager::deleteWorker (const worker_id_t& workerId)
    {
      boost::mutex::scoped_lock const _ (mtx_);
      worker_map_t::iterator w (worker_map_.find (workerId));

      if (w == worker_map_.end())
        throw WorkerNotFoundException();

      worker_map_.erase (w);
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

    boost::optional<std::size_t> WorkerManager::matchRequirements
      ( const Worker::ptr_t& pWorker
      , const job_requirements_t& job_req_set
      ) const
    {
      std::size_t matchingDeg (0);
      if (job_req_set.numWorkers()>1 && pWorker->children_allowed())
      {
        return boost::none;
      }
      for (we::type::requirement_t req : job_req_set.getReqList())
      {
        if (pWorker->hasCapability (req.value()))
        {
          ++matchingDeg;
        }
        else if (req.is_mandatory())
        {
          return boost::none;
        }
      }

      return matchingDeg;
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

        const boost::optional<std::size_t>
          matchingDeg (matchRequirements (it->second, job_reqs));

        if (matchingDeg)
        {
          mmap_match_deg_worker_id.emplace ( *matchingDeg
                                           , worker_id_host_info_t (worker_id, it->second->hostname())
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

  }
}
