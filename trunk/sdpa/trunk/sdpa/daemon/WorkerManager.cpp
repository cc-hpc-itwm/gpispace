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

using namespace std;
using namespace sdpa::daemon;

WorkerManager::WorkerManager(): SDPA_INIT_LOGGER("sdpa::daemon::WorkerManager")
{
	iter_last_worker_ = worker_map_.end();
}

WorkerManager::~WorkerManager()
{
  DLOG(TRACE, "WorkerManager shutting down...");
  LOG_IF( WARN
        , worker_map_.size()
        , "there are still entries left in the worker map: " << worker_map_.size()
        );
  LOG_IF( WARN
        , rank_map_.size()
        , "there are still entries left in the rank map: " << rank_map_.size()
        );
  LOG_IF( WARN
        , owner_map_.size()
        , "there are still entries left in the owner map: " << owner_map_.size()
        );
  LOG_IF( WARN
        , common_queue_.size()
        , "there are still entries left in the common queue: " << common_queue_.size()
        );
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

/**
 * add new worker
 */
void WorkerManager::addWorker( const Worker::worker_id_t& workerId, unsigned int rank, const sdpa::worker_id_t& agent_uuid ) throw (WorkerAlreadyExistException)
{
	lock_type lock(mtx_);

	bool bFound = false;
	for( worker_map_t::iterator it = worker_map_.begin(); !bFound && it != worker_map_.end(); it++ )
	{
		if( it->second->name() ==  workerId)
		{
			//SDPA_LOG_ERROR("An worker with the id "<<workerId<<" already exist into the worker map!");
			bFound = true;
			throw WorkerAlreadyExistException(workerId, rank, agent_uuid);
		}

		if( it->second->rank() == rank )
		{
			//SDPA_LOG_ERROR("An worker with the rank"<<rank<<" already exist into the worker map!");
			bFound = true;
			throw WorkerAlreadyExistException(workerId, rank, agent_uuid);
		}

		/*if( it->second->agent_uuid() == agent_uuid )
		{
			//SDPA_LOG_ERROR("An worker with the rank"<<rank<<" already exist into the worker map!");
			bFound = true;
			throw WorkerAlreadyExistException(workerId, rank, agent_uuid);
		}*/
	}

	Worker::ptr_t pWorker( new Worker( workerId, rank, agent_uuid ));

	worker_map_.insert(worker_map_t::value_type(pWorker->name(), pWorker));
	rank_map_.insert(rank_map_t::value_type(rank, pWorker->name()));

	if(worker_map_.size() == 1)
		iter_last_worker_ = worker_map_.begin();
}

void WorkerManager::detectTimedoutWorkers
  ( sdpa::util::time_type const & timeout
  , std::vector<Worker::worker_id_t> * timedout_workers
  )
{
  lock_type lock(mtx_);

  for( worker_map_t::iterator iter = worker_map_.begin(); iter != worker_map_.end(); iter++ )
    {
      if( sdpa::util::time_diff( iter->second->tstamp (), sdpa::util::now()) > timeout )
        {
          LOG(WARN, "Mark the timed-out workers (no incoming message for " << (timeout / 1000000) << "s!");

          iter->second->set_timedout();

          if (timedout_workers)
            {
              timedout_workers->push_back (iter->first);
            }
        }
    }
}

void WorkerManager::deleteNonResponsiveWorkers (sdpa::util::time_type const & timeout)
{
  std::vector<Worker::worker_id_t> timedout_workers;

  detectTimedoutWorkers (timeout, &timedout_workers);

  if (!timedout_workers.empty())
    {
      lock_type lock (mtx_);

      do
        {
          const Worker::worker_id_t & wid (timedout_workers.back());

          worker_map_.erase(wid);

          timedout_workers.pop_back();
        }
      while (!timedout_workers.empty());
    }
}

// you should here delete_worker as well, for the
// case when the workers unregisters

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
		return a.second->pending().size() < b.second->pending().size();
	}
};


/**
 * get the least loaded worker
 */
unsigned int WorkerManager::getLeastLoadedWorker() throw (NoWorkerFoundException)
{
    DLOG(TRACE, "Get the least loaded worker ...");

	lock_type lock(mtx_);

	if( worker_map_.empty() )
		throw NoWorkerFoundException();

	worker_map_t::iterator it = std::min_element(worker_map_.begin(), worker_map_.end(), compare_workers());
	unsigned int rank_ll = it->second->rank();

	return rank_ll;
}

