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

#include <sdpa/daemon/JobImpl.hpp>
#include <boost/pointer_cast.hpp>
#include <boost/shared_ptr.hpp>
#include <sdpa/daemon/IComm.hpp>

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
        , job_preferences_.size()
        , "there are still entries left in the preferences map: " << job_preferences_.size()
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

	// delete the preferences
	preference_map_t::size_type rc = job_preferences_.erase(job_id);

	if(rc)
        {
          DLOG(TRACE, "Erased the preferences of the job "<<job_id.str());
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

	SDPA_LOG_DEBUG("Begin dump JobManager...");

	os<<"The list of jobs still owned by the JobManager:"<<std::endl;
	for ( job_map_t::const_iterator it (job_map_.begin()); it != job_map_.end(); ++it )
	  SDPA_LOG_INFO(it->second->print_info());

	SDPA_LOG_DEBUG("End dump JobManager...");

	return os.str();
}

const we::preference_t& JobManager::getJobPreferences(const sdpa::job_id_t& jobId) const throw (NoJobPreferences)
{
	lock_type lock(mtx_);
	if( job_preferences_.empty() )
		throw NoJobPreferences(jobId);

	SDPA_LOG_DEBUG("Locate the preferences of the job "<<jobId.str());
	preference_map_t::const_iterator it_pref = job_preferences_.find(jobId);
	if( it_pref == job_preferences_.end() )
		throw NoJobPreferences(jobId);

	const we::preference_t& job_pref = it_pref->second;
	SDPA_LOG_DEBUG("The preferences of the job "<<jobId.str()<<" are: "<<job_pref);

	return job_pref;
}

void JobManager::addJobPreferences(const sdpa::job_id_t& job_id, const we::preference_t& pref) throw (JobNotFoundException)
{
	lock_type lock(mtx_);
	if( job_map_.find( job_id ) == job_map_.end() )
			throw JobNotFoundException( job_id );

	// eventually, re-write the existing preferences
	job_preferences_[job_id] = pref;
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
	for ( job_map_t::const_iterator it(job_map_.begin()); it != job_map_.end(); ++it )
	{
		sdpa::daemon::Job* pJob = it->second.get();
		pJob->set_icomm(p);

		std::string job_status = pJob->getStatus();
		SDPA_LOG_DEBUG("The status of the job "<<pJob->id()<<" is "<<job_status<<"!!!!!!!!");

		// not coming from WE
		// and it's not already in jobs_to_be scheduled!
		if( !p->is_scheduled(pJob->id()) )
		{
			// ATTENTION this attribute should be recovered
			if( p->hasWorkflowEngine())
				pJob->set_local(true);
			else
				pJob->set_local(false);

			SDPA_LOG_DEBUG("The job "<<pJob->id()<<" is not yet scheduled! Schedule it now.");
			p->schedule(pJob->id());
		}
		else
			SDPA_LOG_DEBUG("The job "<<pJob->id()<<" is already scheduled!");
	}
}

void JobManager::resubmitJobsAndResults(IComm* pComm)
{
	SDPA_LOG_INFO("Re-submit finished/failed/cancelled jobs)!");
	SDPA_LOG_INFO("The JobManager has "<<job_map_.size()<<" jobs!");

	for ( job_map_t::const_iterator it(job_map_.begin()); it != job_map_.end(); ++it )
	{
		sdpa::daemon::Job* pJob = it->second.get();

		std::string job_status = pJob->getStatus();
		SDPA_LOG_DEBUG("The status of the job "<<pJob->id()<<" is "<<job_status<<"!!!!!!!!");


		if( pJob->is_local() && job_status.find("Finished") != std::string::npos )
		{
			// create jobFinishedEvent
			sdpa::events::JobFinishedEvent::Ptr pEvtJobFinished( new sdpa::events::JobFinishedEvent(pComm->name(), pComm->master(), pJob->id(), pJob->result() ));

			// send it to the master
			pComm->sendEventToMaster(pEvtJobFinished);
		}
		else if( pJob->is_local() && job_status.find("Failed") != std::string::npos )
			 {
				// create jobFailedEvent
				sdpa::events::JobFailedEvent::Ptr pEvtJobFailed( new sdpa::events::JobFailedEvent( pComm->name(), pComm->master(), pJob->id(), pJob->result() ));

				// send it to the master
				pComm->sendEventToMaster(pEvtJobFailed);
			 }
			  else if( pJob->is_local() && job_status.find("Cancelled") != std::string::npos)
				{
					// create jobCancelledEvent
				  	sdpa::events::CancelJobAckEvent::Ptr pEvtJobCancelled( new sdpa::events::CancelJobAckEvent( pComm->name(), pComm->master(), pJob->id(), sdpa::events::SDPAEvent::message_id_type() ));

				  	// send it to the master
				  	pComm->sendEventToMaster(pEvtJobCancelled);
				}
	}
}
