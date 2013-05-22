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

#include <sdpa/daemon/IAgent.hpp>
#include <sdpa/daemon/JobImpl.hpp>
#include <boost/pointer_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/foreach.hpp>

#include <sdpa/daemon/JobFSM.hpp>

using namespace std;
using namespace sdpa::daemon;

static const std::size_t MAX_PARALLEL_JOBS = 1024;

JobManager::JobManager(const std::string& name)
	: SDPA_INIT_LOGGER(name)
{}

JobManager::~JobManager()
{
	if (job_map_.size())
	{
          DMLOG (WARN, "there are still entries left in the job-map: " << job_map_.size() );
          print();
	}

	if (job_requirements_.size() )
        {
          DMLOG (TRACE, "there are still entries left in the requirements map: " << job_requirements_.size() );
        }
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

void JobManager::addJob(const sdpa::job_id_t& job_id, const Job::ptr_t& pJob) throw(JobNotAddedException)
{
  lock_type lock(mtx_);
  job_map_t::iterator it;
  bool bsucc = false;

  pair<job_map_t::iterator, bool> ret_pair(it, bsucc);
  pair<sdpa::job_id_t, Job::ptr_t> job_pair(job_id, pJob);

  ret_pair =  job_map_.insert(job_pair);

  if(! ret_pair.second)
    throw JobNotAddedException(job_id);
}

void JobManager::deleteJob(const sdpa::job_id_t& job_id) throw(JobNotDeletedException)
{
  lock_type lock(mtx_);

  // delete the requirements of this job
  requirements_map_t::size_type rc = job_requirements_.erase(job_id);

  if(rc)
  {
    DLOG(TRACE, "Erased the requirements of the job "<<job_id.str());
  }

  job_map_t::size_type ret = job_map_.erase(job_id);
  if( !ret )
  {
    throw JobNotDeletedException(job_id);
  }
  else
  {
    DLOG(TRACE, "Erased the job "<<job_id.str()<<" from job map");
  }

  free_slot_.notify_one();
}

std::string JobManager::print() const
{
  lock_type lock(mtx_);
  std::ostringstream os;

  DMLOG (TRACE, "Begin dumping the JobManager...");

  if( job_map_.begin() == job_map_.end() )
    os<<"The JobManager is empty!";
  else
  {
    os<<"The list of jobs still owned by the JobManager:"<<std::endl;
    for ( job_map_t::const_iterator it (job_map_.begin()); it != job_map_.end(); ++it )
    {
      DMLOG (TRACE, it->second->print_info());
    }
  }

  DMLOG (TRACE, "End dumping the JobManager...");

  return os.str();
}

const requirement_list_t JobManager::getJobRequirements(const sdpa::job_id_t& jobId) const throw (NoJobRequirements)
{
  lock_type lock(mtx_);
  if( job_requirements_.empty() )
    throw NoJobRequirements(jobId);

  DLOG(TRACE, "Locate the preferences of the job "<<jobId.str());
  requirements_map_t::const_iterator it_req = job_requirements_.find(jobId);
  if( it_req == job_requirements_.end() )
    throw NoJobRequirements(jobId);

  return it_req->second;;
}

void JobManager::addJobRequirements(const sdpa::job_id_t& job_id, const requirement_list_t& job_req_list) throw (JobNotFoundException)
{
  lock_type lock(mtx_);
  job_requirements_.insert(requirements_map_t::value_type(job_id, job_req_list));
}

bool JobManager::slotAvailable () const
{
	return getNumberOfJobs () < MAX_PARALLEL_JOBS;
}

void JobManager::waitForFreeSlot ()
{
  lock_type lock(mtx_);
  free_slot_.wait (mtx_, boost::bind (&JobManager::slotAvailable, this));
}

void JobManager::resubmitResults(IAgent* pComm)
{
  lock_type lock(mtx_);

  for ( job_map_t::const_iterator it(job_map_.begin()); it != job_map_.end(); ++it )
  {
    sdpa::daemon::Job::ptr_t pJob = it->second;
    std::string job_status = pJob->getStatus();

    if( pJob->isMasterJob() )
    {
    	DLOG(TRACE, "Re-submit to the master "<<pJob->owner()<<" the status of the job"<<pJob->id()<<" ("<<job_status<<" )");
		if( job_status.find("Finished") != std::string::npos )
		{
		  // create jobFinishedEvent
		  sdpa::events::JobFinishedEvent::Ptr pEvtJobFinished( new sdpa::events::JobFinishedEvent(pComm->name(),
																								  pJob->owner(),
																								  pJob->id(),
																								  pJob->result() ));

		  // send it to the master
		  pComm->sendEventToMaster(pEvtJobFinished);
		}
		else if( job_status.find("Failed") != std::string::npos )
		{
		  // create jobFailedEvent
		  sdpa::events::JobFailedEvent::Ptr pEvtJobFailed( new sdpa::events::JobFailedEvent(pComm->name(),
																							pJob->owner(),
																							pJob->id(),
																							pJob->result() ));

		  // send it to the master
		  pComm->sendEventToMaster(pEvtJobFailed);
		}
		else if( job_status.find("Cancelled") != std::string::npos)
		{
		  // create jobCancelledEvent
		  sdpa::events::CancelJobAckEvent::Ptr pEvtJobCancelled( new sdpa::events::CancelJobAckEvent( pComm->name(),
																									  pJob->owner(),
																									  pJob->id()));

		  // send it to the master
		  pComm->sendEventToMaster(pEvtJobCancelled);
		}
		else if( job_status.find("Pending") != std::string::npos )
		{
		  sdpa::events::SubmitJobAckEvent::Ptr pSubmitJobAckEvt(new sdpa::events::SubmitJobAckEvent(pComm->name(),
																									pJob->owner(),
																									pJob->id(),
																									""));

		  // There is a problem with this if uncommented
		  pComm->sendEventToMaster(pSubmitJobAckEvt);
		}
    }
  }
}

sdpa::job_id_list_t JobManager::getListNotCompletedMasterJobs(bool bHasWfe)
{
  lock_type lock(mtx_);
	sdpa::job_id_list_t listJobsNotCompleted;

	for ( job_map_t::iterator it = job_map_.begin(); it != job_map_.end(); ++it )
	{
		sdpa::job_id_t jobId = it->first;
		Job::ptr_t pJob = it->second;

		if( (bHasWfe && pJob->isMasterJob()) || !bHasWfe )
		{
			std::string status = pJob->getStatus();

			// the job is not in a terminal state
			if(	status.find("Finished") 	== std::string::npos
				&& status.find("Failed") 	== std::string::npos
				&& status.find("Cancelled") == std::string::npos )
			{
				listJobsNotCompleted.push_back(jobId);
			}
		}
	}

	return listJobsNotCompleted;
}

size_t JobManager::countMasterJobs() const
{
	lock_type lock(mtx_);
	size_t nMasterJobs = 0;
	BOOST_FOREACH(const job_map_t::value_type& job_pair, job_map_ )
	{
		if( job_pair.second->isMasterJob() )
			nMasterJobs++;
	}

	return nMasterJobs;
}

size_t JobManager::getNumberOfJobs() const
{
	lock_type lock(mtx_);
	return job_map_.size();
}
