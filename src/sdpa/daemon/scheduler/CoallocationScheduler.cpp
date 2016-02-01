#include <sdpa/daemon/scheduler/CoallocationScheduler.hpp>

#include <sdpa/daemon/GenericDaemon.hpp>

#include <boost/range/algorithm.hpp>

#include <climits>
#include <functional>
#include <queue>

namespace sdpa
{
  namespace daemon
  {
    CoallocationScheduler::CoallocationScheduler
      ( std::function<job_requirements_t (const sdpa::job_id_t&)> job_requirements
      , WorkerManager& worker_manager
      )
      : _job_requirements (job_requirements)
      , _worker_manager (worker_manager)
    {}

    bool CoallocationScheduler::delete_job (sdpa::job_id_t const& job)
    {
      return _jobs_to_schedule.erase (job);
    }

    void CoallocationScheduler::enqueueJob (const sdpa::job_id_t& jobId)
    {
      _jobs_to_schedule.push (jobId);
    }

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

    std::set<worker_id_t> CoallocationScheduler::find_job_assignment_minimizing_total_cost
       ( const mmap_match_deg_worker_id_t& mmap_matching_workers
       , const size_t n_req_workers
       , const std::function<double (std::string const&)> transfer_cost
       , const double computational_cost
       )
     {
       if (mmap_matching_workers.size() < n_req_workers)
         return {};

       bounded_priority_queue_t bpq (n_req_workers);

       for ( std::pair<double const, worker_id_host_info_t> const& it
           : mmap_matching_workers
           )
       {
         const worker_id_host_info_t& worker_info = it.second;

         double const cost_preassigned_jobs
           ( _worker_manager.cost_assigned_jobs
             ( worker_info.worker_id()
             , [this] (const job_id_t& job_id) -> double
               {
                 return allocation_table_.at (job_id)->cost();
               }
             )
           );

         double const total_cost
           ( transfer_cost (worker_info.worker_host())
           + computational_cost
           + cost_preassigned_jobs
           );

         bpq.emplace ( total_cost
                     , -1.0*it.first
                     , worker_info.shared_memory_size()
                     , worker_info.last_time_served()
                     , worker_info.worker_id()
                     );
       }

       return bpq.assigned_workers();
     }

    double CoallocationScheduler::compute_reservation_cost
      ( const job_id_t& job_id
      , const std::set<worker_id_t>& workers
      , const double computational_cost
      ) const
    {
      return std::accumulate
        ( workers.begin()
        , workers.end()
        , 0.0
        , [this, &job_id] (const double total, const sdpa::worker_id_t wid)
          {
            return total
              + _job_requirements (job_id).transfer_cost()
                  (_worker_manager.host_INDICATES_A_RACE (wid));
          }
        ) + computational_cost;
    }

    CoallocationScheduler::assignment_t CoallocationScheduler::assignJobsToWorkers()
    {
      boost::mutex::scoped_lock const _ (mtx_alloc_table_);
      if (_worker_manager.all_workers_busy_and_have_pending_jobs())
      {
        return {};
      }

      std::list<job_id_t> jobs_to_schedule (_jobs_to_schedule.get_and_clear());
      if (jobs_to_schedule.empty())
      {
        _worker_manager.steal_work<Reservation>
          ( [this] (job_id_t const& job)
            {return allocation_table_.at (job);}
          , _job_requirements
          );
      }

      std::list<sdpa::job_id_t> nonmatching_jobs_queue;

      while (!jobs_to_schedule.empty())
      {
        sdpa::job_id_t const jobId (jobs_to_schedule.front());
        jobs_to_schedule.pop_front();

        const job_requirements_t& requirements (_job_requirements (jobId));
        const std::set<worker_id_t> matching_workers
          (find_job_assignment_minimizing_total_cost
             ( _worker_manager.getMatchingDegreesAndWorkers (requirements)
             , requirements.numWorkers()
             , requirements.transfer_cost()
             , requirements.computational_cost()
             )
          );

        if (!matching_workers.empty())
        {
          allocation_table_t::iterator const it (allocation_table_.find (jobId));
          if (it != allocation_table_.end())
          {
            throw std::runtime_error ("already have reservation for job " + jobId);
          }

          try
          {
            for (worker_id_t const& worker : matching_workers)
            {
              _worker_manager.assign_job_to_worker (jobId, worker);
            }

            Reservation* const pReservation
              (new Reservation ( matching_workers
                               , compute_reservation_cost ( jobId
                                                          , matching_workers
                                                          , requirements.computational_cost()
                                                          )
                               )
              );

            allocation_table_.emplace (jobId, pReservation);
            _list_pending_jobs.push (jobId);
          }
          catch (std::out_of_range const&)
          {
            for (const worker_id_t& wid : matching_workers)
            {
              _worker_manager.delete_job_from_worker (jobId, wid);
            }

            jobs_to_schedule.push_front (jobId);
          }
        }
        else
        {
          nonmatching_jobs_queue.push_back (jobId);
        }
      }

      _jobs_to_schedule.push (jobs_to_schedule);
      _jobs_to_schedule.push (nonmatching_jobs_queue);

      assignment_t assignment;
      std::transform ( allocation_table_.begin()
                     , allocation_table_.end()
                     , std::inserter (assignment, assignment.end())
                     , [](allocation_table_t::value_type const &p)
                       {
                         return std::make_pair (p.first, p.second->workers());
                       }
                     );

      return assignment;
    }

