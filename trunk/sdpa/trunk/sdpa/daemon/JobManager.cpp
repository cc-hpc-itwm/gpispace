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

#include <sdpa/daemon/IComm.hpp>
#include <sdpa/daemon/JobImpl.hpp>
#include <boost/pointer_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/foreach.hpp>

using namespace std;
using namespace sdpa::daemon;

JobManager::JobManager(): SDPA_INIT_LOGGER("sdpa::daemon::JobManager")  {

}

JobManager::~JobManager(){
  LOG_IF( WARN
        , job_map_.size()
        , "there are still entries left in the job-map: " << job_map_.size()
        );
  LOG_IF( WARN
        , job_map_marked_for_del_.size()
        , "there are still entries left in the mark-for-deletion map: " << job_map_marked_for_del_.size()
        );
  LOG_IF( WARN
        , job_requirements_.size()
        , "there are still entries left in the preferences map: " << job_requirements_.size()
        );

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

void JobManager::markJobForDeletion(const sdpa::job_id_t& job_id, const Job::ptr_t& pJob) throw(JobNotMarkedException)
{
    lock_type lock(mtx_);
    ostringstream os;
    job_map_t::iterator it;
    bool bsucc = false;

    pair<job_map_t::iterator, bool> ret_pair(it, bsucc);
    pair<sdpa::job_id_t, Job::ptr_t> job_pair(job_id, pJob);

    ret_pair =  job_map_marked_for_del_.insert(job_pair);

    if(! ret_pair.second)
      throw JobNotAddedException(job_id);
}

void JobManager::deleteJob(const sdpa::job_id_t& job_id) throw(JobNotDeletedException)
{
    lock_type lock(mtx_);
    ostringstream os;

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
        DLOG(TRACE, "Erased job "<<job_id.str()<<" from job map");
    }

    free_slot_.notify_one();
}

std::vector<sdpa::job_id_t> JobManager::getJobIDList()
{
    lock_type lock(mtx_);
    std::vector<sdpa::job_id_t> v;
    for(job_map_t::iterator it = job_map_.begin(); it!= job_map_.end(); it++)
      v.push_back(it->first);

    return v;
}

std::string JobManager::print() const
{
    lock_type lock(mtx_);
    std::ostringstream os;

    SDPA_LOG_DEBUG("Begin dumping the JobManager...");

    if( job_map_.begin() == job_map_.end() )
            os<<"The JobManager is empty!";
    else
    {
        os<<"The list of jobs still owned by the JobManager:"<<std::endl;
        for ( job_map_t::const_iterator it (job_map_.begin()); it != job_map_.end(); ++it )
          SDPA_LOG_INFO(it->second->print_info());
    }

    SDPA_LOG_DEBUG("End dumping the JobManager...");

    return os.str();
}

const requirement_list_t JobManager::getJobRequirements(const sdpa::job_id_t& jobId) const throw (NoJobRequirements)
{
    lock_type lock(mtx_);
    if( job_requirements_.empty() )
            throw NoJobRequirements(jobId);

    SDPA_LOG_DEBUG("Locate the preferences of the job "<<jobId.str());
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

static const std::size_t MAX_PARALLEL_JOBS = 1024;
bool JobManager::slotAvailable () const
{
  return number_of_jobs () < MAX_PARALLEL_JOBS;
}

void JobManager::waitForFreeSlot ()
{
  lock_type lock(mtx_);
  free_slot_.wait (mtx_, boost::bind (&JobManager::slotAvailable, this));
}

void JobManager::updateJobInfo(sdpa::daemon::IComm* p)
{
    lock_type lock(mtx_);
    for ( job_map_t::const_iterator it(job_map_.begin()); it != job_map_.end(); ++it )
    {
        sdpa::daemon::Job* pJob = it->second.get();
        pJob->set_icomm(p);

        pJob->print_info();

        // if the job is in a terminal state do nothing
        std::string job_status = pJob->getStatus();
        if( job_status.find("Finished") != std::string::npos ||
            job_status.find("Failed") != std::string::npos ||
            job_status.find("Cancelled") != std::string::npos )
          return;

        // else, // not coming from WE
        // and it's not already in jobs_to_be scheduled!
        if( !p->is_scheduled(pJob->id()) )
        {
            // ATTENTION this attribute should be recovered
            //if( p->hasWorkflowEngine())
            //	pJob->set_local(true);
            //else
            //	pJob->set_local(false);

            SDPA_LOG_DEBUG("The job "<<pJob->id()<<" is not yet scheduled! Schedule it now.");
            p->schedule(pJob->id());
        }
        else
          SDPA_LOG_DEBUG("The job "<<pJob->id()<<" is already scheduled!");
    }
}

void JobManager::resubmitJobsAndResults(IComm* pComm)
{
    lock_type lock(mtx_);
    SDPA_LOG_INFO("Re-submit to the master the the results of the jobs that are either finished, failed or cancelled!)!");

    for ( job_map_t::const_iterator it(job_map_.begin()); it != job_map_.end(); ++it )
    {
        sdpa::daemon::Job* pJob = it->second.get();

        std::string job_status = pJob->getStatus();
        SDPA_LOG_DEBUG("The status of the job "<<pJob->id()<<" is "<<job_status<<"!!!!!!!!");

        if( pJob->isMasterJob() && job_status.find("Finished") != std::string::npos )
        {
        	// create jobFinishedEvent
        	sdpa::events::JobFinishedEvent::Ptr pEvtJobFinished( new sdpa::events::JobFinishedEvent(pComm->name(),
        																							pJob->owner(),
        																							pJob->id(),
        																							pJob->result() ));

        	// send it to the master
        	pComm->sendEventToMaster(pEvtJobFinished);
        }
        else if( pJob->isMasterJob() && job_status.find("Failed") != std::string::npos )
        {
        	// create jobFailedEvent
        	sdpa::events::JobFailedEvent::Ptr pEvtJobFailed( new sdpa::events::JobFailedEvent( pComm->name(),
        																						pJob->owner(),
        																						pJob->id(),
        																						pJob->result() ));

        	// send it to the master
        	pComm->sendEventToMaster(pEvtJobFailed);
        }
        else if( pJob->isMasterJob() && job_status.find("Cancelled") != std::string::npos)
        {
        	// create jobCancelledEvent
        	sdpa::events::CancelJobAckEvent::Ptr pEvtJobCancelled( new sdpa::events::CancelJobAckEvent( pComm->name(),
        																								pJob->owner(),
        																								pJob->id(),
        																								pJob->result() ));

        	// send it to the master
        	pComm->sendEventToMaster(pEvtJobCancelled);
        }
    }
}

unsigned int JobManager::countMasterJobs()
{
  lock_type lock(mtx_);
  unsigned int nMasterJobs = 0;
  BOOST_FOREACH(job_map_t::value_type& job_pair, job_map_ )
  {
	  if( job_pair.second->isMasterJob() )
		  nMasterJobs++;
  }

  return nMasterJobs;
}
