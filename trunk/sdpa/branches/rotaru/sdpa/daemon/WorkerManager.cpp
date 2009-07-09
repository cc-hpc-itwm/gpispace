#include <sdpa/daemon/WorkerManager.hpp>


using namespace std;
using namespace sdpa::daemon;

WorkerManager::WorkerManager(): SDPA_INIT_LOGGER("sdpa::daemon::WorkerManager")  {

}

WorkerManager::~WorkerManager(){

}

Worker::ptr_t WorkerManager::findWorker(const Worker::worker_id_t& worker_id ) throw(WorkerNotFoundException)
{
	worker_map_t::iterator it = worker_map_.find(worker_id);
	if( it != worker_map_.end() )
		return it->second;
	else
		throw WorkerNotFoundException(worker_id);
}
