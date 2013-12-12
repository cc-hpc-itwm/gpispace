/*
 * =====================================================================================
 *
 *       Filename:  WorkerManager.hpp
 *
 *    Description:  Worker manager
 *
 *        Version:  1.0
 *        Created:
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Dr. Tiberiu Rotaru, tiberiu.rotaru@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */
#ifndef SDPA_DAEMON_WORKER_MANAGER_HPP
#define SDPA_DAEMON_WORKER_MANAGER_HPP 1

#include <sdpa/daemon/Worker.hpp>
#include <sdpa/daemon/exceptions.hpp>
#include <boost/unordered_map.hpp>
#include <sdpa/engine/IWorkflowEngine.hpp>

#include <boost/optional.hpp>

namespace sdpa { namespace daemon {
  class WorkerManager  {
  public:
    typedef boost::shared_ptr<WorkerManager> ptr_t;
    typedef boost::recursive_mutex mutex_type;
    typedef boost::unique_lock<mutex_type> lock_type;
    typedef boost::condition_variable_any condition_type;

    typedef SynchronizedQueue<std::list<sdpa::job_id_t> > JobQueue;
    typedef boost::unordered_map<Worker::worker_id_t, Worker::ptr_t> worker_map_t;
    typedef boost::unordered_map<sdpa::job_id_t, sdpa::list_match_workers_t> mapJob2PrefWorkersList_t;

    WorkerManager();

    Worker::ptr_t findWorker(const Worker::worker_id_t& worker_id);
    bool hasWorker(const Worker::worker_id_t& worker_id) const;
    bool isDisconnectedWorker(const Worker::worker_id_t& worker_id) const;
    const Worker::worker_id_t& findWorker(const sdpa::job_id_t& job_id);
    const boost::optional<Worker::worker_id_t> findSubmOrAckWorker(const sdpa::job_id_t& job_id) const;
    const boost::optional<Worker::worker_id_t> getAssignedWorker(const sdpa::job_id_t& job_id);

    void addWorker( const Worker::worker_id_t& workerId,
                boost::optional<unsigned int> capacity,
                const capabilities_set_t& cpbset = capabilities_set_t(),
                const unsigned int& agent_rank = 0,
                  const sdpa::worker_id_t& agent_uuid = "" );

    void deleteWorker( const Worker::worker_id_t& workerId);

    bool addCapabilities(const sdpa::worker_id_t&, const sdpa::capabilities_set_t& cpbset);
    void removeCapabilities(const sdpa::worker_id_t&, const sdpa::capabilities_set_t& cpbset);
    void getCapabilities(const std::string& agentName, sdpa::capabilities_set_t& cpbset);

    void dispatchJob(const sdpa::job_id_t& jobId);
    void deleteJob(const sdpa::job_id_t& jobId);
    void deleteWorkerJob(const Worker::worker_id_t& worker_id, const sdpa::job_id_t &job_id );

    size_t numberOfWorkers() { return worker_map_.size(); }
    sdpa::job_id_list_t getJobListAndCleanQueues(const  Worker::ptr_t& pWorker);
    void getWorkerList(sdpa::worker_id_list_t& workerList);
    void getListNotFullWorkers(sdpa::worker_id_list_t& workerList);
    void getListWorkersNotReserved(sdpa::worker_id_list_t& workerList);

    sdpa::worker_id_t getBestMatchingWorker( const job_requirements_t&, const sdpa::worker_id_list_t&) const;

    void reserveWorker(const sdpa::worker_id_t&);

    bool has_job(const sdpa::job_id_t& job_id);

    friend class SchedulerBase; // SchedulerBase::schedule_first()
    void markJobSubmitted(const sdpa::worker_id_list_t& worker_id_list, const sdpa::job_id_t& job_id);
protected:
    worker_map_t  worker_map_;

    SDPA_DECLARE_LOGGER();

    JobQueue common_queue_;

    mutable mutex_type mtx_;
  };
}}

#endif
