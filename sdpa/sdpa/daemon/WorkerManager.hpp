//tiberiu.rotaru@itwm.fraunhofer.de

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
#include <unordered_map>
#include <unordered_set>

namespace sdpa
{
  namespace daemon
  {
    class WorkerManager : boost::noncopyable
    {
    public:
      const boost::optional<worker_id_t> findSubmOrAckWorker (const sdpa::job_id_t& job_id) const;

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

      boost::optional<double> matchRequirements
        ( const worker_id_t& worker
        , const job_requirements_t& job_req_set
        ) const;

      template <typename Reservation>
           void steal_work ( std::list<job_id_t> pending_jobs
                           , std::function<Reservation* (job_id_t const&)> reservation
                           , std::function<job_requirements_t (const sdpa::job_id_t&)>
                               requirements
                           );

    bool submit_and_serve_if_can_start_job_INDICATES_A_RACE
      ( job_id_t const&, std::set<worker_id_t> const&
      , std::function<void ( std::set<worker_id_t> const&
                           , const job_id_t&
                           )> const& serve_job
      );

    bool all_workers_busy_and_have_pending_jobs() const;

    std::set<job_id_t> remove_all_matching_pending_jobs
      ( const worker_id_t&
      , const job_id_list_t&
      , std::function<std::set<worker_id_t> (job_id_t const&)>
      , std::function<job_requirements_t (const sdpa::job_id_t&)>
      );

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
      typedef std::unordered_map<worker_id_t, Worker> worker_map_t;
      worker_map_t  worker_map_;
      worker_connections_t worker_connections_;

      mutable boost::mutex mtx_;
    };

    template <typename Reservation>
    void WorkerManager::steal_work
      ( std::list<job_id_t> pending_jobs
      , std::function<Reservation* (job_id_t const&)> reservation
      , std::function<job_requirements_t (const sdpa::job_id_t&)>
          requirements
      )
    {
      boost::mutex::scoped_lock const _(mtx_);

      auto comp = [this] (worker_id_t a, worker_id_t b)
                  { return worker_map_.at (a).lastTimeServed()
                      > worker_map_.at (b).lastTimeServed();
                  };

      std::set<worker_id_t, decltype(comp)> idle_workers (comp);
      boost::copy ( worker_map_
                  | boost::adaptors::filtered
                      ([] (std::pair<worker_id_t, Worker> const& worker)
                       {  return worker.second.pending_.empty()
                            && worker.second.submitted_.empty()
                            && worker.second.acknowledged_.empty();
                       }
                      )
                  | boost::adaptors::map_keys
                  , std::inserter (idle_workers, idle_workers.begin())
                  );

      std::unordered_set<worker_id_t> idle_workers_assigned;

      for (worker_id_t const& w : idle_workers)
      {
        std::list<job_id_t> ::iterator
          it_job (std::find_if ( pending_jobs.begin()
                               , pending_jobs.end()
                               , [&w, &requirements, this] (job_id_t job)
                                 {return matchRequirements (w, requirements(job));}
                               )
                 );

        if (it_job != pending_jobs.end())
        {
          std::set<worker_id_t> replaceable_workers;
          std::set<worker_id_t> const& reserved_workers
            (reservation (*it_job)->workers());

          std::set_difference ( reserved_workers.begin()
                              , reserved_workers.end()
                              , idle_workers_assigned.begin()
                              , idle_workers_assigned.end()
                              , std::inserter ( replaceable_workers
                                              , replaceable_workers.begin()
                                              )
                              );

          std::set<worker_id_t>::iterator
            it_w (std::max_element ( replaceable_workers.begin()
                                   , replaceable_workers.end()
                                   , [this] (worker_id_t wl, worker_id_t wr)
                                     {return worker_map_.at (wl).pending_.size()
                                        > worker_map_.at (wr).pending_.size();
                                     }
                                   )
                 );

          if (it_w != replaceable_workers.end())
          {
            reservation (*it_job)->replace_worker (*it_w, w);
            worker_map_.at (*it_w).deleteJob (*it_job);
            worker_map_.at (w).assign (*it_job);

            idle_workers_assigned.insert (w);
          }
        }
      }
    }
  }
}
