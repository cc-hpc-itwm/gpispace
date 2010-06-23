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

using namespace std;
using namespace sdpa::daemon;

WorkerManager::WorkerManager(): SDPA_INIT_LOGGER("sdpa::daemon::WorkerManager")
{
	iter_last_worker_ = worker_map_.begin();
}

WorkerManager::~WorkerManager(){

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
 * add new worker
 */
void WorkerManager::addWorker(const Worker::ptr_t &pWorker) throw (WorkerAlreadyExistException)
{
	lock_type lock(mtx_);

	Worker::worker_id_t workerId = pWorker->name();
	const int rank = pWorker->rank();

	for( worker_map_t::iterator it = worker_map_.begin(); it != worker_map_.end(); it++ )
	{
		if( it->second->name() ==  workerId)
		{
			SDPA_LOG_ERROR("An worker with the id "<<workerId<<" already exist into the worker map!");
			throw WorkerAlreadyExistException(workerId, rank);
		}

		if( it->second->rank() == rank )
		{
			SDPA_LOG_WARN("An worker with the rank "<<rank<<" already exists in the worker map!");
			throw WorkerAlreadyExistException(workerId, rank);
		}
	}

	worker_map_.insert(pair<Worker::worker_id_t, Worker::ptr_t>(pWorker->name(),pWorker));

  balanceWorkers();
}

void WorkerManager::balanceWorkers()
{
	lock_type lock(mtx_);
	typedef std::map<Worker::worker_id_t, unsigned int> load_map_t;
	typedef pair<Worker::worker_id_t, unsigned int> loadPair;
	load_map_t loadVector;

	size_t loadBal = 0;
	const size_t N = worker_map_.size();

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
 * get next worker to be served
 */
Worker::ptr_t& WorkerManager::getNextWorker() throw (NoWorkerFoundException)
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
