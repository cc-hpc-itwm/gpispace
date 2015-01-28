//tiberiu.rotaru@itwm.fraunhofer.de

#ifndef SDPA_DAEMON_WORKER_MANAGER_HPP
#define SDPA_DAEMON_WORKER_MANAGER_HPP 1

#include <sdpa/daemon/Worker.hpp>
#include <sdpa/job_requirements.hpp>

#include <boost/bimap.hpp>
#include <boost/bimap/unordered_set_of.hpp>
#include <boost/optional.hpp>

#include <unordered_map>

namespace sdpa
{
  namespace daemon
  {
    class WorkerManager
    {
    public:
      typedef std::unordered_map<worker_id_t, Worker::ptr_t> worker_map_t;

      bool hasWorker (const worker_id_t& worker_id) const;
      const boost::optional<worker_id_t> findSubmOrAckWorker (const sdpa::job_id_t& job_id) const;

      std::string host (const sdpa::worker_id_t& worker) const;

      //! returns whether worker was actually added (i.e. false when already there)
      bool addWorker ( const worker_id_t& workerId
                     , boost::optional<unsigned int> capacity
                     , const capabilities_set_t& cpbset
                     , const bool children_allowed
                     , const std::string& hostname
                     , const fhg::com::p2p::address_t& address
                     );

      void deleteWorker (const worker_id_t& workerId);

      void getCapabilities (sdpa::capabilities_set_t& cpbset) const;

      std::set<worker_id_t> getAllNonReservedWorkers() const;

      mmap_match_deg_worker_id_t getMatchingDegreesAndWorkers (const job_requirements_t&) const;

      double cost_assigned_jobs (const worker_id_t, std::function<double (job_id_t job_id)>);

      boost::optional<std::size_t> matchRequirements
        ( const worker_id_t& worker
        , const job_requirements_t& job_req_set
        ) const;

    bool can_start_job (std::set<worker_id_t> workers) const;

    bool all_workers_busy_and_have_pending_jobs() const;

    std::set<job_id_t> remove_all_matching_pending_jobs (const job_id_list_t&);

    void assign_job_to_worker (const job_id_t&, const worker_id_t&);
    void submit_job_to_worker (const job_id_t&, const worker_id_t&);
    void acknowledge_job_sent_to_worker (const job_id_t&, const worker_id_t&);
    void delete_job_from_worker (const job_id_t &job_id, const worker_id_t& );
    const capabilities_set_t& worker_capabilities (const worker_id_t&) const;
    const std::set<job_id_t> get_worker_jobs_and_clean_queues (const worker_id_t&) const;
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

    private:
      worker_map_t  worker_map_;
      worker_connections_t worker_connections_;

      mutable boost::mutex mtx_;
    };
  }
}
#endif
