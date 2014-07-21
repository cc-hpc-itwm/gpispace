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
#include <sdpa/job_requirements.hpp>

#include <boost/optional.hpp>

#include <unordered_map>

namespace sdpa { namespace daemon {
  class WorkerManager  {
  public:
    typedef std::unordered_map<worker_id_t, Worker::ptr_t> worker_map_t;

    Worker::ptr_t findWorker(const worker_id_t& worker_id);
    bool hasWorker(const worker_id_t& worker_id) const;
    const boost::optional<worker_id_t> findSubmOrAckWorker(const sdpa::job_id_t& job_id) const;

    //! returns whether worker was actually added (i.e. false when already there)
    bool addWorker( const worker_id_t& workerId,
                    boost::optional<unsigned int> capacity,
                    const capabilities_set_t& cpbset = capabilities_set_t() );

    void deleteWorker( const worker_id_t& workerId);

    void getCapabilities(sdpa::capabilities_set_t& cpbset);

    sdpa::job_id_list_t getJobListAndCleanQueues(const  Worker::ptr_t& pWorker);
    worker_id_list_t getListWorkersNotReserved();

    boost::optional<sdpa::worker_id_t> getBestMatchingWorker( const job_requirements_t&, const sdpa::worker_id_list_t&) const;
    mmap_match_deg_worker_id_t getListMatchingWorkers (const job_requirements_t&, const sdpa::worker_id_list_t&) const;

private:
    worker_map_t  worker_map_;

    mutable boost::mutex mtx_;
  };
}}

#endif
