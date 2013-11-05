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
#include <sdpa/daemon/Scheduler.hpp>


namespace sdpa { namespace daemon {
  class WorkerManager  {
  public:
    typedef sdpa::shared_ptr<WorkerManager> ptr_t;
    typedef boost::recursive_mutex mutex_type;
    typedef boost::unique_lock<mutex_type> lock_type;
    typedef boost::condition_variable_any condition_type;

    typedef SynchronizedQueue<std::list<sdpa::job_id_t> > JobQueue;
    typedef boost::unordered_map<Worker::worker_id_t, Worker::ptr_t> worker_map_t;
    typedef boost::unordered_map<sdpa::job_id_t, sdpa::list_match_workers_t> mapJob2PrefWorkersList_t;

    WorkerManager();
    virtual ~WorkerManager();

    Worker::ptr_t& findWorker(const Worker::worker_id_t& worker_id) throw (WorkerNotFoundException);
    const Worker::worker_id_t& findWorker(const sdpa::job_id_t& job_id) throw (NoWorkerFoundException);
    const Worker::worker_id_t& findSubmOrAckWorker(const sdpa::job_id_t& job_id) throw (NoWorkerFoundException);

    void addWorker( const Worker::worker_id_t& workerId,
                unsigned int capacity,
                const capabilities_set_t& cpbset = capabilities_set_t(),
                const unsigned int& agent_rank = 0,
                const sdpa::worker_id_t& agent_uuid = "" ) throw (WorkerAlreadyExistException);

    void deleteWorker( const Worker::worker_id_t& workerId) throw (WorkerNotFoundException);
    void removeWorkers();

    bool addCapabilities(const sdpa::worker_id_t&, const sdpa::capabilities_set_t& cpbset);
    virtual void removeCapabilities(const sdpa::worker_id_t&, const sdpa::capabilities_set_t& cpbset)  throw (WorkerNotFoundException);
    virtual void getCapabilities(const std::string& agentName, sdpa::capabilities_set_t& cpbset);

    const Worker::ptr_t& getNextWorker() throw (NoWorkerFoundException);
    worker_id_t getLeastLoadedWorker() throw (NoWorkerFoundException, AllWorkersFullException);

    void setLastTimeServed(const worker_id_t&, const sdpa::util::time_type&);

    void dispatchJob(const sdpa::job_id_t& jobId);
    void deleteJob(const sdpa::job_id_t& jobId);
    void deleteWorkerJob(const Worker::worker_id_t& worker_id, const sdpa::job_id_t &job_id ) throw (JobNotDeletedException, WorkerNotFoundException);

    size_t numberOfWorkers() { return worker_map_.size(); }
    sdpa::job_id_list_t getJobListAndCleanQueues(const  Worker::ptr_t& pWorker);
    void getWorkerList(sdpa::worker_id_list_t& workerList);
    void getListNotFullWorkers(sdpa::worker_id_list_t& workerList);
    void getListWorkersNotReserved(sdpa::worker_id_list_t& workerList);

    sdpa::worker_id_t getBestMatchingWorker( const job_requirements_t&, sdpa::worker_id_list_t&) throw (NoWorkerFoundException);

    void cancelWorkerJobs(sdpa::daemon::Scheduler*);
    void forceOldWorkerJobsTermination();
    virtual Worker::worker_id_t getWorkerId(unsigned int r);
    void reserveWorker(const sdpa::worker_id_t&) throw (WorkerReservationFailed);

    bool has_job(const sdpa::job_id_t& job_id);

    friend class SchedulerImpl;

    void print()
    {
      if(!common_queue_.empty())
      {
        SDPA_LOG_DEBUG("The content of the common queue is: ");
        common_queue_.print();
      }
      else
        SDPA_LOG_DEBUG("No job without preferences available!");

      if( worker_map_.begin() == worker_map_.end() )
      {
        SDPA_LOG_DEBUG("The worker manager has NO worker! ");
      }
      else
      {
        for( worker_map_t::iterator it = worker_map_.begin(); it!=worker_map_.end(); it++)
          (*it).second->print();
      }
    }

protected:
    worker_map_t  worker_map_;

    SDPA_DECLARE_LOGGER();
    worker_map_t::iterator iter_last_worker_;

    JobQueue common_queue_;

    mutable mutex_type mtx_;
  };

  template <typename TPtrWorker, typename TReqSet>
  int matchRequirements( const TPtrWorker& pWorker, const TReqSet job_req_set, bool bOwn = false )
  {
    int matchingDeg = 0;

    // for all job requirements
    const requirement_list_t& listR = job_req_set.getReqList();
    for( typename TReqSet::const_iterator it = listR.begin(); it != listR.end(); it++ )
    {
      //LOG(ERROR, "Check if the worker "<<pWorker->name()<<" has the capability "<<it->value()<<" ... ");
      if( pWorker->hasCapability(it->value(), bOwn ) )
      {
        // increase the number of matchings
        matchingDeg++;
      }
      else // if the worker doesn't have the capability
        if( it->is_mandatory()) // and the capability is mandatory -> return immediately with a matchingDegree -1
        {
          // At least one mandatory requirement is not fulfilled
          return -1;
        }
    }

    return matchingDeg;
  }

}}

#endif
