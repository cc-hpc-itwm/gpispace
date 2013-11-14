/*
 * =====================================================================================
 *
 *       Filename:  Scheduler.hpp
 *
 *    Description:  Defines a basic scheduler interface
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
#ifndef SDPA_SCHEDULER_HPP
#define SDPA_SCHEDULER_HPP 1

#include <sdpa/daemon/Job.hpp>
#include <sdpa/daemon/Worker.hpp>
#include <sdpa/daemon/exceptions.hpp>
#include <sdpa/events/ErrorEvent.hpp>

#include <sdpa/engine/IWorkflowEngine.hpp>
#include <sdpa/daemon/scheduler/Reservation.hpp>

#include <boost/optional.hpp>

namespace sdpa {
namespace daemon {
  class Scheduler {
  public:
    typedef sdpa::shared_ptr<Scheduler> ptr_t;

    virtual ~Scheduler() {}

    virtual const Worker::worker_id_t& findWorker(const sdpa::job_id_t& job_id) throw (NoWorkerFoundException) = 0;
    virtual const Worker::worker_id_t& findSubmOrAckWorker(const sdpa::job_id_t& job_id) throw (NoWorkerFoundException) = 0;
    virtual const Worker::ptr_t& findWorker(const Worker::worker_id_t&  ) throw(WorkerNotFoundException) = 0;

    virtual void deleteWorkerJob(const Worker::worker_id_t& worker_id, const sdpa::job_id_t &job_id ) throw (JobNotDeletedException, WorkerNotFoundException) = 0;
    virtual void acknowledgeJob(const Worker::worker_id_t& worker_id, const sdpa::job_id_t& job_id) throw(WorkerNotFoundException, JobNotFoundException) = 0;

    virtual void addWorker( const Worker::worker_id_t& workerId,
                            const boost::optional<unsigned int>& capacity = boost::none,
                            const capabilities_set_t& cpbset = capabilities_set_t(),
                            const unsigned int& agent_rank = 0,
                            const sdpa::worker_id_t& agent_uuid = "") throw (WorkerAlreadyExistException) = 0;

    virtual void deleteWorker( const Worker::worker_id_t& workerId) throw (WorkerNotFoundException) = 0;

    virtual void getWorkerList(worker_id_list_t&) = 0;
    virtual Worker::worker_id_t getWorkerId(unsigned int rank) = 0;

    virtual void setLastTimeServed(const worker_id_t&, const sdpa::util::time_type&) = 0;

    virtual size_t numberOfWorkers() = 0;
    virtual void feedWorkers()= 0;
    virtual void removeWorkers() = 0;

    virtual bool addCapabilities(const sdpa::worker_id_t&, const sdpa::capabilities_set_t& cpbset) = 0;
    virtual void removeCapabilities(const sdpa::worker_id_t&, const sdpa::capabilities_set_t& cpbset) throw (WorkerNotFoundException) = 0;
    virtual void getAllWorkersCapabilities(sdpa::capabilities_set_t& cpbset) = 0;
    virtual void getWorkerCapabilities(const sdpa::worker_id_t&, sdpa::capabilities_set_t& cpbset) = 0;

    virtual void schedule(const sdpa::job_id_t& jobId) = 0;
    virtual void schedule_remotely(const sdpa::job_id_t &job) = 0;
    virtual void schedule_local(const sdpa::job_id_t &job) = 0;

    virtual void rescheduleWorkerJob(const Worker::worker_id_t& worker_id, const sdpa::job_id_t &job) = 0;
    virtual void rescheduleJob(const sdpa::job_id_t&) = 0;

    virtual bool has_job(const sdpa::job_id_t& job_id) = 0;
    virtual void delete_job(const sdpa::job_id_t & job_id) = 0;
    virtual void assignJobsToWorkers() = 0;

    virtual void workerFinished(const worker_id_t&, const job_id_t&) = 0;
    virtual void workerFailed(const worker_id_t&, const job_id_t&) = 0;
    virtual void workerCanceled(const worker_id_t&, const job_id_t&) = 0;
    virtual bool allPartialResultsCollected(const job_id_t&) = 0;
    virtual bool groupFinished(const sdpa::job_id_t&) = 0;
    virtual void releaseReservation(const sdpa::job_id_t& jobId) = 0;

    virtual void start(IAgent*) = 0;
    virtual void stop() = 0;
    virtual void run() = 0;
    virtual void print() = 0;

    virtual void printPendingJobs() = 0;
  };
}}

#endif
