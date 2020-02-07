#pragma once

#include <sdpa/daemon/Worker.hpp>
#include <sdpa/daemon/scheduler/Reservation.hpp>
#include <sdpa/events/CancelJobEvent.hpp>
#include <sdpa/requirements_and_preferences.hpp>

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

    using WorkerSet = std::set<worker_id_t>;
    using Workers_and_implementation = std::pair<WorkerSet, Implementation>;

    class WorkerManager : boost::noncopyable
    {
      typedef std::unordered_map<worker_id_t, Worker> worker_map_t;
      using worker_iterator = worker_map_t::iterator;

    private:
      class WorkerEquivalenceClass
      {
        friend class WorkerManager;

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
        unsigned int n_workers() const;

        void add_worker_entry (worker_iterator);
        void remove_worker_entry (worker_iterator);

        void steal_work
          (std::function<scheduler::Reservation* (job_id_t const&)>, WorkerManager&);

      private:
        unsigned int _n_pending_jobs;
        unsigned int _n_running_jobs;
        std::unordered_set<worker_id_t> _worker_ids;
        std::unordered_set<worker_id_t> _idle_workers;
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

      void steal_work (std::function<scheduler::Reservation* (job_id_t const&)> reservation);

    bool submit_and_serve_if_can_start_job_INDICATES_A_RACE
      ( job_id_t const&
      , WorkerSet const&
      , Implementation const&
      , std::function<void ( WorkerSet const&
                           , Implementation const&
                           , const job_id_t&
                           )> const& serve_job
      );

    std::unordered_set<sdpa::job_id_t> delete_or_cancel_worker_jobs
      ( worker_id_t const&
      , std::function<Job* (sdpa::job_id_t const&)>
      , std::function<scheduler::Reservation* (sdpa::job_id_t const&)>
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
      void assign_job_to_worker
        (const job_id_t& job_id, worker_iterator worker, double cost);
      void delete_job_from_worker
        (const job_id_t &job_id, const worker_iterator worker, double cost);
      void submit_job_to_worker (const job_id_t&, const worker_id_t&);
      void change_equivalence_class (worker_iterator, std::set<std::string> const&);

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
  }
}
