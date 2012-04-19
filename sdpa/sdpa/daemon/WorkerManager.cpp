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
#include <sdpa/daemon/WorkerManager.hpp>
#include <boost/unordered_map.hpp>
#include <algorithm>
#include <sdpa/types.hpp>
#include <boost/foreach.hpp>
#include "boost/bind.hpp"
#include <sdpa/daemon/IComm.hpp>

#include <algorithm>

using namespace std;
using namespace sdpa::daemon;

WorkerManager::WorkerManager(): SDPA_INIT_LOGGER("sdpa::daemon::WorkerManager")
{
	lock_type lock(mtx_);
	iter_last_worker_ = worker_map_.end();
}

WorkerManager::~WorkerManager()
{
	lock_type lock(mtx_);

	SDPA_LOG_DEBUG( "WorkerManager shutting down...");
	if( worker_map_.size() )
	{
		SDPA_LOG_WARN( "there are still entries left in the worker map: " << worker_map_.size());
	}

	if( common_queue_.size() )
	{
		SDPA_LOG_WARN( "there are still entries left in the common queue: " << common_queue_.size());
	}
}

/**
 * find worker
 */
Worker::ptr_t &WorkerManager::findWorker(const Worker::worker_id_t& worker_id ) throw(WorkerNotFoundException)
{
  lock_type lock(mtx_);
  worker_map_t::iterator it = worker_map_.find(worker_id);
  if( it != worker_map_.end() )
    return it->second;
  else
    throw WorkerNotFoundException(worker_id);
}

/**
 * find worker
 */
const Worker::worker_id_t &WorkerManager::findWorker(const sdpa::job_id_t& job_id) throw (NoWorkerFoundException)
{
  lock_type lock(mtx_);

  for( worker_map_t::iterator it = worker_map_.begin(); it!= worker_map_.end(); it++ )
    if( it->second->has_job(job_id) )
      return  it->second->name();

  throw NoWorkerFoundException();
}

const Worker::worker_id_t&
WorkerManager::findAcknowlegedWorker(const sdpa::job_id_t& job_id) throw (NoWorkerFoundException)
{
  lock_type lock(mtx_);

  for (worker_map_t::const_iterator it = worker_map_.begin(); it!= worker_map_.end(); ++it)
    if( it->second->is_job_acknowleged(job_id))
      return it->second->name();

  throw NoWorkerFoundException();
}

/**
 * add new worker
 */
void WorkerManager::addWorker( 	const Worker::worker_id_t& workerId,
								unsigned int capacity,
								const capabilities_set_t& cpbSet,
								const unsigned int& agent_rank,
								const sdpa::worker_id_t& agent_uuid ) throw (WorkerAlreadyExistException)
{
	lock_type lock(mtx_);

	bool bFound = false;
	for( worker_map_t::iterator it = worker_map_.begin(); !bFound && it != worker_map_.end(); it++ )
	{
		//if( it->second->name() ==  workerId )
		if( it->first ==  workerId )
		{
			//SDPA_LOG_ERROR("An worker with the id "<<workerId<<" already exist into the worker map!");
			bFound = true;
			throw WorkerAlreadyExistException(workerId, agent_uuid);
		}
	}

	// add TCpbSet HERE !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	Worker::ptr_t pWorker( new Worker( workerId, capacity, agent_rank, agent_uuid ) );
	pWorker->addCapabilities(cpbSet);

	worker_map_.insert(worker_map_t::value_type(pWorker->name(), pWorker));

	SDPA_LOG_INFO( "Created new worker: name = "<<pWorker->name()<<" with rank = "<<pWorker->rank()<<" and capacity = "<<pWorker->capacity() );

	if(worker_map_.size() == 1)
		iter_last_worker_ = worker_map_.begin();
}

