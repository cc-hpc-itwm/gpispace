/*
 * =====================================================================================
 *
 *       Filename:  WorkerManager.cpp
 *
 *    Description:  Worker manager implementation
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

#include <fhg/assert.hpp>
#include <sdpa/daemon/WorkerManager.hpp>
#include <boost/unordered_map.hpp>
#include <algorithm>
#include <sdpa/types.hpp>
#include <boost/foreach.hpp>
#include "boost/bind.hpp"
#include <sdpa/daemon/IAgent.hpp>

#include <algorithm>
#include <limits>

#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/range/algorithm/count_if.hpp>

using namespace std;
using namespace sdpa::daemon;

WorkerManager::WorkerManager()
  : SDPA_INIT_LOGGER("sdpa::daemon::WorkerManager")
  , iter_last_worker_ (worker_map_.end())
{}

Worker::ptr_t &WorkerManager::findWorker(const Worker::worker_id_t& worker_id ) throw(WorkerNotFoundException)
{
  lock_type lock(mtx_);
  worker_map_t::iterator it = worker_map_.find(worker_id);
  if( it != worker_map_.end() )
    return it->second;
  else
    throw WorkerNotFoundException(worker_id);
}

const Worker::worker_id_t &WorkerManager::findWorker(const sdpa::job_id_t& job_id) throw (NoWorkerFoundException)
{
  lock_type lock(mtx_);

  BOOST_FOREACH ( Worker::ptr_t worker, worker_map_ | boost::adaptors::map_values
                | boost::adaptors::filtered
                  (boost::bind (&Worker::has_job, _1, job_id))
                )
  {
    return worker->name();
  }

  throw NoWorkerFoundException();
}

const Worker::worker_id_t& WorkerManager::findSubmOrAckWorker(const sdpa::job_id_t& job_id) throw (NoWorkerFoundException)
{
  lock_type lock(mtx_);

  BOOST_FOREACH ( Worker::ptr_t worker, worker_map_ | boost::adaptors::map_values
                | boost::adaptors::filtered
                  (boost::bind (&Worker::isJobSubmittedOrAcknowleged, _1, job_id))
                )
  {
    return worker->name();
  }

  throw NoWorkerFoundException();
}

void WorkerManager::addWorker(  const Worker::worker_id_t& workerId,
                                boost::optional<unsigned int> capacity,
                                const capabilities_set_t& cpbSet,
                                const unsigned int& agent_rank,
                                const sdpa::worker_id_t& agent_uuid ) throw (WorkerAlreadyExistException)
{
  lock_type lock(mtx_);

  bool bFound = false;
  for( worker_map_t::iterator it = worker_map_.begin(); !bFound && it != worker_map_.end(); it++ )
  {
    //if( it->second->name() ==  workerId )
    if( it->first == workerId )
    {
      //SDPA_LOG_ERROR("An worker with the id "<<workerId<<" already exist into the worker map!");
      bFound = true;
      throw WorkerAlreadyExistException(workerId, agent_uuid);
    }
  }

  Worker::ptr_t pWorker( new Worker( workerId, capacity, agent_rank, agent_uuid ) );
  pWorker->addCapabilities(cpbSet);

  worker_map_.insert(worker_map_t::value_type(pWorker->name(), pWorker));

  if (pWorker->capacity())
  {
    DMLOG (TRACE, "Created new worker: name = "<<pWorker->name()<<" with rank = "<<pWorker->rank()<<" and capacity = "<<*pWorker->capacity());
  }
  else
  {
    DMLOG (TRACE, "Created new worker: name = "<<pWorker->name()<<" with rank = "<<pWorker->rank()<<" and unlimited capacity");
  }

  if(worker_map_.size() == 1)
    iter_last_worker_ = worker_map_.begin();
}


/**
 * get next worker to be served (Round-Robin scheduling)
 */
const Worker::ptr_t& WorkerManager::getNextWorker() throw (NoWorkerFoundException)
{
  lock_type lock(mtx_);

  if( worker_map_.empty() )
    throw NoWorkerFoundException();

  if (iter_last_worker_ == worker_map_.end())
    iter_last_worker_ = worker_map_.begin();

  worker_map_t::iterator iter(iter_last_worker_);
  iter_last_worker_++;

  return iter->second;
}

