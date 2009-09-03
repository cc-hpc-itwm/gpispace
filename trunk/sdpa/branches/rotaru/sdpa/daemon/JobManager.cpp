#include <sdpa/daemon/JobManager.hpp>


using namespace std;
using namespace sdpa::daemon;

JobManager::JobManager(): SDPA_INIT_LOGGER("sdpa::daemon::JobManager")  {

}

JobManager::~JobManager(){

}

//helpers
Job::ptr_t JobManager::findJob(const sdpa::job_id_t& job_id ) throw(JobNotFoundException)
{
	job_map_t::iterator it = job_map_.find( job_id );
	if( it != job_map_.end() )
		return it->second;
	else
		throw JobNotFoundException( job_id );
}

//helpers
Job::ptr_t JobManager::getJob()
{
	Job::ptr_t ptrJob;
	ptrJob.reset();

	//find a job that is into the Pending state. Leave it there don't erase it!
	if(!job_map_.empty())
	{
		job_map_t::iterator it = job_map_.begin();
		ptrJob = it->second;
	}

	return ptrJob;
}


void JobManager::addJob(const sdpa::job_id_t& job_id, const Job::ptr_t& pJob) throw(JobNotAddedException)
{
	ostringstream os;
	job_map_t::iterator it;
	bool bsucc = false;

	pair<job_map_t::iterator, bool> ret_pair(it, bsucc);
	pair<sdpa::job_id_t, Job::ptr_t> job_pair(job_id, pJob);

	ret_pair =  job_map_.insert(job_pair);

	if(ret_pair.second)
	{
		os<<"Inserted job "<<job_id<<" into the job map"<<std::endl;
		SDPA_LOG_DEBUG(os.str());
	}
	else
		throw JobNotAddedException(job_id);
}

void JobManager::markJobForDeletion(const sdpa::job_id_t& job_id, const Job::ptr_t& pJob) throw(JobNotMarkedException)
{
	ostringstream os;
	job_map_t::iterator it;
	bool bsucc = false;

	pair<job_map_t::iterator, bool> ret_pair(it, bsucc);
	pair<sdpa::job_id_t, Job::ptr_t> job_pair(job_id, pJob);

	ret_pair =  job_map_marked_for_del_.insert(job_pair);

	if(ret_pair.second)
	{
		os<<"Marked job "<<job_id<<" for deletion"<<std::endl;
		SDPA_LOG_DEBUG(os.str());
	}
	else
		throw JobNotAddedException(job_id);
}

void JobManager::deleteJob(const sdpa::job_id_t& job_id) throw(JobNotDeletedException)
{
	ostringstream os;
	job_map_t::size_type ret = job_map_.erase(job_id);
	if( !ret )
		throw JobNotDeletedException(job_id);
	else
	{
		os<<"Erased job "<<job_id<<" from job map"<<std::endl;
		SDPA_LOG_DEBUG(os.str());
	}
}

std::vector<sdpa::job_id_t> JobManager::getJobIDList()
{
	std::vector<sdpa::job_id_t> v;
	for(job_map_t::iterator it = job_map_.begin(); it!= job_map_.end(); it++)
		v.push_back(it->first);

	return v;
}
