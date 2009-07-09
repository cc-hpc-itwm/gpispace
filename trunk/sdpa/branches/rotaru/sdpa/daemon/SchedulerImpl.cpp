#include <sdpa/daemon/SchedulerImpl.hpp>

using namespace sdpa::daemon;

SchedulerImpl::SchedulerImpl(const  JobManager::ptr_t p_job_man, const WorkerManager::ptr_t p_worker_man)
{
	ptr_job_man_ 	= p_job_man;
    ptr_worker_man_ = p_worker_man;
}

SchedulerImpl::~SchedulerImpl()
{

}


void SchedulerImpl::acknowledge(const sdpa::job_id_t& job_id ) {

}

Job::ptr_t SchedulerImpl::get_next_job(const Worker::worker_id_t &worker_id, const sdpa::job_id_t &last_job) {

}

void SchedulerImpl::schedule_local(const Job::ptr_t &job) {

}

void SchedulerImpl::schedule(const Job::ptr_t &job) {

}

