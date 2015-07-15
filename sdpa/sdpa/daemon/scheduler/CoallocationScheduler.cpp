// tiberiu.rotaru@itwm.fraunhofer.de

#include <sdpa/daemon/scheduler/CoallocationScheduler.hpp>

#include <sdpa/daemon/GenericDaemon.hpp>

#include <climits>
#include <chrono>
#include <functional>
#include <queue>

namespace sdpa
{
  namespace daemon
  {
    CoallocationScheduler::CoallocationScheduler
        (std::function<job_requirements_t (const sdpa::job_id_t&)> job_requirements)
      : _job_requirements (job_requirements)
      , _worker_manager()
    {}

    const WorkerManager& CoallocationScheduler::worker_manager() const
    {
      return _worker_manager;
    }
    WorkerManager& CoallocationScheduler::worker_manager()
    {
      return _worker_manager;
    }

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

      struct min_cost_max_deg_comp
      {
        bool operator() (const cost_deg_wid_t& lhs, const cost_deg_wid_t& rhs) const
        {
          return std::tie (std::get<0> (lhs), std::get<1> (lhs), std::get<2> (lhs), std::get<3> (lhs))
               < std::tie (std::get<0> (rhs), std::get<1> (rhs), std::get<2> (rhs), std::get<3> (rhs));
        }
      };

      typedef std::priority_queue < cost_deg_wid_t
                                  , std::vector<cost_deg_wid_t>
                                  , min_cost_max_deg_comp
                                  > base_priority_queue_t;

      class bounded_priority_queue_t : private base_priority_queue_t
      {
      public:
        explicit bounded_priority_queue_t (std::size_t capacity)
          : capacity_ (capacity)
        {}

      void push (cost_deg_wid_t next_tuple)
      {
        if (size() < capacity_)
        {
          base_priority_queue_t::push (next_tuple);
          return;
        }

        if (comp (next_tuple, top()))
        {
          pop();
          base_priority_queue_t::push (next_tuple);
        }
      }

      using base_priority_queue_t::pop;
      using base_priority_queue_t::top;
      using base_priority_queue_t::size;

      container_type::const_iterator begin() const {return c.begin();}
      container_type::const_iterator end() const {return c.end();}

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

       std::set<worker_id_t> assigned_workers;

       bounded_priority_queue_t bpq (n_req_workers);

       for ( mmap_match_deg_worker_id_t::const_iterator it = mmap_matching_workers.begin()
           ; it != mmap_matching_workers.end()
           ; ++it
           )
       {
         const worker_id_host_info_t& worker_info = it->second;

         double cost_preassigned_jobs = worker_manager().cost_assigned_jobs
                                          ( worker_info.worker_id()
                                          , [this](const job_id_t& job_id) -> double
                                            {
                                              return allocation_table_.at (job_id)->cost();
                                            }
                                          );

         double total_cost = transfer_cost (worker_info.worker_host())
                           + computational_cost
                           + cost_preassigned_jobs;

         bpq.push (std::make_tuple ( total_cost
                                   , -1.0*it->first
                                   , worker_info.shared_memory_size()
                                   , worker_info.last_time_served()
                                   , worker_info.worker_id()
                                   )
                  );
       }

       std::transform ( bpq.begin()
                      , bpq.end()
                      , std::inserter (assigned_workers, assigned_workers.begin())
                      , [] (const cost_deg_wid_t& cost_deg_wid) -> worker_id_t
                        {
                          return  std::get<4> (cost_deg_wid);
                        }
                      );

       return assigned_workers;
     }

    double CoallocationScheduler::compute_reservation_cost
      ( const job_id_t& job_id
      , const std::set<worker_id_t>& workers
      , const std::function<std::string (const sdpa::worker_id_t& wid)> host
      , const double computational_cost
      ) const
    {
      const job_requirements_t& requirements (_job_requirements (job_id));

      return std::accumulate ( workers.begin()
                             , workers.end()
                             , 0.0
                             , [&requirements,host] (const double total, const sdpa::worker_id_t wid)
                               {return total + requirements.transfer_cost() (host (wid));}
                             ) + computational_cost;
    }

