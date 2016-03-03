#include <sdpa/daemon/WorkerManager.hpp>
#include <sdpa/types.hpp>

#include <fhg/assert.hpp>

#include <boost/range/algorithm/count_if.hpp>
#include <boost/range/algorithm_ext/push_back.hpp>

namespace sdpa
{
  namespace daemon
  {
    namespace
    {
      typedef std::tuple<double, double, unsigned long, double, worker_id_t> cost_deg_wid_t;
      typedef std::priority_queue < cost_deg_wid_t
                                  , std::vector<cost_deg_wid_t>
                                  > base_priority_queue_t;

      class bounded_priority_queue_t : private base_priority_queue_t
      {
      public:
        explicit bounded_priority_queue_t (std::size_t capacity)
          : capacity_ (capacity)
        {}

        template<typename... Args>
          void emplace (Args&&... args)
        {
          if (size() < capacity_)
          {
            base_priority_queue_t::emplace (std::forward<Args> (args)...);
            return;
          }

          cost_deg_wid_t const next_tuple (std::forward<Args> (args)...);

          if (comp (next_tuple, top()))
          {
            pop();
            base_priority_queue_t::emplace (std::move (next_tuple));
          }
        }

        std::set<worker_id_t> assigned_workers() const
        {
          std::set<worker_id_t> assigned_workers;

          std::transform ( c.begin()
                         , c.end()
                         , std::inserter (assigned_workers, assigned_workers.begin())
                         , [] (const cost_deg_wid_t& cost_deg_wid) -> worker_id_t
                           {
                             return  std::get<4> (cost_deg_wid);
                           }
                         );

          return assigned_workers;
        }

      private:
        size_t capacity_;
      };
    }

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

      worker_equiv_classes_[result.first->second.capability_names_].add_worker_entry (result.first);
    }


    void WorkerManager::deleteWorker (const worker_id_t& workerId)
    {
      boost::mutex::scoped_lock const _ (mtx_);

      auto const worker (worker_map_.find (workerId));
      fhg_assert (worker != worker_map_.end(), "Worker not found when deletion was requested!");

      auto const equivalence_class
        (worker_equiv_classes_.find (worker->second.capability_names_));

      equivalence_class->second.remove_worker_entry (worker);
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
                                                                   , worker.second._last_time_idle
                                                                   )
                                           );
        }
      }

      return mmap_match_deg_worker_id;
    }

    std::set<worker_id_t> WorkerManager::find_job_assignment_minimizing_total_cost
      ( const mmap_match_deg_worker_id_t& mmap_matching_workers
      , const job_requirements_t& requirements
      , const std::function<double (job_id_t const&)> cost_reservation
      ) const
    {
      const size_t n_req_workers (requirements.numWorkers());

      if (mmap_matching_workers.size() < n_req_workers)
        return {};

      bounded_priority_queue_t bpq (n_req_workers);

      for ( std::pair<double const, worker_id_host_info_t> const& it
          : mmap_matching_workers
          )
      {
        const worker_id_host_info_t& worker_info = it.second;
        double const cost_preassigned_jobs
          (worker_map_.at (worker_info.worker_id()).cost_assigned_jobs
             (cost_reservation)
          );

        double const total_cost
          ( requirements.transfer_cost() (worker_info.worker_host())
          + requirements.computational_cost()
          + cost_preassigned_jobs
          );

        bpq.emplace ( total_cost
                    , -1.0*it.first
                    , worker_info.shared_memory_size()
                    , worker_info.last_time_idle()
                    , worker_info.worker_id()
                    );
      }

      return bpq.assigned_workers();
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
          worker->second.delete_pending_job (job_id);
          equivalence_class.dec_pending_jobs (1);
        }
        else
        {
          worker->second.delete_submitted_job (job_id);
          equivalence_class.dec_running_jobs (1);
        }
      }
    }

    const capabilities_set_t& WorkerManager::worker_capabilities (const worker_id_t& worker) const
    {
      boost::mutex::scoped_lock const _(mtx_);
      return worker_map_.at (worker)._capabilities;
    }

    void WorkerManager::change_equivalence_class ( worker_map_t::const_iterator worker
                                                 , std::set<std::string> const& old_cpbs
                                                 )
    {
      {
        auto equivalence_class
          (worker_equiv_classes_.find (old_cpbs));
        fhg_assert (equivalence_class != worker_equiv_classes_.end());

        equivalence_class->second.remove_worker_entry (worker);
        if (equivalence_class->second.n_workers() == 0)
        {
          worker_equiv_classes_.erase (equivalence_class);
        }
      }

      {
        auto& equivalence_class
          (worker_equiv_classes_[worker->second.capability_names_]);
        equivalence_class.add_worker_entry (worker);
      }
    }

    bool WorkerManager::add_worker_capabilities ( const worker_id_t& worker_id
                                                , const capabilities_set_t& cpb_set
                                                )
    {
      boost::mutex::scoped_lock const _(mtx_);

      worker_map_t::iterator worker (worker_map_.find (worker_id));
      const std::set<std::string> old_cpbs (worker->second.capability_names_);

      if (worker->second.addCapabilities (cpb_set))
      {
        change_equivalence_class (worker, old_cpbs);
        return true;
      }

      return false;
    }

    bool WorkerManager::remove_worker_capabilities ( const worker_id_t& worker_id
                                                   , const capabilities_set_t& cpb_set
                                                   )
    {
      boost::mutex::scoped_lock const _(mtx_);

      worker_map_t::iterator worker (worker_map_.find (worker_id));
      const std::set<std::string> old_cpbs (worker->second.capability_names_);

      if (worker->second.removeCapabilities (cpb_set))
      {
        change_equivalence_class (worker, old_cpbs);
        return true;
      }

      return false;
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
      fhg_assert ( _n_pending_jobs >= k
                 , "The number of pending jobs of a group of workers cannot be a negative number"
                 );

      _n_pending_jobs -= k;
    }

    void WorkerManager::WorkerEquivalenceClass::inc_running_jobs (unsigned int k)
    {
      _n_running_jobs += k;
    }

    void WorkerManager::WorkerEquivalenceClass::dec_running_jobs (unsigned int k)
    {
      fhg_assert ( _n_running_jobs >= k
                 , "The number of running jobs of a group of workers cannot be a negative number"
                 );

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

    void WorkerManager::WorkerEquivalenceClass::add_worker_entry (worker_map_t::const_iterator worker)
    {
      _worker_ids.insert (worker->first);
      inc_pending_jobs (worker->second.pending_.size());
      inc_running_jobs ( worker->second.submitted_.size()
                       + worker->second.acknowledged_.size()
                       );
    }

    void WorkerManager::WorkerEquivalenceClass::remove_worker_entry (worker_map_t::const_iterator worker)
    {
      _worker_ids.erase (worker->first);
      dec_pending_jobs (worker->second.pending_.size());
      dec_running_jobs ( worker->second.submitted_.size()
                       + worker->second.acknowledged_.size()
                       );
    }
  }
}
