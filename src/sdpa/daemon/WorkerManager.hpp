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

      mmap_match_deg_worker_id_t getMatchingDegreesAndWorkers (const job_requirements_t&) const;

      double cost_assigned_jobs (const worker_id_t, std::function<double (job_id_t job_id)>);

      template <typename Reservation>
      void steal_work (std::function<Reservation* (job_id_t const&)> reservation);

    bool submit_and_serve_if_can_start_job_INDICATES_A_RACE
      ( job_id_t const&, std::set<worker_id_t> const&
      , std::function<void ( std::set<worker_id_t> const&
                           , const job_id_t&
                           )> const& serve_job
      );

    bool all_workers_busy_and_have_pending_jobs() const;

    std::unordered_set<job_id_t>
      remove_pending_jobs_from_workers_with_similar_capabilities
        (worker_id_t const&);

    void assign_job_to_worker (const job_id_t&, const worker_id_t&);
    void acknowledge_job_sent_to_worker (const job_id_t&, const worker_id_t&);
    void delete_job_from_worker (const job_id_t &job_id, const worker_id_t& );
    const capabilities_set_t& worker_capabilities (const worker_id_t&) const;
    const std::set<job_id_t> get_worker_jobs_and_clean_queues (const worker_id_t&);
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
      void change_equivalence_class (worker_map_t::const_iterator, std::set<std::string> const&);

      boost::optional<double> matchRequirements
        ( Worker const&
        , const job_requirements_t& job_req_set
        ) const;

      worker_map_t  worker_map_;
      worker_connections_t worker_connections_;
      std::map<std::set<std::string>, WorkerEquivalenceClass> worker_equiv_classes_;

      mutable boost::mutex mtx_;
    };

    template <typename Reservation>
    void WorkerManager::steal_work
      (std::function<Reservation* (job_id_t const&)> reservation)
    {
      boost::mutex::scoped_lock const _(mtx_);
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

      std::function<bool (worker_id_t const&, worker_id_t const&)> const
        comp { [&worker_map, &cost] ( worker_id_t const& lhs
                                    , worker_id_t const& rhs
                                    )
               {
                 return worker_map.at (lhs).cost_assigned_jobs (cost)
                   < worker_map.at (rhs).cost_assigned_jobs (cost);
               }
             };

      std::priority_queue < worker_id_t
                          , std::vector<worker_id_t>
                          , decltype (comp)
                          > to_steal_from (comp);

      std::function<bool (worker_id_t const&, worker_id_t const&)> const
        comp_idles { [&worker_map, &cost] ( worker_id_t const& lhs
                                          , worker_id_t const& rhs
                                          )
                     {
                       return worker_map.at (lhs).lastTimeServed()
                         > worker_map.at (rhs).lastTimeServed();
                     }
                   };

      std::priority_queue < worker_id_t
                          , std::vector<worker_id_t>
                          , decltype (comp_idles)
                          > idles (comp_idles);

      for (worker_id_t const& w : _worker_ids)
      {
        Worker const& worker (worker_map.at (w));

        bool const has_pending (!worker.pending_.empty());
        bool const has_running (!worker.submitted_.empty() || !worker.acknowledged_.empty());

        if ((has_pending && has_running) || (worker.pending_.size() > 1))
        {
          to_steal_from.emplace (w);
        }
        else if (!has_running && !has_pending)
        {
          idles.emplace (w);
        }
      }

      while (!(idles.empty() || to_steal_from.empty()))
      {
        worker_id_t const& richest (to_steal_from.top());
        worker_id_t const& thief (idles.top());
        Worker& richest_worker (worker_map.at (richest));

        auto it_job (std::max_element ( richest_worker.pending_.begin()
                                      , richest_worker.pending_.end()
                                      , [&reservation] ( worker_id_t const& r
                                                       , worker_id_t const& l
                                                       )
                                        {
                                          return reservation (r)->cost()
                                            < reservation (l)->cost();
                                        }
                                      )
                    );

        reservation (*it_job)->replace_worker (richest, thief);

        worker_map.at (thief).assign (*it_job);
        richest_worker.pending_.erase (*it_job);

        idles.pop();
        to_steal_from.pop();

        if (richest_worker.pending_.size() > 1)
        {
          to_steal_from.emplace (richest);
        }
      }
    }
  }
}
