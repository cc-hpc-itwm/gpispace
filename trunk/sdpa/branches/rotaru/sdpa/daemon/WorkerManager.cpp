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
	iter_last_worker_ = worker_map_.end();
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
	unsigned int rank = pWorker->rank();

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

	worker_map_.insert(pair<Worker::worker_id_t, Worker::ptr_t>(pWorker->name(),pWorker));
	if(worker_map_.size() == 1)
		iter_last_worker_ = worker_map_.begin();

	//redistribute fairly the load among the workers, if necessary
}

/**
 * get next worker to be served
 */
Worker::ptr_t &WorkerManager::getNextWorker() throw (NoWorkerFoundException)
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