Worker::worker_id_t WorkerManager::getOwnerId(const sdpa::job_id_t& job_id) throw (JobNotAssignedException)
{
	lock_type lock(mtx_);
	Worker::worker_id_t owner_worker_id = owner_map().at(job_id);
	return owner_worker_id;
}

const sdpa::job_id_t WorkerManager::stealWork(const Worker::worker_id_t& worker_id) throw (NoJobScheduledException)
{
	//find a job that prefers worker_id, with the highest preference degree
	const Worker::ptr_t& ptrWorker = findWorker(worker_id);

	// take the job with the lowest pref_deg (= the position into the ranks list)
	// check if it still exists into the pending queue of the owner worker,
	// steal it and become the new owner

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

	// erase the job from
	throw NoJobScheduledException(worker_id);
}

const sdpa::job_id_t WorkerManager::getNextJob(const Worker::worker_id_t& worker_id, const sdpa::job_id_t &last_job_id) throw (NoJobScheduledException, WorkerNotFoundException)
{
	//SDPA_LOG_DEBUG("Get the next job ...");

	sdpa::job_id_t jobId;

	try {
		const Worker::ptr_t& ptrWorker = findWorker(worker_id);
		ptrWorker->update();

		// look first into the worker's queue
		try {
			jobId = ptrWorker->get_next_job(last_job_id);

			// delete the job from the affinity list of the other workers
			deleteJobFromAllAffinityLists(jobId);

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

				MLOG( TRACE
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

					// delete the job from the affinity list of the other workers
					deleteJobFromAllAffinityLists(jobId);

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
		SDPA_LOG_ERROR("Worker not found!");
		throw ex2;
	}
}

void WorkerManager::dispatchJob(const sdpa::job_id_t& jobId)
{
	SDPA_LOG_DEBUG( "Dispatch the job " << jobId.str() );
	/*SDPA_LOG_DEBUG( "Content of the common queue before: ");
	common_queue_.print();*/
	common_queue_.push(jobId);
	/*SDPA_LOG_DEBUG( "Content of the common queue adterwards: " );
	common_queue_.print();*/
}

void WorkerManager::delete_job (sdpa::job_id_t const & job)
{
  if (common_queue_.erase(job))
  {
    LOG(TRACE, "removed job from common queue...");
  }
  else
  {
    for( worker_map_t::iterator iter (worker_map_.begin())
       ; iter != worker_map_.end()
       ; iter++
       )
    {
      iter->second->delete_job(job);
    }
  }
}

void WorkerManager::deleteWorkerJob(const Worker::worker_id_t& worker_id, const sdpa::job_id_t &job_id ) throw (JobNotDeletedException, WorkerNotFoundException)
{
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

const Worker::worker_id_t& WorkerManager::worker(unsigned int rank) throw (NoWorkerFoundException)
{
	 lock_type lock(mtx_);
	 if( rank_map().empty() )
		throw NoWorkerFoundException();

	 return rank_map().at(rank);
}

// for any worker in ranks() -> should pass as parameter a ist of workers !
void WorkerManager::deleteJobFromAllAffinityLists(const sdpa::job_id_t& job_id)
{
	lock_type lock(mtx_);
	owner_map().erase(job_id);
	for( worker_map_t::iterator iter = worker_map_.begin(); iter != worker_map_.end(); iter++ )
	{
		const Worker::ptr_t& ptrWorker = iter->second;
		ptrWorker->delete_from_affinity_list(job_id);
	}
}

void WorkerManager::delWorker( const Worker::worker_id_t& workerId ) throw (WorkerNotFoundException)
{
	lock_type lock(mtx_);
	worker_map_t::iterator w (worker_map_.find (workerId));

	if (w == worker_map_.end())
		throw WorkerNotFoundException(workerId);
	worker_map_.erase (w);

	for (rank_map_t::iterator it (rank_map_.begin()); it != rank_map_.end(); ++it)
		if (it->second == workerId)
		{
			rank_map_.erase (it);
			break;
		}
}

void WorkerManager::getListOfRegisteredRanks( std::vector<unsigned int>& v)
{
	 lock_type lock(mtx_);
	 for( worker_map_t::iterator iter = worker_map_.begin(); iter != worker_map_.end(); iter++ )
		 v.push_back(iter->second->rank());
}

void WorkerManager::make_owner(const sdpa::job_id_t& job_id, const worker_id_t& worker_id )
{
	 lock_type lock(mtx_);
	 owner_map().insert(WorkerManager::owner_map_t::value_type(job_id, worker_id));
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