    CoallocationScheduler::assignment_t CoallocationScheduler::assignJobsToWorkers()
    {
      boost::mutex::scoped_lock const _ (mtx_alloc_table_);
      if (worker_manager().all_workers_busy_and_have_pending_jobs())
      {
        return {};
      }

      std::list<job_id_t> jobs_to_schedule (_jobs_to_schedule.get_and_clear());

      std::list<sdpa::job_id_t> nonmatching_jobs_queue;

      while (!jobs_to_schedule.empty())
      {
        sdpa::job_id_t jobId (jobs_to_schedule.front());
        jobs_to_schedule.pop_front();

        const job_requirements_t& requirements (_job_requirements (jobId));
        const std::set<worker_id_t> matching_workers
          (find_job_assignment_minimizing_total_cost
             ( worker_manager().getMatchingDegreesAndWorkers (requirements)
             , requirements.numWorkers()
             , requirements.transfer_cost()
             , requirements.computational_cost()
             )
          );

        if (!matching_workers.empty())
        {
          allocation_table_t::iterator it (allocation_table_.find (jobId));
          if (it != allocation_table_.end())
          {
            throw std::runtime_error ("already have reservation for job " + jobId);
          }

          try
          {
            for (worker_id_t const& worker : matching_workers)
            {
              worker_manager().assign_job_to_worker (jobId, worker);
            }

            Reservation* pReservation
              (new Reservation ( matching_workers
                               , compute_reservation_cost ( jobId
                                                          , matching_workers
                                                          , std::bind (&WorkerManager::host, &_worker_manager, std::placeholders::_1)
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
              worker_manager().delete_job_from_worker (jobId, wid);
            }

            jobs_to_schedule.push_front (jobId);
          }
        }
        else
        {
          nonmatching_jobs_queue.push_back (jobId);
        }
      }

      for (job_id_t id : jobs_to_schedule)
      {
        _jobs_to_schedule.push (id);
      }

      for (const sdpa::job_id_t& id : nonmatching_jobs_queue)
      {
        _jobs_to_schedule.push (id);
      }

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
      job_id_list_t matching_jobs;

      boost::mutex::scoped_lock const _ (mtx_alloc_table_);
      for (const job_id_t& job_id : allocation_table_ | boost::adaptors::map_keys)
      {
        const job_requirements_t& requirements (_job_requirements (job_id));
        if (worker_manager().matchRequirements (worker, requirements))
        {
          matching_jobs.push_back (job_id);
        }
      }

      std::set<job_id_t> removed_matching_pending_jobs
        (worker_manager().remove_all_matching_pending_jobs (matching_jobs));

      for (const job_id_t& job_id : removed_matching_pending_jobs)
      {
        allocation_table_.erase (job_id);
        _list_pending_jobs.erase (job_id);
        _jobs_to_schedule.push (job_id);
      }
    }

    bool CoallocationScheduler::cancelNotTerminatedWorkerJobs ( std::function<void (worker_id_t const&)> func
                                                              , const sdpa::job_id_t& job_id)
    {
      boost::mutex::scoped_lock const _ (mtx_alloc_table_);
      sdpa::worker_id_list_t list_not_terminated_workers;

      const allocation_table_t::const_iterator it
        (allocation_table_.find (job_id));

      if (it != allocation_table_.end())
      {
        Reservation* ptr_reservation(it->second);
        list_not_terminated_workers = ptr_reservation->getListNotTerminatedWorkers();
      }

      for (worker_id_t worker_id : list_not_terminated_workers)
      {
        func (worker_id);
      }

      return !list_not_terminated_workers.empty();
    }

    std::set<job_id_t> CoallocationScheduler::start_pending_jobs
      (std::function<void (const sdpa::worker_id_list_t&, const job_id_t&)> serve_job)
    {
      std::set<job_id_t> jobs_started;
      boost::mutex::scoped_lock const _ (mtx_alloc_table_);
      std::list<job_id_t> pending_jobs (_list_pending_jobs.get_and_clear());
      for (const job_id_t& job_id: pending_jobs)
      {
        std::set<worker_id_t> workers (allocation_table_.at (job_id)->workers());
        if (worker_manager().can_start_job (workers))
        {
          for (worker_id_t const& worker : workers)
          {
            worker_manager().submit_job_to_worker (job_id, worker);
          }

          serve_job ({workers.begin(), workers.end()}, job_id);
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
        Reservation* ptr_reservation(it->second);
        for (std::string worker : ptr_reservation->workers())
        {
          try {
            worker_manager().delete_job_from_worker (job_id, worker);
          }
          catch (std::out_of_range const &)
          {
            // the worker might be gone in between
          }
        }

        delete ptr_reservation;
        _list_pending_jobs.erase (it->first);
        allocation_table_.erase (it);
      }
    }

    void CoallocationScheduler::workerFinished
      (const worker_id_t& wid, const job_id_t& jid)
    {
      boost::mutex::scoped_lock const _ (mtx_alloc_table_);
      allocation_table_t::iterator it = allocation_table_.find (jid);
      if (it != allocation_table_.end())
      {
        it->second->storeWorkerResult (wid, Reservation::FINISHED);
      }
      else
      {
        throw std::runtime_error
          ("workerFinished: job missing in allocation table");
      }
    }

    void CoallocationScheduler::workerFailed
      (const worker_id_t& wid, const job_id_t& jid)
    {
      boost::mutex::scoped_lock const _ (mtx_alloc_table_);
      allocation_table_t::iterator it = allocation_table_.find (jid);
      if (it != allocation_table_.end())
      {
        it->second->storeWorkerResult (wid, Reservation::FAILED);
      }
      else
      {
        throw std::runtime_error
          ("workerFailed: job missing in allocation table");
      }
    }

    void CoallocationScheduler::workerCanceled
      (const worker_id_t& wid, const job_id_t& jid)
    {
      boost::mutex::scoped_lock const _ (mtx_alloc_table_);
      allocation_table_t::iterator it = allocation_table_.find (jid);
      if (it != allocation_table_.end())
      {
        it->second->storeWorkerResult (wid, Reservation::CANCELED);
      }
      else
      {
        throw std::runtime_error
          ("workerCanceled: job missing in allocation table");
      }
    }

    bool CoallocationScheduler::allPartialResultsCollected (const job_id_t& jid)
    {
      boost::mutex::scoped_lock const _ (mtx_alloc_table_);
      allocation_table_t::iterator it = allocation_table_.find (jid);
      if (it != allocation_table_.end())
      {
        return it->second->allWorkersTerminated();
      }
      else
      {
        throw std::runtime_error
          ("allPartialResultsCollected: job missing in allocation table");
      }
    }

    bool CoallocationScheduler::groupFinished (const sdpa::job_id_t& jid)
    {
      boost::mutex::scoped_lock const _ (mtx_alloc_table_);
      allocation_table_t::iterator it = allocation_table_.find (jid);
      if (it != allocation_table_.end())
      {
        return it->second->allGroupTasksFinishedSuccessfully();
      }
      else
      {
        throw std::runtime_error
          ("groupFinished: job missing in allocation table");
      }
    }

    void CoallocationScheduler::locked_job_id_list::push (job_id_t item)
    {
      boost::mutex::scoped_lock const _ (mtx_);
      container_.push_back (item);
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
