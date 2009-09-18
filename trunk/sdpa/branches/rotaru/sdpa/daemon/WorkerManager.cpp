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
	worker_map_t::iterator it = worker_map_.find(worker_id);
	if( it != worker_map_.end() )
		return it->second;
	else
		throw WorkerNotFoundException(worker_id);
}

/**
 * add new worker
 */
void WorkerManager::addWorker(const Worker::ptr_t &pWorker)
{
	worker_map_[pWorker->name()] = pWorker;
}

/**
 * get next worker to be served
 */
Worker::ptr_t &WorkerManager::getNextWorker() throw (NoWorkerFoundException)
{
	if( worker_map_.empty() )
		throw NoWorkerFoundException();

	if(iter_last_worker_ != worker_map_.end())
		iter_last_worker_++;
	else
		iter_last_worker_= worker_map_.begin();

	return iter_last_worker_->second;
}
