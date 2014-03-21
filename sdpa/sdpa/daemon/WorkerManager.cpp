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
#include <boost/bind.hpp>

#include <algorithm>
#include <limits>

#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/range/algorithm/count_if.hpp>

namespace sdpa
{
  namespace daemon
  {
Worker::ptr_t WorkerManager::findWorker(const worker_id_t& worker_id )
{
  boost::mutex::scoped_lock const _ (mtx_);
  worker_map_t::iterator it = worker_map_.find(worker_id);
  if( it != worker_map_.end() )
    return it->second;
  else
    throw WorkerNotFoundException();
}

bool WorkerManager::hasWorker(const worker_id_t& worker_id) const
{
  boost::mutex::scoped_lock const _ (mtx_);
  return worker_map_.find(worker_id) != worker_map_.end();
}

const boost::optional<worker_id_t> WorkerManager::findSubmOrAckWorker(const sdpa::job_id_t& job_id) const
{
  boost::mutex::scoped_lock const _ (mtx_);

  for ( Worker::ptr_t worker
      : worker_map_
      | boost::adaptors::map_values
      | boost::adaptors::filtered
          (boost::bind (&Worker::has_job, _1, job_id))
      )
  {
    return worker->name();
  }

  return boost::none;
}

bool WorkerManager::addWorker(  const worker_id_t& workerId,
                                boost::optional<unsigned int> capacity,
                                const capabilities_set_t& cpbSet )
{
  boost::mutex::scoped_lock const _ (mtx_);

  if (worker_map_.count (workerId) != 0)
  {
    return false;
  }

  Worker::ptr_t pWorker( new Worker( workerId, capacity, cpbSet ) );
  worker_map_.insert(worker_map_t::value_type(pWorker->name(), pWorker));

  return true;
}


void WorkerManager::deleteWorker( const worker_id_t& workerId )
{
  boost::mutex::scoped_lock const _ (mtx_);
  worker_map_t::iterator w (worker_map_.find (workerId));

  if (w == worker_map_.end())
    throw WorkerNotFoundException();

  worker_map_.erase (w);
}

sdpa::worker_id_list_t WorkerManager::getListWorkersNotReserved()
{
  boost::mutex::scoped_lock const _ (mtx_);
  worker_id_list_t workerList;
  for( worker_map_t::iterator iter = worker_map_.begin(); iter != worker_map_.end(); iter++ )
  {
    Worker::ptr_t ptrWorker = iter->second;
    if( !ptrWorker->isReserved() )
    	workerList.push_back(ptrWorker->name());
  }

  return workerList;
}

void WorkerManager::getCapabilities(sdpa::capabilities_set_t& agentCpbSet)
{
  boost::mutex::scoped_lock const _ (mtx_);

  for (Worker::ptr_t worker : worker_map_ | boost::adaptors::map_values)
  {
    for (sdpa::capability_t capability : worker->capabilities())
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

bool WorkerManager::checkIfAbortedJobAndDelete (const worker_id_t& workerId, const sdpa::job_id_t& jobId)
{
  boost::mutex::scoped_lock const _ (mtx_);

  worker_map_t::iterator it (worker_map_.find (workerId));

  if (it == worker_map_.end())
    throw WorkerNotFoundException();

  if (it->second->isAborted (jobId))
  {
    it->second->deleteJob (jobId);
    return true;
  }

  return false;
}

namespace
{
  boost::optional<std::size_t> matchRequirements
    (const Worker::ptr_t& pWorker, const job_requirements_t& job_req_set)
  {
    std::size_t matchingDeg (0);

    for (we::type::requirement_t req : job_req_set.getReqList())
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

boost::optional<sdpa::worker_id_t> WorkerManager::getBestMatchingWorker
  (const job_requirements_t& listJobReq, const sdpa::worker_id_list_t& workerList) const
{
  boost::mutex::scoped_lock const _ (mtx_);

  boost::optional<double> last_schedule_time;
  boost::optional<worker_id_t> bestMatchingWorkerId;
  boost::optional<std::size_t> maxMatchingDeg;

  for (sdpa::worker_id_t workerId : workerList)
  {
    const worker_map_t::const_iterator it (worker_map_.find (workerId));
    if (it == worker_map_.end())
      continue;

    Worker::ptr_t pWorker (it->second);

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

  return bestMatchingWorkerId;
}
  }
}