    void CoallocationScheduler::reschedule_pending_jobs_matching_worker
      (const worker_id_t& worker)
    {
      boost::mutex::scoped_lock const _ (mtx_alloc_table_);

      const std::unordered_set<job_id_t> removed_jobs
        (_worker_manager.remove_pending_jobs_from_workers_with_similar_capabilities
          (worker)
        );

      for (job_id_t const& job_id : removed_jobs)
      {
        _list_pending_jobs.erase (job_id);
        delete allocation_table_.at (job_id);
        allocation_table_.erase (job_id);
      }

      _jobs_to_schedule.push (removed_jobs);
    }

    bool CoallocationScheduler::cancelNotTerminatedWorkerJobs ( std::function<void (worker_id_t const&)> func
                                                              , const sdpa::job_id_t& job_id)
    {
      boost::mutex::scoped_lock const _ (mtx_alloc_table_);

      const allocation_table_t::const_iterator it
        (allocation_table_.find (job_id));

      if (it != allocation_table_.end())
      {
        return it->second->apply_to_workers_without_result (std::move (func));
      }

      return false;
    }

    std::set<job_id_t> CoallocationScheduler::start_pending_jobs
      (std::function<void (std::set<worker_id_t> const&, const job_id_t&)> serve_job)
    {
      std::set<job_id_t> jobs_started;
      boost::mutex::scoped_lock const _ (mtx_alloc_table_);
      std::list<job_id_t> const pending_jobs (_list_pending_jobs.get_and_clear());
      for (const job_id_t& job_id: pending_jobs)
      {
        std::set<worker_id_t> const& workers (allocation_table_.at (job_id)->workers());
        if (_worker_manager.submit_and_serve_if_can_start_job_INDICATES_A_RACE
             (job_id, workers, serve_job)
           )
        {
          jobs_started.insert (job_id);
        }
        else
        {
          _list_pending_jobs.push (job_id);
        }
      }

      return jobs_started;
    }

    void CoallocationScheduler::releaseReservation (const sdpa::job_id_t& job_id)
    {
      boost::mutex::scoped_lock const _ (mtx_alloc_table_);
      const allocation_table_t::const_iterator it
       (allocation_table_.find (job_id));

      if (it != allocation_table_.end())
      {
        Reservation const* const ptr_reservation(it->second);
        for (std::string const& worker : ptr_reservation->workers())
        {
          _worker_manager.delete_job_from_worker (job_id, worker);
        }

        delete ptr_reservation;
        _list_pending_jobs.erase (it->first);
        allocation_table_.erase (it);
      }
      //! \todo why can we ignore this?
    }

    void CoallocationScheduler::store_result ( worker_id_t const& worker_id
                                             , job_id_t const& job_id
                                             , terminal_state result
                                             )
    {
      boost::mutex::scoped_lock const _ (mtx_alloc_table_);
      auto const it (allocation_table_.find (job_id));
      //! \todo assert only as this probably is a logical error?
      if (it == allocation_table_.end())
      {
        throw std::runtime_error ("store_result: unknown job");
      }

      it->second->store_result (worker_id, result);
    }

    boost::optional<job_result_type>
      CoallocationScheduler::get_aggregated_results_if_all_terminated (job_id_t const& job_id)
    {
      boost::mutex::scoped_lock const _ (mtx_alloc_table_);
      auto const it (allocation_table_.find (job_id));
      //! \todo assert only as this probably is a logical error?
      if (it == allocation_table_.end())
      {
        throw std::runtime_error
          ("get_aggregated_results_if_all_terminated: unknown job");
      }

      return it->second->get_aggregated_results_if_all_terminated();
    }

    void CoallocationScheduler::locked_job_id_list::push (job_id_t item)
    {
      boost::mutex::scoped_lock const _ (mtx_);
      container_.push_back (item);
    }

    template <typename Range>
    void CoallocationScheduler::locked_job_id_list::push (Range range)
    {
      boost::mutex::scoped_lock const _ (mtx_);
      container_.insert (container_.end(), std::begin (range), std::end (range));
    }

    size_t CoallocationScheduler::locked_job_id_list::erase (const job_id_t& item)
    {
      boost::mutex::scoped_lock const _ (mtx_);
      size_t count (0);
      std::list<job_id_t>::iterator iter (container_.begin());
      while (iter != container_.end())
      {
        if (item == *iter)
        {
          iter = container_.erase(iter);
          ++count;
        }
        else
        {
          ++iter;
        }
      }
      return count;
    }

    std::list<job_id_t> CoallocationScheduler::locked_job_id_list::get_and_clear()
    {
      boost::mutex::scoped_lock const _ (mtx_);

      std::list<job_id_t> ret;
      std::swap (ret, container_);
      return ret;
    }
  }
}