void WorkerManager::balanceWorkers()
{
  lock_type lock(mtx_);
  typedef boost::unordered_map<Worker::worker_id_t, unsigned int> load_map_t;
  typedef pair<Worker::worker_id_t, unsigned int> loadPair;
  load_map_t loadVector;

  size_t loadBal = 0;
  size_t N = worker_map_.size();

  if( N==0 )
    return;

  for( worker_map_t::iterator it = worker_map_.begin(); it!=worker_map_.end(); it++)
    loadBal += it->second->pending().size();

  loadBal = loadBal%N?loadBal/N:loadBal/N + 1;

  bool bFinished = false;

  while( !bFinished )
  {
    bFinished = true;;
    for( worker_map_t::iterator it = worker_map_.begin(); it!=worker_map_.end(); it++ )
    {
      size_t loadCurrNode = it->second->pending().size();

      if( loadCurrNode > loadBal )
      {
        for( worker_map_t::iterator itNb = worker_map_.begin(); itNb!=worker_map_.end(); itNb++)
        {
          size_t loadNb = itNb->second->pending().size();
          if( loadCurrNode > loadNb )
          {
            // transfer load = (loadCurrNode - loadNb)/N from the current node to the neighboring node
            size_t delta = (loadCurrNode - loadNb)/N;
            if(delta)
            {
              bFinished = false;
              for( size_t k=0; k<delta; k++)
              {
                // look for nodes who prefer the neighboring worker
                // if there are any, move them first and then, the nodes
                // for which no preference was expressed
                sdpa::job_id_t jobId = it->second->pending().pop();
                itNb->second->pending().push(jobId);
              }
            }
            else if( loadCurrNode - loadNb > 1 ) //still unbalanced
            {
              bFinished = false;
              loadCurrNode--;
              // move just one job
              sdpa::job_id_t jobId = it->second->pending().pop();
              itNb->second->pending().push(jobId);
            }
          }
        }
      }
    }
  }
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

/**
 * compare the workers
 */
struct compare_workers
{
  typedef WorkerManager::worker_map_t::value_type T;
  bool operator()( T const& a, T const& b)
  {
	  return a.second->nbAllocatedJobs() < b.second->nbAllocatedJobs();
  }
};

/**
 * get the least loaded worker
 */
sdpa::worker_id_t WorkerManager::getLeastLoadedWorker() throw (NoWorkerFoundException, AllWorkersFullException)
{
  //SDPA_LOG_INFO("Get the least loaded worker ...");

  lock_type lock(mtx_);

  if( worker_map_.empty() )
    throw NoWorkerFoundException();

  worker_map_t::iterator it = std::min_element(worker_map_.begin(), worker_map_.end(), compare_workers());

  if( it->second->nbAllocatedJobs() >= it->second->capacity() )
    throw AllWorkersFullException();

  return it->first;
}

/*Worker::worker_id_t WorkerManager::getOwnerId(const sdpa::job_id_t& job_id) throw (JobNotAssignedException)
{
  lock_type lock(mtx_);
  Worker::worker_id_t owner_worker_id = owner_map().at(job_id);
  return owner_worker_id;
}*/

const sdpa::job_id_t WorkerManager::stealWork(const Worker::worker_id_t& worker_id) throw (NoJobScheduledException)
{
	lock_type lock(mtx_);
  //find a job that prefers worker_id, with the highest preference degree
  const Worker::ptr_t& ptrWorker = findWorker(worker_id);

  // take the job with the lowest pref_deg (= the position into the ranks list)
  // check if it still exists into the pending queue of the owner worker,
  // steal it and become the new owner

  /*
  Worker::mi_ordered_prefs& mi_pref = get<0>(ptrWorker->mi_affinity_list);
  Worker::mi_ordered_prefs::iterator it = mi_pref.begin();

  if( it == mi_pref.end() )
    throw NoJobScheduledException(worker_id);

  while( it != mi_pref.end() )
  {
    // check if the job it->job_id_ is in the pending queue of the owner_worker_id_
    sdpa::job_id_t job_id = it->job_id_;

    try {
      Worker::worker_id_t owner_worker_id = getOwnerId(job_id);

      // delete the corresponding pair (pref_deg, job_id) from jobs_preferring_this_
      mi_pref.erase(it);

      const Worker::ptr_t& ptrOwnerWorker = findWorker(owner_worker_id);

      // look for the job into the owner worker's pending queue and steal the job if it is there
      Worker::JobQueue::lock_type lockQ( ptrOwnerWorker->pending().mutex() );
      for ( Worker::JobQueue::iterator iter = ptrOwnerWorker->pending().begin(); iter != ptrOwnerWorker->pending().end(); iter++ )
      {
        if( job_id == *iter )
        {
          // remove the job from the owner's queue and become the new owner
          ptrOwnerWorker->pending().erase(iter);

          // become owner
          make_owner(job_id, worker_id);

          // take the job and return job_id_
          return job_id;
        }
      }

      // if not, continue with the next preferences (sorted)
      it = mi_pref.begin();
    }
    catch (JobNotAssignedException)
    {
      // ignore this preference
      mi_pref.erase(it);
    }
  }

  */

  // erase the job from
  throw NoJobScheduledException(worker_id);
}

const sdpa::job_id_t WorkerManager::getNextJob(const Worker::worker_id_t& worker_id, const sdpa::job_id_t &last_job_id) throw (NoJobScheduledException, WorkerNotFoundException)
{
  //SDPA_LOG_DEBUG("Get the next job ...");

	lock_type lock(mtx_);
  sdpa::job_id_t jobId;

  try {
    const Worker::ptr_t& ptrWorker = findWorker(worker_id);
    //ptrWorker->update();

    // look first into the worker's queue
    try {
        jobId = ptrWorker->get_next_job(last_job_id);

        SDPA_LOG_INFO("The worker "<<worker_id<<" has a capacity of "<<ptrWorker->capacity()<<" jobs and has "<<ptrWorker->nbAllocatedJobs()<<" jobs allocated!");

        return jobId;
    }
    catch(const NoJobScheduledException& ex)
    {
      try {

        //SDPA_LOG_INFO("Try to get a job from the common queue");

        /*SDPA_LOG_DEBUG("The content of the common queue is: ");
        common_queue_.print();*/

        jobId = common_queue_.pop();

        /*SDPA_LOG_DEBUG("Popped the job "<<jobId<<"The content of the common queue is now: ");
        common_queue_.print();*/

        DMLOG( TRACE
             , "Putting job "
             << jobId
             << " into the submitted queue of the worker "
             << worker_id
             );
        ptrWorker->submitted().push(jobId);
        ptrWorker->update();

        return jobId;
      }
      catch(const QueueEmpty& ex0)
      {
        // try to steal some work from other workers
        // if not possible, throw an exception
        try {

          //SDPA_LOG_INFO("Try to steal work from another worker ...");
          jobId = stealWork(worker_id);
          ptrWorker->submitted().push(jobId);

          return jobId;
        }
        catch( const NoJobScheduledException& )
        {
          //SDPA_LOG_INFO("There is really no job to assign/steal for the worker "<<worker_id<<"  ...");
          throw;
        }
      }
    }
  }
  catch(const WorkerNotFoundException& ex2 )
  {
      SDPA_LOG_WARN("Worker not found!");
      throw ex2;
  }
}

void WorkerManager::dispatchJob(const sdpa::job_id_t& jobId)
{
	lock_type lock(mtx_);
	DLOG(TRACE, "Dispatch the job " << jobId.str() );
	common_queue_.push(jobId);
}

void WorkerManager::delete_job (sdpa::job_id_t const & job)
{
	lock_type lock(mtx_);
	if (common_queue_.erase(job))
	{
		SDPA_LOG_DEBUG("removed job from the central queue...");
	}
	else
	{
		for( worker_map_t::iterator iter (worker_map_.begin()); iter != worker_map_.end(); iter++ )
		{
			iter->second->delete_job(job);
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
		ptrWorker->delete_job(job_id);
	}
	catch(JobNotDeletedException const &) {
		SDPA_LOG_ERROR("Could not delete the job "<<job_id.str()<<"!");
	}
	catch(WorkerNotFoundException const &) {
		SDPA_LOG_ERROR("Worker "<<worker_id<<" not found!");
	}
}

void WorkerManager::delWorker( const Worker::worker_id_t& workerId ) throw (WorkerNotFoundException)
{
  lock_type lock(mtx_);
  worker_map_t::iterator w (worker_map_.find (workerId));

  if (w == worker_map_.end())
    throw WorkerNotFoundException(workerId);

  worker_map_.erase (w);
}

bool WorkerManager::has_job(const sdpa::job_id_t& job_id)
{
  lock_type lock(mtx_);
  if( common_queue_.find(job_id) != common_queue_.end() )
  {
    SDPA_LOG_DEBUG( "The job " << job_id<<" is in the common queue" );
    return true;
  }

  for( worker_map_t::iterator iter = worker_map_.begin(); iter != worker_map_.end(); iter++ )
    if(iter->second->has_job( job_id ))
    {
      SDPA_LOG_DEBUG( "The job " << job_id<<" is already assigned to the worker "<<iter->first );
      return true;
    }

  return false;
}

void WorkerManager::getWorkerList(std::list<std::string>& workerList)
{
  lock_type lock(mtx_);
  for( worker_map_t::iterator iter = worker_map_.begin(); iter != worker_map_.end(); iter++ )
    workerList.push_back(iter->second->name());
}

void WorkerManager::setLastTimeServed(const sdpa::worker_id_t& workerId, const sdpa::util::time_type& last_time_srv )
{
	 lock_type lock(mtx_);
	try {
		Worker::ptr_t ptrWorker = findWorker(workerId);
		ptrWorker->setLastTimeServed(last_time_srv);
	}
	catch( const WorkerNotFoundException& exc)
	{
		  SDPA_LOG_WARN( "Couldn't update the last service time for the worker "<<workerId );
	}
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

void WorkerManager::getWorkerListNotFull(sdpa::worker_id_list_t& workerList)
{
	lock_type lock(mtx_);
	for( worker_map_t::iterator iter = worker_map_.begin(); iter != worker_map_.end(); iter++ )
	{
		Worker::ptr_t ptrWorker = iter->second;
		if( !(ptrWorker->pending().empty() && common_queue_.empty()) &&
			ptrWorker->nbAllocatedJobs()<ptrWorker->capacity())
				workerList.push_back(ptrWorker->name());
	}

	CComparator comparator(this);
	sort(workerList.begin(), workerList.end(), comparator);
}

bool WorkerManager::addCapabilities(const sdpa::worker_id_t& worker_id, const sdpa::capabilities_set_t& cpbSet)
{
	lock_type lock(mtx_);
	worker_map_t::iterator it = worker_map_.find(worker_id);
	if( it != worker_map_.end() )
	{
		return it->second->addCapabilities(cpbSet);
	}
	else
		throw WorkerNotFoundException(worker_id);
}

void WorkerManager::removeCapabilities(const sdpa::worker_id_t& worker_id, const sdpa::capabilities_set_t& TCpbSet) throw (WorkerNotFoundException)
{
	lock_type lock(mtx_);
	worker_map_t::iterator it = worker_map_.find(worker_id);
	if( it != worker_map_.end() )
	{
		it->second->removeCapabilities(TCpbSet);
	}
	else
		throw WorkerNotFoundException(worker_id);
}

bool hasSameName(sdpa::capability_t& cpb1, sdpa::capability_t& cpb2)
{
	return (cpb1.name() == cpb2.name()) && (cpb1.type() == cpb2.type());
}

void WorkerManager::getCapabilities(const std::string& agentName, sdpa::capabilities_set_t& agentCpbSet)
{
	lock_type lock(mtx_);

	for( worker_map_t::iterator it_worker = worker_map_.begin(); it_worker != worker_map_.end(); it_worker++ )
	{
		sdpa::capabilities_set_t workerCpbSet = it_worker->second->capabilities();
		for(sdpa::capabilities_set_t::iterator itw_cpbs = workerCpbSet.begin(); itw_cpbs != workerCpbSet.end();  itw_cpbs++  )
		{
			sdpa::capabilities_set_t::iterator itag_cpbs = agentCpbSet.find(*itw_cpbs);
			if(itag_cpbs == agentCpbSet.end())
				agentCpbSet.insert(*itw_cpbs);

			else
				if(itag_cpbs->depth() > itw_cpbs->depth())
				{
					const_cast<sdpa::capability_t&>(*itag_cpbs).setDepth(itw_cpbs->depth());
				}
		}
	}
}

template <typename TPtrWorker, typename TReqSet>
int matchRequirements( const TPtrWorker& pWorker, const TReqSet job_req_set, bool bOwn = false )
{
	int matchingDeg = 0;

	// for all job requirements
	for( typename TReqSet::const_iterator it = job_req_set.begin(); it != job_req_set.end(); it++ )
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
				//LOG(ERROR, "The worker "<<pWorker->name()<<" doesn't have the required capability: "<<it->value()<<"!");
				//std:cout<<pWorker->capabilities();
				return 0;
			}
	}

	return matchingDeg;
}

Worker::ptr_t WorkerManager::getBestMatchingWorker( const requirement_list_t& listJobReq ) throw (NoWorkerFoundException)
{
	lock_type lock(mtx_);
	if( worker_map_.empty() )
          throw NoWorkerFoundException();

	int maxMatchingDeg = 0;
	sdpa::util::time_type last_schedule_time = sdpa::util::now();

	// the worker id of the worker that fulfills most of the requirements
	// a matching degree 0 means that either at least a mandatory requirement
	// is not fulfilled or the worker does not have at all that capability
	worker_id_t bestMatchingWorkerId;

	BOOST_FOREACH( worker_map_t::value_type& pair, worker_map_ )
	{
		Worker::ptr_t pWorker = pair.second;

		if( !pWorker->disconnected() ) // if the worker is disconnected, skip it!
		{
			int matchingDeg = matchRequirements( pair.second, listJobReq, true ); // only proper capabilities of the worker
			if( matchingDeg > maxMatchingDeg || ( matchingDeg == maxMatchingDeg && pWorker->lastScheduleTime()<last_schedule_time ) )
			{
				maxMatchingDeg = matchingDeg;
				bestMatchingWorkerId = pair.first;
				last_schedule_time = pWorker->lastScheduleTime();
			}
		}
	}

	if(maxMatchingDeg != 0)
		return worker_map_[bestMatchingWorkerId];
	else
	{
		maxMatchingDeg = 0;
		BOOST_FOREACH( worker_map_t::value_type& pair, worker_map_ )
		{
			Worker::ptr_t pWorker = pair.second;

			if( !pWorker->disconnected() ) // if the worker is disconnected, skip it!
			{
				int matchingDeg = matchRequirements( pair.second, listJobReq, false ); // aggregated capabilities of the worker
				if( matchingDeg > maxMatchingDeg || ( matchingDeg == maxMatchingDeg && pWorker->lastScheduleTime()<last_schedule_time ) )
				{
					maxMatchingDeg = matchingDeg;
					bestMatchingWorkerId = pair.first;
					last_schedule_time = pWorker->lastScheduleTime();
				}
			}
		}

		if(maxMatchingDeg != 0)
			return worker_map_[bestMatchingWorkerId];
		else
                  throw NoWorkerFoundException();
	}
}

void WorkerManager::cancelWorkerJobs(sdpa::daemon::Scheduler* ptrSched)
{
	lock_type lock(mtx_);
	BOOST_FOREACH( worker_map_t::value_type& pair, worker_map_ )
	{
		worker_id_t workerId = pair.first;
		Worker::ptr_t pWorker = pair.second;
		pWorker->pending().clear();

		JobId jobId;

		while(!pWorker->submitted().empty())
		{
			jobId = pWorker->submitted().pop();
			ptrSched->planForCancellation(workerId, jobId);
		}
		// for all jobs that are submitted or acknowledged

		while(!pWorker->acknowledged().empty())
		{
			jobId = pWorker->acknowledged().pop();
			ptrSched->planForCancellation(workerId, jobId);
		}
	}
}

Worker::worker_id_t WorkerManager::getWorkerId(unsigned int r)
{
	lock_type lock(mtx_);
	BOOST_FOREACH( worker_map_t::value_type& pair, worker_map_ )
	{
		worker_id_t workerId  = pair.first;
		Worker::ptr_t pWorker = pair.second;
		if(pWorker->rank() == r)
			return workerId;
	}

	return "";
}

void WorkerManager::removeWorkers()
{
	lock_type lock(mtx_);
	common_queue_.clear();
	worker_map_.clear();
}
