#pragma once

#include <sdpa/daemon/Worker.hpp>
#include <sdpa/job_requirements.hpp>
#include <sdpa/events/CancelJobEvent.hpp>

#include <boost/bimap.hpp>
#include <boost/bimap/unordered_set_of.hpp>
#include <boost/noncopyable.hpp>
#include <boost/optional.hpp>
#include <boost/range/algorithm.hpp>
#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/adaptor/map.hpp>

#include <algorithm>
#include <queue>
#include <unordered_map>
#include <unordered_set>

namespace sdpa
{
  namespace daemon
  {
    class WorkerManager : boost::noncopyable
    {
      typedef std::unordered_map<worker_id_t, Worker> worker_map_t;

    private:
      class WorkerEquivalenceClass
      {
      public:
        WorkerEquivalenceClass();
        WorkerEquivalenceClass (const WorkerEquivalenceClass&) = delete;
        WorkerEquivalenceClass (WorkerEquivalenceClass&&) = delete;
        WorkerEquivalenceClass& operator= (const WorkerEquivalenceClass&) = delete;
        WorkerEquivalenceClass& operator= (const WorkerEquivalenceClass&&) = delete;
        ~WorkerEquivalenceClass() = default;

        void inc_pending_jobs (unsigned int);
        void dec_pending_jobs (unsigned int);
        void inc_running_jobs (unsigned int);
        void dec_running_jobs (unsigned int);

        unsigned int n_pending_jobs() const;
        unsigned int n_running_jobs() const;
        unsigned int n_idle_workers() const;
        unsigned int n_workers() const;

        void add_worker_entry (worker_map_t::const_iterator);
        void remove_worker_entry (worker_map_t::const_iterator);

        template <typename Reservation>
          void steal_work
            ( std::function<Reservation* (job_id_t const&)>
            , worker_map_t&
            );

      private:
        unsigned int _n_pending_jobs;
        unsigned int _n_running_jobs;
        unsigned int _n_idle_workers;
        std::unordered_set<worker_id_t> _worker_ids;
      };

    public:
      std::unordered_set<worker_id_t> findSubmOrAckWorkers
        (const sdpa::job_id_t& job_id) const;

      boost::optional<intertwine::vmem::rank_t>
        vmem_rank (worker_id_t const&) const;

      //! throws if workerId was not unique
      void addWorker ( const worker_id_t& workerId
                     , const capabilities_set_t& cpbset
                     , boost::optional<intertwine::vmem::size_t> vmem_cache_size
                     , boost::optional<intertwine::vmem::rank_t> vmem_rank
                     , const bool children_allowed
                     , const fhg::com::p2p::address_t& address
                     );

      void deleteWorker (const worker_id_t& workerId);

      void getCapabilities (sdpa::capabilities_set_t& cpbset) const;

      mmap_match_deg_worker_id_t getMatchingDegreesAndWorkers_TESTING_ONLY (const job_requirements_t&) const;
      std::set<worker_id_t> find_job_assignment_minimizing_total_cost
        ( const mmap_match_deg_worker_id_t&
        , const job_requirements_t&
        , const std::function<double (job_id_t const&)>
        ) const;

      std::set<worker_id_t> find_assignment
        ( const job_requirements_t&
        , const std::function<double (job_id_t const&)>
        ) const;

      template <typename Reservation>
        void steal_work (std::function<Reservation* (job_id_t const&)> reservation);

      bool submit_and_serve_if_can_start_job_INDICATES_A_RACE
        ( job_id_t const&, std::set<worker_id_t> const&
        , std::function<void ( std::set<worker_id_t> const&
                             , const job_id_t&
                             )> const& serve_job
        );

      bool all_workers_busy_and_have_pending_jobs() const;

      template <typename T>
        std::unordered_set<sdpa::job_id_t> delete_or_cancel_worker_jobs
          ( worker_id_t const&
          , std::function<Job* (sdpa::job_id_t const&)>
          , std::function<T* (sdpa::job_id_t const&)>
          , std::function<void (sdpa::worker_id_t const&, job_id_t const&)>
          );

      void assign_job_to_worker (const job_id_t&, const worker_id_t&);
      void acknowledge_job_sent_to_worker (const job_id_t&, const worker_id_t&);
      void delete_job_from_worker (const job_id_t &job_id, const worker_id_t& );
      const capabilities_set_t& worker_capabilities (const worker_id_t&) const;
      bool add_worker_capabilities (const worker_id_t&, const capabilities_set_t&);
      bool remove_worker_capabilities (const worker_id_t&, const capabilities_set_t&);
      void set_worker_backlog_full (const worker_id_t&, bool);

      using worker_connections_t
        = boost::bimap < boost::bimaps::unordered_set_of<std::string>
                       , boost::bimaps::unordered_set_of<fhg::com::p2p::address_t>
                       >;

      boost::optional<WorkerManager::worker_connections_t::right_iterator>
        worker_by_address (fhg::com::p2p::address_t const&);

      boost::optional<WorkerManager::worker_connections_t::left_iterator>
        address_by_worker (std::string const&);

      bool hasWorker_INDICATES_A_RACE_TESTING_ONLY (const worker_id_t& worker_id) const;

      std::unordered_set<worker_id_t> workers_to_send_cancel (job_id_t const& job_id);

      cache_info_t const& cache_info
        ( intertwine::vmem::rank_t const&
        , intertwine::vmem::cache_id_t const&
        ) const;
    private:
      void submit_job_to_worker (const job_id_t&, const worker_id_t&);
      void change_equivalence_class (worker_map_t::const_iterator, std::set<std::string> const&);

