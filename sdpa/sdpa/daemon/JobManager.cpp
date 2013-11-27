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

#include <sdpa/daemon/IAgent.hpp>
#include <sdpa/daemon/Job.hpp>
#include <boost/pointer_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/foreach.hpp>

using namespace std;
using namespace sdpa::daemon;

JobManager::JobManager(const std::string& name)
  : SDPA_INIT_LOGGER(name)
{}

//helpers
Job::ptr_t JobManager::findJob(const sdpa::job_id_t& job_id )
{
  lock_type lock(mtx_);
  job_map_t::iterator it = job_map_.find( job_id );
  if( it != job_map_.end() )
    return it->second;
  else
    return NULL;
}

void JobManager::addJob(const sdpa::job_id_t& job_id, const Job::ptr_t& pJob, const job_requirements_t& job_req_list )
{
  lock_type lock(mtx_);
  job_map_t::iterator it;
  bool bsucc = false;

  pair<job_map_t::iterator, bool> ret_pair(it, bsucc);
  pair<sdpa::job_id_t, Job::ptr_t> job_pair(job_id, pJob);

  ret_pair =  job_map_.insert(job_pair);
  if(!ret_pair.second)
     throw JobNotAddedException(job_id);

  if(!job_req_list.empty())
    job_requirements_.insert(requirements_map_t::value_type(job_id, job_req_list));
}

void JobManager::deleteJob(const sdpa::job_id_t& job_id)
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
}

//! \todo Why doesn't every job have an entry here?
const job_requirements_t JobManager::getJobRequirements(const sdpa::job_id_t& jobId) const
{
  lock_type lock(mtx_);

  DLOG(TRACE, "Locate the preferences of the job "<<jobId.str());
  requirements_map_t::const_iterator it_req = job_requirements_.find(jobId);
  if( it_req == job_requirements_.end() )
    return job_requirements_t();

  return it_req->second;;
}

void JobManager::addJobRequirements(const sdpa::job_id_t& job_id, const job_requirements_t& job_req_list)
{
  lock_type lock(mtx_);
  job_requirements_.insert(requirements_map_t::value_type(job_id, job_req_list));
}

void JobManager::resubmitResults(IAgent* pComm)
{
  lock_type lock(mtx_);

  for ( job_map_t::const_iterator it(job_map_.begin()); it != job_map_.end(); ++it )
  {
    sdpa::daemon::Job::ptr_t pJob = it->second;

    if( pJob->isMasterJob() )
    {
      switch (pJob->getStatus())
      {
      case sdpa::status::FINISHED:
        {
          sdpa::events::JobFinishedEvent::Ptr pEvtJobFinished( new sdpa::events::JobFinishedEvent(pComm->name(),
                                                                                                 pJob->owner(),
                                                                                                 pJob->id(),
                                                                                                 pJob->result() ));
          pComm->sendEventToMaster(pEvtJobFinished);
        }
        break;

      case sdpa::status::FAILED:
        {
          sdpa::events::JobFailedEvent::Ptr pEvtJobFailed( new sdpa::events::JobFailedEvent(pComm->name(),
                                                                                           pJob->owner(),
                                                                                           pJob->id(),
                                                                                           pJob->result() ));
          pComm->sendEventToMaster(pEvtJobFailed);
        }
        break;

      case sdpa::status::CANCELED:
        {
          sdpa::events::CancelJobAckEvent::Ptr pEvtJobCanceled( new sdpa::events::CancelJobAckEvent( pComm->name(),
                                                                                                    pJob->owner(),
                                                                                                    pJob->id()));

          pComm->sendEventToMaster(pEvtJobCanceled);
        }
        break;

      case sdpa::status::PENDING:
        {
          sdpa::events::SubmitJobAckEvent::Ptr pSubmitJobAckEvt(new sdpa::events::SubmitJobAckEvent(pComm->name(),
                                                                                                   pJob->owner(),
                                                                                                   pJob->id()));
          // There is a problem with this if uncommented
          pComm->sendEventToMaster(pSubmitJobAckEvt);
        }
        break;
      }
    }
  }
}

bool JobManager::hasJobs() const
{
  lock_type lock(mtx_);
  return !job_map_.empty();
}
