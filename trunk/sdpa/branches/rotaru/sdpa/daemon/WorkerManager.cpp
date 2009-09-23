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
void WorkerManager::addWorker(const Worker::ptr_t &pWorker)
{
	lock_type lock(mtx_);
	worker_map_.insert(pair<Worker::worker_id_t, Worker::ptr_t>(pWorker->name(),pWorker));
	if(worker_map_.size() == 1)
		iter_last_worker_ = worker_map_.begin();
}

/**
 * get next worker to be served
 */
Worker::ptr_t &WorkerManager::getNextWorker() throw (NoWorkerFoundException)
{
	lock_type lock(mtx_);

	if( worker_map_.empty() )
		throw NoWorkerFoundException();

	if(iter_last_worker_ != worker_map_.end())
		return iter_last_worker_++->second;
	else
		return (iter_last_worker_= worker_map_.begin())->second;
}
