#pragma once

#include <sdpa/daemon/Worker.hpp>
#include <sdpa/requirements_and_preferences.hpp>
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
    class worker_id_host_info_t
    {
    public:
      worker_id_host_info_t ( worker_id_t worker_id
                            , std::string worker_host
                            , unsigned long shared_memory_size
                            , double last_time_idle
                            , boost::optional<std::string> implementation
                            );

      const worker_id_t& worker_id() const {return worker_id_;}
      const std::string& worker_host() const {return worker_host_;}
      double last_time_idle() const {return _last_time_idle;}
      unsigned long shared_memory_size() const {return shared_memory_size_;}
      boost::optional<std::string> const& implementation() const;

    private:
      worker_id_t worker_id_;
      std::string worker_host_;
      unsigned long shared_memory_size_;
      double _last_time_idle;
      boost::optional<std::string> _implementation;
    };
    using mmap_match_deg_worker_id_t
      = std::multimap<double, worker_id_host_info_t, std::greater<double>>;

    using Implementation = boost::optional<std::string>;
    using WorkerSet = std::set<worker_id_t>;
    using Workers_and_implementation = std::pair<WorkerSet, Implementation>;

    class WorkerManager : boost::noncopyable
    {
      typedef std::unordered_map<worker_id_t, Worker> worker_map_t;
      using worker_ptr = worker_map_t::iterator;

    private:
      class WorkerEquivalenceClass
      {
        friend class WorkerManager;

        struct iterator_hash
        {
          size_t operator()(worker_ptr it) const
          {
            return std::hash<worker_id_t>() (it->first);
          }
        };

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

        void add_worker_entry (worker_ptr);
        void remove_worker_entry (worker_ptr);

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
        std::unordered_set<worker_ptr, iterator_hash> _idle_workers;
      };

    public:
      std::unordered_set<worker_id_t> findSubmOrAckWorkers
        (const sdpa::job_id_t& job_id) const;

      std::string host_INDICATES_A_RACE (const sdpa::worker_id_t& worker) const;

      //! throws if workerId was not unique
      void addWorker ( const worker_id_t& workerId
                     , const capabilities_set_t& cpbset
                     , unsigned long allocated_shared_memory_size
                     , const bool children_allowed
                     , const std::string& hostname
                     , const fhg::com::p2p::address_t& address
                     );

      void deleteWorker (const worker_id_t& workerId);

      void getCapabilities (sdpa::capabilities_set_t& cpbset) const;

      Workers_and_implementation find_assignment
        (const Requirements_and_preferences&) const;

      template <typename Reservation>
      void steal_work (std::function<Reservation* (job_id_t const&)> reservation);

    bool submit_and_serve_if_can_start_job_INDICATES_A_RACE
      ( job_id_t const&
      , WorkerSet const&
      , Implementation const&
      , std::function<void ( WorkerSet const&
                           , Implementation const&
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

    void assign_job_to_worker (const job_id_t&, const worker_id_t&, double cost);
    void acknowledge_job_sent_to_worker (const job_id_t&, const worker_id_t&);
    void delete_job_from_worker (const job_id_t &job_id, const worker_id_t&, double);
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

    private:
      void submit_job_to_worker (const job_id_t&, const worker_id_t&);
      void change_equivalence_class (worker_ptr, std::set<std::string> const&);

      std::pair<boost::optional<double>, boost::optional<std::string>>
        match_requirements_and_preferences
          ( std::set<std::string> const& capabilities
          , const Requirements_and_preferences&
          ) const;

      worker_map_t  worker_map_;
      worker_connections_t worker_connections_;
      std::map<std::set<std::string>, WorkerEquivalenceClass> worker_equiv_classes_;

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

      std::vector<worker_ptr> thief_candidates;
      for (worker_id_t const& w : _worker_ids)
      {
        auto const& it (worker_map.find (w));
        fhg_assert (it != worker_map.end());
        Worker const& worker (it->second);

        if (!worker.has_running_jobs() && !worker.has_pending_jobs())
        {
          thief_candidates.emplace_back (it);
        }
      }

      if (thief_candidates.empty())
      {
        return;
      }

      std::function<double (job_id_t const& job_id)> const cost
        { [&reservation] (job_id_t const& job_id)
          {
            return reservation (job_id)->cost();
          }
        };

      std::function<bool (worker_ptr const&, worker_ptr const&)> const
        comp { [] (worker_ptr const& lhs, worker_ptr const& rhs)
               {
                 return lhs->second.cost_assigned_jobs()
                   < rhs->second.cost_assigned_jobs();
               }
             };

      std::priority_queue < worker_ptr
                          , std::vector<worker_ptr>
                          , decltype (comp)
                          > to_steal_from (comp);

      std::function<bool (worker_ptr const&, worker_ptr const&)> const
        comp_thieves { [] (worker_ptr const& lhs, worker_ptr const& rhs)
                     {
                       return lhs->second._last_time_idle
                         > rhs->second._last_time_idle;
                     }
                   };

      std::priority_queue < worker_ptr
                          , std::vector<worker_ptr>
                          , decltype (comp_thieves)
                          > thieves (comp_thieves, thief_candidates);

      for (worker_id_t const& w : _worker_ids)
      {
        auto const& it (worker_map.find (w));
        Worker const& worker (it->second);

        if (worker.stealing_allowed())
        {
          to_steal_from.emplace (it);
        }
      }

      while (!(thieves.empty() || to_steal_from.empty()))
      {
        worker_ptr const richest (to_steal_from.top());
        worker_ptr const& thief (thieves.top());
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

        reservation (*it_job)->replace_worker
          ( richest->first
          , thief->first
          , [&thief] (const std::string& cpb)
            {
              return thief->second.hasCapability (cpb);
            }
          );

        thief->second.assign (*it_job, cost (*it_job));
        richest_worker.delete_pending_job (*it_job, cost (*it_job));

        thieves.pop();
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
               ( [&jobId, &cancel_worker_job] (worker_id_t const& wid)
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