void WorkerManager::dispatchJob(const sdpa::job_id_t& jobId)
{
  lock_type lock(mtx_);
  DLOG(TRACE, "Dispatch the job " << jobId.str() );
  common_queue_.push(jobId);
}

void WorkerManager::deleteJob (sdpa::job_id_t const & job)
{
  lock_type lock(mtx_);
  if (!common_queue_.erase(job))
  {
    BOOST_FOREACH ( Worker::ptr_t worker, worker_map_ | boost::adaptors::map_values
                  | boost::adaptors::filtered
                    (boost::bind (&Worker::has_job, _1, job))
                  )
    {
      worker->deleteJob (job);
    }
  }
}

void WorkerManager::deleteWorkerJob(const Worker::worker_id_t& worker_id, const sdpa::job_id_t &job_id ) throw (JobNotDeletedException, WorkerNotFoundException)
{
  lock_type lock(mtx_);
  try {
    Worker::ptr_t ptrWorker = findWorker(worker_id);
    // delete job from worker's queues

    DLOG(TRACE, "Deleting the job " << job_id.str() << " from the "<<worker_id<<"'s queues!");
    ptrWorker->deleteJob(job_id);
  }
  catch(JobNotDeletedException const &) {
    SDPA_LOG_ERROR("Could not delete the job "<<job_id.str()<<"!");
  }
  catch(WorkerNotFoundException const &) {
    SDPA_LOG_ERROR("Worker "<<worker_id<<" not found!");
  }
}

void WorkerManager::deleteWorker( const Worker::worker_id_t& workerId ) throw (WorkerNotFoundException)
{
  lock_type lock(mtx_);
  worker_map_t::iterator w (worker_map_.find (workerId));

  if (w == worker_map_.end())
    throw WorkerNotFoundException(workerId);

  worker_map_.erase (w);
  DMLOG (TRACE, "worker " << workerId << " removed");
}

bool WorkerManager::has_job(const sdpa::job_id_t& job_id)
{
  lock_type lock(mtx_);

  return common_queue_.has_item(job_id)
      || !boost::empty ( worker_map_
                       | boost::adaptors::map_values
                       | boost::adaptors::filtered
                         (boost::bind (&Worker::has_job, _1, job_id))
                       );
}

void WorkerManager::getWorkerList(sdpa::worker_id_list_t& workerList)
{
  lock_type lock(mtx_);
  for( worker_map_t::iterator iter = worker_map_.begin(); iter != worker_map_.end(); iter++ )
    workerList.push_back(iter->second->name());
}

class CComparator
{
public:
  CComparator(sdpa::daemon::WorkerManager* ptrWorkerMan)
  {
    m_ptrWorkerMan = ptrWorkerMan;
  }

  bool operator() (sdpa::worker_id_t widLeft, sdpa::worker_id_t widRight)
  {
    Worker::ptr_t ptrWorkerL = m_ptrWorkerMan->findWorker(widLeft);
    Worker::ptr_t ptrWorkerR = m_ptrWorkerMan->findWorker(widRight);

    return (ptrWorkerL->lastTimeServed( )< ptrWorkerR->lastTimeServed() );
  }

private:
  sdpa::daemon::WorkerManager* m_ptrWorkerMan;
};

void WorkerManager::getListNotFullWorkers(sdpa::worker_id_list_t& workerList)
{
  lock_type lock(mtx_);
  for( worker_map_t::iterator iter = worker_map_.begin(); iter != worker_map_.end(); iter++ )
  {
    Worker::ptr_t ptrWorker = iter->second;
    if( !ptrWorker->capacity()
     || ptrWorker->nbAllocatedJobs()<ptrWorker->capacity()
      )
    	workerList.push_back(ptrWorker->name());
  }

  CComparator comparator(this);
  workerList.sort (comparator);
}