      boost::optional<double> matchRequirements
        ( Worker const&
        , const job_requirements_t& job_req_set
        ) const;

      worker_map_t  worker_map_;
      worker_connections_t worker_connections_;
      std::map<std::set<std::string>, WorkerEquivalenceClass> worker_equiv_classes_;
      std::unordered_map<cache_info_key_t, cache_info_t, cache_info_key_hasher> _caches;

      mutable std::mutex mtx_;
    };

    template <typename Reservation>
      void WorkerManager::steal_work
        (std::function<Reservation* (job_id_t const&)> reservation)
    {
      std::lock_guard<std::mutex> const _(mtx_);
      for (WorkerEquivalenceClass& weqc : worker_equiv_classes_
                                        | boost::adaptors::map_values
          )
      {
        weqc.steal_work (reservation, worker_map_);
      }
    }

    template <typename Reservation>
      void WorkerManager::WorkerEquivalenceClass::steal_work
        ( std::function<Reservation* (job_id_t const&)> reservation
        , worker_map_t& worker_map
        )
    {
      if (n_running_jobs() == n_workers())
      {
        return;
      }

      if (n_pending_jobs() == 0)
      {
        return;
      }

      std::function<double (job_id_t const& job_id)> const cost
        { [&reservation] (job_id_t const& job_id)
          {
            return reservation (job_id)->cost();
          }
        };

      using worker_ptr = worker_map_t::iterator;

      std::function<bool (worker_ptr const&, worker_ptr const&)> const
        comp { [&cost] (worker_ptr const& lhs, worker_ptr const& rhs)
               {
                 return lhs->second.cost_assigned_jobs (cost)
                   < rhs->second.cost_assigned_jobs (cost);
               }
             };

      std::priority_queue < worker_ptr
                          , std::vector<worker_ptr>
                          , decltype (comp)
                          > to_steal_from (comp);

      std::function<bool (worker_ptr const&, worker_ptr const&)> const
        comp_idles { [] (worker_ptr const& lhs, worker_ptr const& rhs)
                     {
                       return lhs->second._last_time_idle
                         > rhs->second._last_time_idle;
                     }
                   };

      std::priority_queue < worker_ptr
                          , std::vector<worker_ptr>
                          , decltype (comp_idles)
                          > idles (comp_idles);

      for (worker_id_t const& w : _worker_ids)
      {
        worker_ptr const& worker_ptr (worker_map.find (w));
        fhg_assert (worker_ptr != worker_map.end());
        Worker const& worker (worker_ptr->second);

        if (worker.stealing_allowed())
        {
          to_steal_from.emplace (worker_ptr);
        }
        else if (!worker.has_running_jobs() && !worker.has_pending_jobs())
        {
          idles.emplace (worker_ptr);
        }
      }

      while (!(idles.empty() || to_steal_from.empty()))
      {
        worker_ptr const richest (to_steal_from.top());
        worker_ptr const& thief (idles.top());
        Worker& richest_worker (richest->second);

        auto it_job (std::max_element ( richest_worker.pending_.begin()
                                      , richest_worker.pending_.end()
                                      , [&reservation] ( job_id_t const& r
                                                       , job_id_t const& l
                                                       )
                                        {
                                          return reservation (r)->cost()
                                            < reservation (l)->cost();
                                        }
                                      )
                    );

        fhg_assert (it_job != richest_worker.pending_.end());

        reservation (*it_job)->replace_worker (richest->first, thief->first);

        thief->second.assign (*it_job);
        richest_worker.pending_.erase (*it_job);

        idles.pop();
        to_steal_from.pop();

        if (richest_worker.stealing_allowed())
        {
          to_steal_from.emplace (richest);
        }
      }
    }

    template <typename T>
      std::unordered_set<sdpa::job_id_t> WorkerManager::delete_or_cancel_worker_jobs
        ( worker_id_t const& worker_id
        , std::function<Job* (sdpa::job_id_t const&)> get_job
        , std::function<T* (sdpa::job_id_t const&)> get_reservation
        , std::function<void (sdpa::worker_id_t const&, job_id_t const&)> cancel_worker_job
        )
    {
      std::lock_guard<std::mutex> const _(mtx_);

      Worker const& worker (worker_map_.at (worker_id));

      std::unordered_set<sdpa::job_id_t> jobs_to_reschedule
        ( worker.pending_.begin()
        , worker.pending_.end()
        );

      std::unordered_set<sdpa::job_id_t> jobs_to_cancel;
      std::set_union ( worker.submitted_.begin()
                     , worker.submitted_.end()
                     , worker.acknowledged_.begin()
                     , worker.acknowledged_.end()
                     , std::inserter (jobs_to_cancel, jobs_to_cancel.begin())
                      );

      for (job_id_t const& jobId : jobs_to_cancel)
      {
        Job* const pJob = get_job (jobId);
        fhg_assert (pJob);

        T* reservation (get_reservation (jobId));
        pJob->Reschedule();

        //! \note would never be set otherwise (function is only
        //! called after a worker died)
        reservation->mark_as_canceled_if_no_result_stored_yet (worker_id);

        if ( !reservation->apply_to_workers_without_result // false for worker 5, true for worker 4
               ( [&jobId, &cancel_worker_job, this] (worker_id_t const& wid)
                 {
                   cancel_worker_job (wid, jobId);
                 }
               )
           )
        {
          jobs_to_reschedule.emplace (jobId);
        }
      }

      return jobs_to_reschedule;
    }
  }
}
