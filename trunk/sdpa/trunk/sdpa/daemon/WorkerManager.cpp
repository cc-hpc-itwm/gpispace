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
#include <algorithm>

using namespace std;
using namespace sdpa::daemon;

WorkerManager::WorkerManager(): SDPA_INIT_LOGGER("sdpa::daemon::WorkerManager")
{
	iter_last_worker_ = worker_map_.end();
}

WorkerManager::~WorkerManager()
{

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
void WorkerManager::addWorker( const Worker::worker_id_t& workerId, unsigned int rank ) throw (WorkerAlreadyExistException)
{
	lock_type lock(mtx_);

	bool bFound = false;
	for( worker_map_t::iterator it = worker_map_.begin(); !bFound && it != worker_map_.end(); it++ )
	{
		if( it->second->name() ==  workerId)
		{
			SDPA_LOG_ERROR("An worker with the id "<<workerId<<" already exist into the worker map!");
			bFound = true;
			throw WorkerAlreadyExistException(workerId, rank);
		}

		if( it->second->rank() == rank )
		{
			SDPA_LOG_ERROR("An worker with the rank"<<rank<<" already exist into the worker map!");
			bFound = true;
			throw WorkerAlreadyExistException(workerId, rank);
		}
	}

	Worker::ptr_t pWorker( new Worker( workerId, rank ));

	worker_map_.insert(pair<Worker::worker_id_t, Worker::ptr_t>(pWorker->name(), pWorker));
	rank_map_.insert( pair<unsigned int, Worker::worker_id_t>(rank, pWorker->name()) );

	if(worker_map_.size() == 1)
		iter_last_worker_ = worker_map_.begin();
}

// you should here delete_worker as well, for the
// case when the workers unregisters

void WorkerManager::balanceWorkers()
{
	lock_type lock(mtx_);
	typedef std::map<Worker::worker_id_t, unsigned int> load_map_t;
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
							for( int k=0; k<delta; k++)
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
		return a.second->pending().size() < a.second->pending().size();
	}
};


/**
 * get the least loaded worker
 */
unsigned int WorkerManager::getLeastLoadedWorker() throw (NoWorkerFoundException)
{
	SDPA_LOG_DEBUG("Get the least loaded worker ...");

	lock_type lock(mtx_);

	if( worker_map_.empty() )
		throw NoWorkerFoundException();

	worker_map_t::iterator it = std::min_element(worker_map_.begin(), worker_map_.end(), compare_workers());
	unsigned int rank_ll = it->second->rank();


	return rank_ll;
}

const sdpa::job_id_t WorkerManager::getNextJob(const Worker::worker_id_t& worker_id,  const sdpa::job_id_t &last_job_id) throw (NoJobScheduledException)
{
	SDPA_LOG_DEBUG("Get the next job ...");

	sdpa::job_id_t jobId;
	Worker::ptr_t ptrWorker = findWorker(worker_id);
	ptrWorker->update();

	// look first into the worker's queue
	try {
		jobId = ptrWorker->get_next_job(last_job_id);
		return jobId;
	}
	catch(const NoJobScheduledException& ex)
	{
		// if there is a job into the common queue serve it
		try {
			jobId = common_queue_.pop();
			ptrWorker->submitted().push(jobId);
			return jobId;
		}
		catch(const QueueEmpty& ex)
		{
			// throw an exception if not possible
			throw NoJobScheduledException(worker_id);
		}
	}
}

void WorkerManager::dispatchJob(const sdpa::job_id_t& jobId)
{
	SDPA_LOG_DEBUG("appending job(" << jobId.str() << ") to the common queue");
	common_queue_.push(jobId);
}