void WorkerManager::getListWorkersNotReserved(sdpa::worker_id_list_t& workerList)
{
  lock_type lock(mtx_);
  for( worker_map_t::iterator iter = worker_map_.begin(); iter != worker_map_.end(); iter++ )
  {
    Worker::ptr_t ptrWorker = iter->second;
    if( !ptrWorker->isReserved() )
    	workerList.push_back(ptrWorker->name());
  }

  CComparator comparator(this);
  workerList.sort (comparator);
}

bool WorkerManager::addCapabilities(const sdpa::worker_id_t& worker_id, const sdpa::capabilities_set_t& cpbSet)
{
  return findWorker (worker_id)->addCapabilities (cpbSet);
}

void WorkerManager::removeCapabilities(const sdpa::worker_id_t& worker_id, const sdpa::capabilities_set_t& TCpbSet) throw (WorkerNotFoundException)
{
  findWorker (worker_id)->removeCapabilities (TCpbSet);
}

void WorkerManager::getCapabilities(const std::string& agentName, sdpa::capabilities_set_t& agentCpbSet)
{
  lock_type lock(mtx_);

  BOOST_FOREACH (Worker::ptr_t worker, worker_map_ | boost::adaptors::map_values)
  {
    BOOST_FOREACH (sdpa::capability_t capability, worker->capabilities())
    {
      const sdpa::capabilities_set_t::iterator itag_cpbs
        (agentCpbSet.find (capability));
      if (itag_cpbs == agentCpbSet.end())
      {
        agentCpbSet.insert (capability);
      }
      else if (itag_cpbs->depth() > capability.depth())
      {
        agentCpbSet.erase (itag_cpbs);
        agentCpbSet.insert (capability);
      }
    }
  }
}

namespace
{
  boost::optional<std::size_t> matchRequirements
    (const Worker::ptr_t& pWorker, const job_requirements_t& job_req_set)
  {
    std::size_t matchingDeg (0);

    BOOST_FOREACH (we::type::requirement_t req, job_req_set.getReqList())
    {
      if (pWorker->hasCapability (req.value()))
      {
        ++matchingDeg;
      }
      else if (req.is_mandatory())
      {
        return boost::none;
      }
    }

    return matchingDeg;
  }
}

sdpa::worker_id_t WorkerManager::getBestMatchingWorker
  (const job_requirements_t& listJobReq, const sdpa::worker_id_list_t& workerList) const
{
  lock_type lock(mtx_);

  if (worker_map_.empty())
    throw NoWorkerFoundException();

  boost::optional<sdpa::util::time_type> last_schedule_time;
  boost::optional<worker_id_t> bestMatchingWorkerId;
  boost::optional<std::size_t> maxMatchingDeg;

  BOOST_FOREACH (sdpa::worker_id_t workerId, workerList)
  {
    const Worker::ptr_t pWorker (worker_map_.at (workerId));
    if (pWorker->disconnected())
      continue;

    const boost::optional<std::size_t> matchingDeg
      (matchRequirements (pWorker, listJobReq));

    if ( matchingDeg < maxMatchingDeg
      || ( matchingDeg == maxMatchingDeg
        && pWorker->lastScheduleTime() >= last_schedule_time
         )
       )
    {
      continue;
    }

    maxMatchingDeg = matchingDeg;
    bestMatchingWorkerId = workerId;
    last_schedule_time = pWorker->lastScheduleTime();
  }

  return maxMatchingDeg
    ? *bestMatchingWorkerId
    : throw NoWorkerFoundException();
}

void WorkerManager::reserveWorker(const sdpa::worker_id_t& worker_id)
{
  findWorker (worker_id)->reserve();
}

void addToList(Worker::JobQueue* pQueue, sdpa::job_id_list_t& jobList)
{
  while( !pQueue->empty() )
  {
      sdpa::job_id_t jobId = pQueue->pop();
      jobList.push_back(jobId);
  }
}

sdpa::job_id_list_t WorkerManager::getJobListAndCleanQueues(const Worker::ptr_t& pWorker)
{
  lock_type lock(mtx_);
  sdpa::job_id_list_t listAssignedJobs;

  addToList(&pWorker->submitted(), listAssignedJobs);
  addToList(&pWorker->acknowledged(), listAssignedJobs);

  return listAssignedJobs;
}
