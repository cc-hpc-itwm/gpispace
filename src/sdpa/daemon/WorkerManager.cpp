#include <sdpa/daemon/WorkerManager.hpp>
#include <sdpa/types.hpp>

#include <boost/range/algorithm/count_if.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>

namespace sdpa
{
  namespace daemon
  {
    std::string WorkerManager::host_INDICATES_A_RACE (const sdpa::worker_id_t& worker) const
    {
      return worker_map_.at(worker)._hostname;
    }

    bool WorkerManager::hasWorker_INDICATES_A_RACE_TESTING_ONLY(const worker_id_t& worker_id) const
    {
      boost::mutex::scoped_lock const _ (mtx_);
      return worker_map_.find(worker_id) != worker_map_.end();
    }

    std::unordered_set<worker_id_t> WorkerManager::findSubmOrAckWorkers
      (job_id_t const& job_id) const
    {
      boost::mutex::scoped_lock const _ (mtx_);
      std::unordered_set<worker_id_t> submitted_or_ack_workers;

      boost::copy ( worker_map_
                  | boost::adaptors::filtered
                    ( [&job_id] (std::pair<worker_id_t const, Worker> const& w)
                      {
                       return w.second.submitted_.count (job_id)
                         || w.second.acknowledged_.count (job_id);
                      }
                    )
                  | boost::adaptors::map_keys
                  , std::inserter ( submitted_or_ack_workers
                                  , submitted_or_ack_workers.begin()
                                  )
                  );

      return submitted_or_ack_workers;
    }

    void WorkerManager::addWorker ( const worker_id_t& workerId
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
      auto result = worker_map_.emplace ( workerId
                                        , Worker ( cpbSet
                                                 , allocated_shared_memory_size
                                                 , children_allowed
                                                 , hostname
                                                 )
                                        );

      worker_equiv_classes_[result.first->second.capability_names_].add_worker_entry (workerId);
    }


    void WorkerManager::deleteWorker (const worker_id_t& workerId)
    {
      boost::mutex::scoped_lock const _ (mtx_);

      auto const worker (worker_map_.find (workerId));
      assert (worker != worker_map_.end());

      auto const equivalence_class
        (worker_equiv_classes_.find (worker->second.capability_names_));

      equivalence_class->second.remove_worker_entry (workerId);
      if (equivalence_class->second.n_workers() == 0)
      {
        worker_equiv_classes_.erase (equivalence_class);
      }

      worker_map_.erase (worker);
      worker_connections_.left.erase (workerId);
    }

