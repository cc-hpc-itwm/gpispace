//tiberiu.rotaru@itwm.fraunhofer.de

#pragma once

#include <sdpa/daemon/Worker.hpp>
#include <sdpa/job_requirements.hpp>

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

      template <typename Reservation>
      void steal_work ( std::function<Reservation* (job_id_t const&)> reservation
                      , std::function<job_requirements_t (const sdpa::job_id_t&)>
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

    private:
      boost::optional<double> matchRequirements
        ( Worker const&
        , const job_requirements_t& job_req_set
        ) const;

      typedef std::unordered_map<worker_id_t, Worker> worker_map_t;
      worker_map_t  worker_map_;
      worker_connections_t worker_connections_;

      mutable boost::mutex mtx_;
    };

    template <typename Reservation>
    void WorkerManager::steal_work
      ( std::function<Reservation* (job_id_t const&)> reservation
      , std::function<job_requirements_t (const sdpa::job_id_t&)>
          requirements
      )
    {
      boost::mutex::scoped_lock const _(mtx_);
      auto comp = [this] ( decltype (worker_map_)::iterator const& lhs
                         , decltype (worker_map_)::iterator const& rhs
                         )
                 { return lhs->second.pending_.size()
                     > rhs->second.pending_.size();
                 };

      std::set<decltype (worker_map_)::iterator, decltype(comp)> workers_to_steal_from (comp);
      std::list<decltype (worker_map_)::iterator> idle_workers;

      for ( decltype (worker_map_)::iterator worker_it (worker_map_.begin())
          ; worker_it != worker_map_.end()
          ; ++worker_it
          )
      {
        auto const job_count ( worker_it->second.pending_.size()
                             + worker_it->second.submitted_.size()
                             + worker_it->second.acknowledged_.size()
                             );

        if (job_count > 1)
        {
          workers_to_steal_from.emplace (worker_it);
        }
        else if (job_count == 0)
        {
          idle_workers.emplace_back (worker_it);
        }
      }

      if ( workers_to_steal_from.empty()
        || idle_workers.empty()
         )
        return;

      for (decltype (worker_map_)::iterator const& w : workers_to_steal_from)
      {
        Worker& worker (w->second);

        for ( auto idle_worker_it (idle_workers.begin())
            ; idle_worker_it != idle_workers.end()
            ; ++idle_worker_it
            )
        {
          worker_id_t const& idle_worker_id ((*idle_worker_it)->first);
          Worker& idle_worker ((*idle_worker_it)->second);

          std::set<job_id_t>::iterator const it_job
            ( std::find_if ( worker.pending_.begin()
                           , worker.pending_.end()
                           , [&idle_worker, &requirements, this] (job_id_t job)
                             {
                               return matchRequirements ( idle_worker
                                                        , requirements(job)
                                                        );
                             }
                           )
            );

          if (it_job != worker.pending_.end())
          {
            reservation (*it_job)->replace_worker (w->first, idle_worker_id);

            worker.deleteJob (*it_job);
            idle_worker.assign (*it_job);
            idle_workers.erase (idle_worker_it);

            break;
          }
        }

        if (idle_workers.empty())
          break;
      }
    }
  }
}
