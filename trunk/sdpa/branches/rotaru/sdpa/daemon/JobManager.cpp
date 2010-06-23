/*
 * =====================================================================================
 *
 *       Filename:  JobManager.cpp
 *
 *    Description:  Job manager
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
#include <sdpa/daemon/JobManager.hpp>

#include <iostream>
#include <fstream>
#include <string>

#include <boost/serialization/base_object.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/list.hpp>
#include <boost/serialization/map.hpp>

using namespace std;
using namespace sdpa::daemon;

JobManager::JobManager(): SDPA_INIT_LOGGER("sdpa::daemon::JobManager")  {

}

JobManager::~JobManager(){

}


//helpers
Job::ptr_t& JobManager::findJob(const sdpa::job_id_t& job_id ) throw(JobNotFoundException)
{
	lock_type lock(mtx_);
	job_map_t::iterator it = job_map_.find( job_id );
	if( it != job_map_.end() )
		return it->second;
	else
		throw JobNotFoundException( job_id );
}

/*
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
}*/


void JobManager::addJob(const sdpa::job_id_t& job_id, const Job::ptr_t& pJob) throw(JobNotAddedException)
{
	lock_type lock(mtx_);
	ostringstream os;
	job_map_t::iterator it;
	bool bsucc = false;

	pair<job_map_t::iterator, bool> ret_pair(it, bsucc);
	pair<sdpa::job_id_t, Job::ptr_t> job_pair(job_id, pJob);

	ret_pair =  job_map_.insert(job_pair);

	if(ret_pair.second)
	{
		os<<"Inserted job "<<job_id<<" into the job map";
		SDPA_LOG_DEBUG(os.str());
	}
	else
		throw JobNotAddedException(job_id);
}

void JobManager::markJobForDeletion(const sdpa::job_id_t& job_id, const Job::ptr_t& pJob) throw(JobNotMarkedException)
{
	lock_type lock(mtx_);
	ostringstream os;
	job_map_t::iterator it;
	bool bsucc = false;

	pair<job_map_t::iterator, bool> ret_pair(it, bsucc);
	pair<sdpa::job_id_t, Job::ptr_t> job_pair(job_id, pJob);

	ret_pair =  job_map_marked_for_del_.insert(job_pair);

	if(ret_pair.second)
	{
		os<<"Marked job "<<job_id<<" for deletion";
		SDPA_LOG_DEBUG(os.str());
	}
	else
		throw JobNotAddedException(job_id);
}

void JobManager::deleteJob(const sdpa::job_id_t& job_id) throw(JobNotDeletedException)
{
	lock_type lock(mtx_);
	ostringstream os;

	// delete the preferences
	preference_map_t::size_type rc = job_preferences_.erase(job_id);
	if( !rc )
		SDPA_LOG_WARN("The job "<<job_id.str()<<" had no preferences");
	else
		SDPA_LOG_DEBUG("Erased the preferences of the job "<<job_id.str());

	job_map_t::size_type ret = job_map_.erase(job_id);
	if( !ret )
		throw JobNotDeletedException(job_id);
	else
		SDPA_LOG_DEBUG("Erased job "<<job_id.str()<<" from job map");
}

std::vector<sdpa::job_id_t> JobManager::getJobIDList()
{
	std::vector<sdpa::job_id_t> v;
	for(job_map_t::iterator it = job_map_.begin(); it!= job_map_.end(); it++)
		v.push_back(it->first);

	return v;
}

std::string JobManager::print()
{
	//lock_type lock(mtx_);
	std::ostringstream os;

	os<<"Begin dump ..."<<std::endl;

	if(begin() != end())
	{
		os<<"The list of jobs still owned by the JobManager:"<<std::endl;
		iterator it;
		for(it = begin(); it != end(); it++)
		{
			os<<"       -> job "<<(*it).second->id()<<std::endl;
		}
	}
	else
		os<<"No job left to the JobManager!"<<std::endl;

	os<<"End dump ..."<<std::endl;
	//retStr = cout.str();

	return os.str();
}