    void WorkerManager::getCapabilities (sdpa::capabilities_set_t& agentCpbSet) const
    {
      boost::mutex::scoped_lock const _ (mtx_);

      for (Worker const& worker : worker_map_ | boost::adaptors::map_values)
      {
        for (sdpa::capability_t const& capability : worker._capabilities)
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
      ( Worker const& worker
      , const job_requirements_t& job_req_set
      ) const
    {
      std::size_t matchingDeg (0);
      if (job_req_set.numWorkers()>1 && worker._children_allowed)
      {
        return boost::none;
      }
      for (we::type::requirement_t const& req : job_req_set.getReqList())
      {
        if (worker.hasCapability (req.value()))
        {
          if (!req.is_mandatory())
          {
            ++matchingDeg;
          }
        }
        else if (req.is_mandatory())
        {
          return boost::none;
        }
      }

      return (matchingDeg + 1.0)/(worker._capabilities.size() + 1.0);
    }

    mmap_match_deg_worker_id_t WorkerManager::getMatchingDegreesAndWorkers
      ( const job_requirements_t& job_reqs
      ) const
    {
      boost::mutex::scoped_lock const lock_worker_map (mtx_);

      if (worker_map_.size() < job_reqs.numWorkers())
      {
        return {};
      }

      mmap_match_deg_worker_id_t mmap_match_deg_worker_id;

      // note: the multimap container maintains the elements
      // sorted according to the specified comparison criteria
      // (here std::greater<int>, i.e. in the descending order of the matching degrees).
      // Searching and insertion operations have logarithmic complexity, as the
      // multimaps are implemented as binary search trees

      for (std::pair<worker_id_t const, Worker> const& worker : worker_map_)
      {
        if ( job_reqs.shared_memory_amount_required()
           > worker.second._allocated_shared_memory_size
           )
          continue;

        if (worker.second.backlog_full())
          continue;

        const boost::optional<double>
          matchingDeg (matchRequirements (worker.second, job_reqs));

        if (matchingDeg)
        {
          mmap_match_deg_worker_id.emplace ( matchingDeg.get()
                                           , worker_id_host_info_t ( worker.first
                                                                   , worker.second._hostname
                                                                   , worker.second._allocated_shared_memory_size
                                                                   , worker.second.lastTimeServed()
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
      return worker_map_.at (worker_id).cost_assigned_jobs (cost_reservation);
    }

    bool WorkerManager::submit_and_serve_if_can_start_job_INDICATES_A_RACE
      ( job_id_t const& job_id
      , std::set<worker_id_t> const& workers
      , std::function<void ( std::set<worker_id_t> const&
                           , const job_id_t&
                           )
                     > const& serve_job
      )
    {
      boost::mutex::scoped_lock const _(mtx_);
      bool const can_start
        ( std::all_of ( std::begin(workers)
                      , std::end(workers)
                      , [this] (const worker_id_t& worker_id)
                        {
                          return worker_map_.count (worker_id)
                            && !worker_map_.at (worker_id).isReserved();
                        }
                      )
        );

      if (can_start)
      {
        for (worker_id_t const& worker_id : workers)
        {
          submit_job_to_worker (job_id, worker_id);
        }

        serve_job (workers, job_id);
      }

      return can_start;
    }

    bool WorkerManager::all_workers_busy_and_have_pending_jobs() const
    {
      boost::mutex::scoped_lock const _(mtx_);
      return std::all_of ( worker_map_.begin()
                         , worker_map_.end()
                         , [](const worker_map_t::value_type& p)
                             {return p.second.isReserved() && p.second.has_pending_jobs();}
                         );
    }

    std::unordered_set<job_id_t>
      WorkerManager::remove_pending_jobs_from_workers_with_similar_capabilities
        (worker_id_t const& w)
    {
      boost::mutex::scoped_lock const _(mtx_);
      std::unordered_set<job_id_t> removed_jobs;

      capabilities_set_t const cpb_set (worker_map_.at (w)._capabilities);

      for (Worker& worker : worker_map_ | boost::adaptors::map_values)
      {
        if (std::all_of ( cpb_set.begin()
                        , cpb_set.end()
                        , [&worker](capability_t const& cpb)
                          {return worker.hasCapability (cpb.name());}
                        )
           )
        {
          removed_jobs.insert ( worker.pending_.begin()
                              , worker.pending_.end()
                              );
          worker.pending_.clear();
        }
      }

      return removed_jobs;
    }

    std::unordered_set<worker_id_t> WorkerManager::workers_to_send_cancel
      (job_id_t const& job_id)
    {
      std::unordered_set<worker_id_t> workers_to_cancel;

      boost::copy ( worker_map_
                  | boost::adaptors::filtered
                    ( [&job_id] (std::pair<worker_id_t const, Worker> const& w)
                      {
                       return w.second.submitted_.count (job_id)
                         || w.second.acknowledged_.count (job_id);
                      }
                    )
                  | boost::adaptors::map_keys
                  , std::inserter (workers_to_cancel, workers_to_cancel.begin())
                  );

      return workers_to_cancel;
    }

    void WorkerManager::assign_job_to_worker (const job_id_t& job_id, const worker_id_t& worker_id)
    {
      boost::mutex::scoped_lock const _(mtx_);
      Worker& worker (worker_map_.at (worker_id));
      worker.assign (job_id);
      worker_equiv_classes_.at
        (worker.capability_names_).inc_pending_jobs (1);
    }

    void WorkerManager::submit_job_to_worker (const job_id_t& job_id, const worker_id_t& worker_id)
    {
      Worker& worker (worker_map_.at (worker_id));
      worker.submit (job_id);
      auto& equivalence_class (worker_equiv_classes_.at (worker.capability_names_));

      equivalence_class.dec_pending_jobs (1);
      equivalence_class.inc_running_jobs (1);
    }

    void WorkerManager::acknowledge_job_sent_to_worker ( const job_id_t& job_id
                                                       , const worker_id_t& worker_id
                                                       )
    {
      boost::mutex::scoped_lock const _(mtx_);
      worker_map_.at (worker_id).acknowledge (job_id);
    }

    void WorkerManager::delete_job_from_worker ( const job_id_t &job_id
                                               , const worker_id_t& worker_id
                                               )
    {
      boost::mutex::scoped_lock const _(mtx_);
      auto worker (worker_map_.find (worker_id));
      if (worker != worker_map_.end())
      {
        auto& equivalence_class
          (worker_equiv_classes_.at (worker->second.capability_names_));

        if (worker->second.pending_.count (job_id))
        {
          equivalence_class.dec_pending_jobs (1);
        }
        else
        {
          equivalence_class.dec_running_jobs (1);
        }

        worker->second.deleteJob (job_id);
      }
    }

    const capabilities_set_t& WorkerManager::worker_capabilities (const worker_id_t& worker) const
    {
      boost::mutex::scoped_lock const _(mtx_);
      return worker_map_.at (worker)._capabilities;
    }

    const std::set<job_id_t> WorkerManager::get_worker_jobs_and_clean_queues (const worker_id_t& worker)
    {
      boost::mutex::scoped_lock const _(mtx_);
      return worker_map_.at (worker).getJobListAndCleanQueues();
    }

    void WorkerManager::change_equivalence_class ( worker_id_t const& worker_id
                                                 , Worker const& worker
                                                 , std::set<std::string> const& old_cpbs
                                                 )
    {
      const unsigned int n_pending_jobs (worker.pending_.size());
      const unsigned int n_running_jobs ( worker.submitted_.size()
                                        + worker.acknowledged_.size()
                                        );
      {
        auto& equivalence_class
          (worker_equiv_classes_.at (old_cpbs));
        equivalence_class.remove_worker_entry (worker_id);
        equivalence_class.dec_pending_jobs (n_pending_jobs);
        equivalence_class.dec_running_jobs (n_running_jobs);
      }

      {
        auto& equivalence_class
          (worker_equiv_classes_[worker.capability_names_]);
        equivalence_class.add_worker_entry (worker_id);
        equivalence_class.inc_pending_jobs (n_pending_jobs);
        equivalence_class.inc_running_jobs (n_running_jobs);
      }
    }

    bool WorkerManager::add_worker_capabilities ( const worker_id_t& worker_id
                                                , const capabilities_set_t& cpb_set
                                                )
    {
      boost::mutex::scoped_lock const _(mtx_);

      Worker& worker (worker_map_.at (worker_id));
      const std::set<std::string> old_cpbs (worker.capability_names_);

      bool rv (worker.addCapabilities (cpb_set));
      if (rv)
      {
        change_equivalence_class (worker_id, worker, old_cpbs);
      }

      return rv;
    }

    bool WorkerManager::remove_worker_capabilities ( const worker_id_t& worker_id
                                                   , const capabilities_set_t& cpb_set
                                                   )
    {
      boost::mutex::scoped_lock const _(mtx_);

      Worker& worker (worker_map_.at (worker_id));
      const std::set<std::string> old_cpbs (worker.capability_names_);

      bool rv (worker.removeCapabilities (cpb_set));
      if (rv)
      {
        change_equivalence_class (worker_id, worker, old_cpbs);
      }

      return rv;
    }

    void WorkerManager::set_worker_backlog_full ( const worker_id_t& worker_id
                                                , bool val
                                                )
    {
      boost::mutex::scoped_lock const _ (mtx_);
      return worker_map_.at (worker_id).set_backlog_full (val);
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

    WorkerManager::WorkerEquivalenceClass::WorkerEquivalenceClass()
      : _n_pending_jobs (0)
      , _n_running_jobs (0)
      , _n_idle_workers (0)
    {}

    void WorkerManager::WorkerEquivalenceClass::inc_pending_jobs (unsigned int k)
    {
      _n_pending_jobs += k;
    }

    void WorkerManager::WorkerEquivalenceClass::dec_pending_jobs (unsigned int k)
    {
      if (_n_pending_jobs < k)
        throw std::runtime_error
          ("The number of pending jobs of a group of workers cannot be a negative number");

      _n_pending_jobs -= k;
    }

    void WorkerManager::WorkerEquivalenceClass::inc_running_jobs (unsigned int k)
    {
      _n_running_jobs += k;
    }

    void WorkerManager::WorkerEquivalenceClass::dec_running_jobs (unsigned int k)
    {
      if (_n_running_jobs < k)
        throw std::runtime_error
          ("The number of running jobs of a group of workers cannot be a negative number");

      _n_running_jobs -= k;
    }

    unsigned int WorkerManager::WorkerEquivalenceClass::n_pending_jobs() const
    {
      return _n_pending_jobs;
    }

    unsigned int WorkerManager::WorkerEquivalenceClass::n_running_jobs() const
    {
      return _n_running_jobs;
    }

    unsigned int WorkerManager::WorkerEquivalenceClass::n_idle_workers() const
    {
      return _n_idle_workers;
    }

    unsigned int WorkerManager::WorkerEquivalenceClass::n_workers() const
    {
      return _worker_ids.size();
    }

    void WorkerManager::WorkerEquivalenceClass::add_worker_entry (worker_id_t const& w)
    {
      _worker_ids.insert (w);
    }

    void WorkerManager::WorkerEquivalenceClass::remove_worker_entry (worker_id_t const& w)
    {
      _worker_ids.erase (w);
    }
  }
}
