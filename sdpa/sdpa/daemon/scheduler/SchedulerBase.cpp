/*
 * =====================================================================================
 *
 *       Filename:  SchedulerBase.hpp
 *
 *    Description:  Implements a simple scheduler
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

#include <fhg/assert.hpp>

#include <sdpa/daemon/JobFSM.hpp>
#include <sdpa/daemon/scheduler/SchedulerBase.hpp>
#include <sdpa/events/SubmitJobAckEvent.hpp>
#include <sdpa/events/RequestJobEvent.hpp>
#include <sdpa/events/LifeSignEvent.hpp>
#include <boost/tokenizer.hpp>

#include <cassert>
#include <sdpa/capability.hpp>

#include <sdpa/daemon/NotificationService.hpp>

using namespace sdpa::daemon;
using namespace sdpa::events;
using namespace std;

SchedulerBase::SchedulerBase(sdpa::daemon::IAgent* pCommHandler, bool bUseRequestModel )
  : ptr_worker_man_(new WorkerManager())
  , bStopRequested(false)
  , ptr_comm_handler_(pCommHandler)
  , SDPA_INIT_LOGGER((pCommHandler?pCommHandler->name().c_str():"Scheduler"))
  , m_timeout(boost::posix_time::milliseconds(100))
  , m_bUseRequestModel(bUseRequestModel)

{
}

SchedulerBase::~SchedulerBase()
{
  try  {
    if( pending_jobs_queue_.size() )
    {
      SDPA_LOG_WARN("The scheduler has still "<<pending_jobs_queue_.size()<<" jobs into his queue!");
    }

    stop();
  }
  catch (std::exception const & ex)
  {
    SDPA_LOG_ERROR("exception during SchedulerBase::stop(): " << ex.what());
  }
  catch (...)
  {
    SDPA_LOG_ERROR("unexpected exception during SchedulerBase::stop()");
  }
}

void SchedulerBase::addWorker(  const Worker::worker_id_t& workerId,
                                const unsigned int& capacity,
                                const capabilities_set_t& cpbset,
                                const unsigned int& agent_rank,
                                const sdpa::worker_id_t& agent_uuid ) throw (WorkerAlreadyExistException)
{
  try {
    ptr_worker_man_->addWorker(workerId, capacity, cpbset, agent_rank, agent_uuid);
    cond_workers_registered.notify_all();
    cond_feed_workers.notify_one();
  }
  catch( const WorkerAlreadyExistException& ex)
  {
    throw ex;
  }
}

void SchedulerBase::rescheduleWorkerJob( const Worker::worker_id_t& worker_id, const sdpa::job_id_t& job_id )
{
  if(bStopRequested)
  {
      SDPA_LOG_WARN("The scheduler is requested to stop. Job re-asignement is not anymore possible.");
      return;
  }

  ostringstream os;
  if(!ptr_comm_handler_)
  {
      SDPA_LOG_ERROR("Invalid communication handler. ");
      stop();
      return;
  }

  try
  {
      // delete it from the worker's queues
      Worker::ptr_t pWorker = findWorker(worker_id);
      pWorker->deleteJob(job_id);
  }
  catch (const WorkerNotFoundException& ex)
  {
      SDPA_LOG_WARN("Cannot find the worker "<<worker_id);
  }
  catch(JobNotDeletedException const & ex)
  {
      SDPA_LOG_WARN("The job " << job_id << " could not be deleted: " << ex.what());
  }
  catch(const std::exception& ex)
  {
      SDPA_LOG_WARN( "Could not re-schedule the job " << job_id << ": unexpected error!"<<ex.what() );
  }

  rescheduleJob(job_id);
}

void SchedulerBase::reschedule( const Worker::worker_id_t & worker_id, sdpa::job_id_list_t& workerJobList )
{
  if(!bStopRequested) {
      while( !workerJobList.empty() ) {
          sdpa::job_id_t jobId = workerJobList.front();
	  DMLOG (TRACE, "Re-scheduling the job "<<jobId.str()<<" ... ");
	  rescheduleWorkerJob(worker_id, jobId);
	  workerJobList.pop_front();
      }
  }
  else {
      SDPA_LOG_WARN("The scheduler is requested to stop. Job re-scheduling is not anymore possible.");
  }
}

void SchedulerBase::deleteWorker( const Worker::worker_id_t& worker_id ) throw (WorkerNotFoundException)
{
  // first re-schedule the work:
  // inspect all queues and re-schedule each job
  try {

    // mark the worker dirty -> don't take it in consideration for re-scheduling
    const Worker::ptr_t& pWorker = findWorker(worker_id);
    pWorker->set_disconnected(true);

    sdpa::job_id_list_t workerJobList(ptr_worker_man_->getJobListAndCleanQueues(pWorker));
    reschedule(worker_id, workerJobList);

    if( !workerJobList.empty() )
    {
        SDPA_LOG_WARN( "The worker " << worker_id << " has still has assigned jobs!");
        pWorker->print();
    }

    // delete the worker from the worker map
    ptr_worker_man_->deleteWorker(worker_id);
  }
  catch (const WorkerNotFoundException& ex)
  {
      SDPA_LOG_ERROR("Cannot delete the worker "<<worker_id<<". Worker not found!");
      throw ex;
  }
}

/*
        Schedule a job locally, send the job to WE
*/
/*
void SchedulerBase::schedule_local(const sdpa::job_id_t &jobId)
{
  DMLOG (TRACE, "Schedule the job "<<jobId.str()<<" to the workflow engine!");

  id_type wf_id = jobId.str();

  if( !ptr_comm_handler_ )
  {
    SDPA_LOG_ERROR("Cannot schedule locally the job "<<jobId<<"! No communication handler specified.");
    stop();
    return;
  }

  try {
    const Job::ptr_t& pJob = ptr_comm_handler_->findJob(jobId);

    // Should set the workflow_id here, or send it together with the workflow description
    DMLOG (TRACE, "The status of the job "<<jobId<<" is "<<pJob->getStatus());
    DMLOG (TRACE, "Submit the workflow attached to the job "<<jobId<<" to WE. ");
    pJob->Dispatch();
    DMLOG (TRACE, "The status of the job "<<jobId<<" is "<<pJob->getStatus());

    if(pJob->description().empty() )
    {
    	SDPA_LOG_ERROR("Empty Workflow!");
        // declare job as failed
    	JobFailedEvent::Ptr pEvtJobFailed
    	      (new JobFailedEvent( sdpa::daemon::WE
    	                         , ptr_comm_handler_->name()
    	                         , jobId
    	                         , ""
    	                         )
    	      );

    	pEvtJobFailed->error_code() = fhg::error::UNEXPECTED_ERROR;
    	pEvtJobFailed->error_message() = "The job has an empty workflow attached!";
    	ptr_comm_handler_->sendEventToSelf(pEvtJobFailed);
    }

    ptr_comm_handler_->submitWorkflow(wf_id, pJob->description());
  }
  catch(const NoWorkflowEngine& ex)
  {
    SDPA_LOG_ERROR("No workflow engine!");
    sdpa::job_result_t result(ex.what());

    JobFailedEvent::Ptr pEvtJobFailed
      (new JobFailedEvent( sdpa::daemon::WE
                         , ptr_comm_handler_->name()
                         , jobId
                         , result
                         )
      );
    pEvtJobFailed->error_code() = fhg::error::UNEXPECTED_ERROR;
    pEvtJobFailed->error_message() = "no workflow engine attached!";
    ptr_comm_handler_->sendEventToSelf(pEvtJobFailed);
  }
  catch(const JobNotFoundException& ex)
  {
    SDPA_LOG_ERROR("Job not found! Could not schedule locally the job "<<ex.job_id().str());
    sdpa::job_result_t result(ex.what());

    JobFailedEvent::Ptr pEvtJobFailed
      (new JobFailedEvent( sdpa::daemon::WE
                         , ptr_comm_handler_->name()
                         , jobId
                         , result
                         )
      );
    pEvtJobFailed->error_code() = fhg::error::UNEXPECTED_ERROR;
    pEvtJobFailed->error_message() = "job could not be found";

    ptr_comm_handler_->sendEventToSelf(pEvtJobFailed);
  }
  catch (std::exception const & ex)
  {
    SDPA_LOG_ERROR("Exception occurred when trying to submit the workflow "<<wf_id<<" to WE: "<<ex.what());

    //send a JobFailed event
    sdpa::job_result_t result(ex.what());

    JobFailedEvent::Ptr pEvtJobFailed
      (new JobFailedEvent( sdpa::daemon::WE
                         , ptr_comm_handler_->name()
                         , jobId
                         , result
                         )
      );
    pEvtJobFailed->error_code() = fhg::error::UNEXPECTED_ERROR;
    pEvtJobFailed->error_message() = ex.what();
    ptr_comm_handler_->sendEventToSelf(pEvtJobFailed);
  }
}*/

/*
        Schedule a job locally, send the job to WE
*/
void SchedulerBase::schedule_local(const sdpa::job_id_t &jobId)
{
  DMLOG (TRACE, "Schedule the job "<<jobId.str()<<" to the workflow engine!");

  id_type wf_id = jobId.str();

  if( !ptr_comm_handler_ )
  {
    SDPA_LOG_ERROR("Cannot schedule locally the job "<<jobId<<"! No communication handler specified.");
    stop();
    return;
  }

  try {
    const Job::ptr_t& pJob = ptr_comm_handler_->findJob(jobId);

    // Should set the workflow_id here, or send it together with the workflow description
    DMLOG (TRACE, "The status of the job "<<jobId<<" is "<<pJob->getStatus());
    DMLOG (TRACE, "Submit the workflow attached to the job "<<jobId<<" to WE. ");
    pJob->Dispatch();
    DMLOG (TRACE, "The status of the job "<<jobId<<" is "<<pJob->getStatus());

    if(pJob->description().empty() )
    {
        SDPA_LOG_ERROR("Empty Workflow!");
        // declare job as failed
        JobFailedEvent::Ptr pEvtJobFailed
              (new JobFailedEvent( sdpa::daemon::WE
                                 , ptr_comm_handler_->name()
                                 , jobId
                                 , ""
                                 , fhg::error::UNEXPECTED_ERROR
                                 , "The job has an empty workflow attached!"
                                 )
              );

        ptr_comm_handler_->sendEventToSelf(pEvtJobFailed);
    }

    ptr_comm_handler_->submitWorkflow(wf_id, pJob->description());
  }
  catch(const NoWorkflowEngine& ex)
  {
    SDPA_LOG_ERROR("No workflow engine!");
    sdpa::job_result_t result(ex.what());

    JobFailedEvent::Ptr pEvtJobFailed
      (new JobFailedEvent( sdpa::daemon::WE
                         , ptr_comm_handler_->name()
                         , jobId
                         , result
                         , fhg::error::UNEXPECTED_ERROR
                         , "no workflow engine attached!"
                         )
      );
    ptr_comm_handler_->sendEventToSelf(pEvtJobFailed);
  }
  catch(const JobNotFoundException& ex)
  {
    SDPA_LOG_ERROR("Job not found! Could not schedule locally the job "<<ex.job_id().str());
    sdpa::job_result_t result(ex.what());

    JobFailedEvent::Ptr pEvtJobFailed
      (new JobFailedEvent( sdpa::daemon::WE
                         , ptr_comm_handler_->name()
                         , jobId
                         , result
                         , fhg::error::UNEXPECTED_ERROR
                         , "job could not be found"
                         )
      );

    ptr_comm_handler_->sendEventToSelf(pEvtJobFailed);
  }
  catch (std::exception const & ex)
  {
    SDPA_LOG_ERROR("Exception occurred when trying to submit the workflow "<<wf_id<<" to WE: "<<ex.what());

    //send a JobFailed event
    sdpa::job_result_t result(ex.what());

    JobFailedEvent::Ptr pEvtJobFailed
      (new JobFailedEvent( sdpa::daemon::WE
                         , ptr_comm_handler_->name()
                         , jobId
                         , result
                         , fhg::error::UNEXPECTED_ERROR
                         , ex.what()
                         )
      );
    ptr_comm_handler_->sendEventToSelf(pEvtJobFailed);
  }
}

void SchedulerBase::delete_job (sdpa::job_id_t const & job)
{
  SDPA_LOG_DEBUG("removing job " << job << " from the scheduler's queue ....");
  if (pending_jobs_queue_.erase(job))
  {
    SDPA_LOG_DEBUG("removed job from the central queue...");
  }
  else
    ptr_worker_man_->deleteJob(job);
}

void printMatchingWorkers(const sdpa::job_id_t& jobId, const sdpa::list_match_workers_t& listMatchingWorkers)
{
  /*ostringstream ossMatchWorkers;ossMatchWorkers<<"[";
  for( sdpa::list_match_workers_t::iterator it=listMatchingWorkers.begin(); it!=listMatchingWorkers.end(); it++ )
  {
          ossMatchWorkers<<"("<<it->first<<","<<it->second<<")";
          if(boost::next(it)!=listMatchingWorkers.end())
                  ossMatchWorkers<<",";
  }

  ossMatchWorkers<<"]";
  DLOG( INFO, "The workers matching the requirements of the job "<<jobId<<" are: "<<ossMatchWorkers.str());*/
}

/*void SchedulerBase::schedule_remotely(const sdpa::job_id_t& jobId)
{
  try {
      const Job::ptr_t& pJob = ptr_comm_handler_->findJob(jobId);

      ptr_worker_man_->dispatchJob(jobId);
      // put the job into the running state
      pJob->Dispatch();
      cond_feed_workers.notify_one();
  }
  catch(const JobNotFoundException& ex)
  {
    sdpa::job_result_t result(ex.what());

    JobFailedEvent::Ptr pEvtJobFailed(new JobFailedEvent(ptr_comm_handler_->name()
                                                         , ptr_comm_handler_->name()
                                                         , jobId
                                                         , result
                                                         ));

    pEvtJobFailed->error_code() = fhg::error::UNEXPECTED_ERROR;
    pEvtJobFailed->error_message() = "job could not be found";

    ptr_comm_handler_->sendEventToSelf(pEvtJobFailed);
  }
  catch (std::exception const & ex)
  {
    //send a JobFailed event
    sdpa::job_result_t result(ex.what());

    JobFailedEvent::Ptr pEvtJobFailed
          (new JobFailedEvent(  ptr_comm_handler_->name()
                                , ptr_comm_handler_->name()
                                , jobId
                                , result
                                 ));

    pEvtJobFailed->error_code() = fhg::error::UNEXPECTED_ERROR;
    pEvtJobFailed->error_message() = ex.what();
    ptr_comm_handler_->sendEventToSelf(pEvtJobFailed);
  }
}*/


void SchedulerBase::schedule_remotely(const sdpa::job_id_t& jobId)
{
  try {
      const Job::ptr_t& pJob = ptr_comm_handler_->findJob(jobId);

      ptr_worker_man_->dispatchJob(jobId);
      // put the job into the running state
      pJob->Dispatch();
      cond_feed_workers.notify_one();
  }
  catch(const JobNotFoundException& ex)
  {
    sdpa::job_result_t result(ex.what());

    JobFailedEvent::Ptr pEvtJobFailed(new JobFailedEvent(ptr_comm_handler_->name()
                                                         , ptr_comm_handler_->name()
                                                         , jobId
                                                         , result
                                                         , fhg::error::UNEXPECTED_ERROR
                                                         , "job could not be found"
                                                         ));

    ptr_comm_handler_->sendEventToSelf(pEvtJobFailed);
  }
  catch (std::exception const & ex)
  {
    //send a JobFailed event
    sdpa::job_result_t result(ex.what());

    JobFailedEvent::Ptr pEvtJobFailed
          (new JobFailedEvent(  ptr_comm_handler_->name()
                                , ptr_comm_handler_->name()
                                , jobId
                                , result
                             , fhg::error::UNEXPECTED_ERROR
                             , ex.what()
                                 ));

    ptr_comm_handler_->sendEventToSelf(pEvtJobFailed);
  }
}

void SchedulerBase::schedule(const sdpa::job_id_t& jobId)
{
  //DMLOG(TRACE, "Schedule the job " << jobId.str());
  pending_jobs_queue_.push(jobId);
}

const Worker::ptr_t& SchedulerBase::findWorker(const Worker::worker_id_t& worker_id ) throw(WorkerNotFoundException)
{
  try {
    return ptr_worker_man_->findWorker(worker_id);
  }
  catch(WorkerNotFoundException)
  {
    throw WorkerNotFoundException(worker_id);
  }
}

const Worker::worker_id_t& SchedulerBase::findWorker(const sdpa::job_id_t& job_id) throw (NoWorkerFoundException)
{
  try {
    return ptr_worker_man_->findWorker(job_id);
  }
  catch(const NoWorkerFoundException& ex)
  {
    throw ex;
  }
}

const Worker::worker_id_t& SchedulerBase::findSubmOrAckWorker(const sdpa::job_id_t& job_id) throw (NoWorkerFoundException)
{
  return ptr_worker_man_->findSubmOrAckWorker(job_id);
}

void SchedulerBase::start(IAgent* p)
{
  if(p)
    ptr_comm_handler_ = p;

  bStopRequested = false;
  if(!ptr_comm_handler_)
  {
    SDPA_LOG_ERROR("The scheduler cannot be started. Invalid communication handler. ");
    return;
  }

  m_thread_run = boost::thread(boost::bind(&SchedulerBase::run, this));
  m_thread_feed = boost::thread(boost::bind(&SchedulerBase::feedWorkers, this));
}

void SchedulerBase::stop()
{
  bStopRequested = true;

  m_thread_run.interrupt();
  m_thread_feed.interrupt();

  if (m_thread_run.joinable() )
    m_thread_run.join();

  if (m_thread_feed.joinable() )
    m_thread_feed.join();

  pending_jobs_queue_.clear();
  cancellation_list_.clear();
  ptr_worker_man_->removeWorkers();
}

bool SchedulerBase::postRequest(const MasterInfo& masterInfo, bool force)
{
  if(!ptr_comm_handler_)
  {
    SDPA_LOG_ERROR("The scheduler cannot be started. Invalid communication handler. ");
    stop();
    return false;
  }

  bool bReqPosted = false;

  if(force || ( !ptr_comm_handler_->isTop()  &&  masterInfo.is_registered() ) )
  {
    bool bReqAllowed = ptr_comm_handler_->requestsAllowed();
    if(!bReqAllowed)
    {
      SDPA_LOG_DEBUG("The agent "<<ptr_comm_handler_->name()<<" is not allowed to post job requests!");
    }

    if( force || bReqAllowed )
    {
      ptr_comm_handler_->requestJob(masterInfo);
      //SDPA_LOG_DEBUG("The agent "<<ptr_comm_handler_->name()<<" has posted a new job request!");
      bReqPosted = true;
    }
  }

  return bReqPosted;
}

void SchedulerBase::checkRequestPosted()
{
  BOOST_FOREACH(sdpa::MasterInfo masterInfo, ptr_comm_handler_->getListMasterInfo())
  {
    if( !masterInfo.is_registered() )
    {
      MLOG (TRACE, "I am not registered yet...");

      const unsigned long reg_timeout( ptr_comm_handler_->cfg().get<unsigned long>("registration_timeout", 10 *1000*1000) );
      boost::this_thread::sleep(boost::posix_time::microseconds(reg_timeout));

      ptr_comm_handler_->requestRegistration(masterInfo);
    }
    else
    {
      // post job request if number_of_jobs() < #registered workers +1
      if( useRequestModel() )
        postRequest(masterInfo);
    }
  }
}

void SchedulerBase::getListNotFullWorkers(sdpa::worker_id_list_t& workerList)
{
  workerList.clear();
  ptr_worker_man_->getListNotFullWorkers(workerList);
}

sdpa::worker_id_t SchedulerBase::findSuitableWorker(const job_requirements_t& job_reqs, sdpa::worker_id_list_t& listAvailWorkers)
{
  lock_type lock(mtx_);
  sdpa::worker_id_t matchingWorkerId;

  try {
      matchingWorkerId = ptr_worker_man_->getBestMatchingWorker(job_reqs, listAvailWorkers);
      sdpa::worker_id_list_t::iterator it = std::find(listAvailWorkers.begin(), listAvailWorkers.end(), matchingWorkerId);
      listAvailWorkers.erase(it);
  }
  catch(NoWorkerFoundException& exc) {
  }

  return matchingWorkerId;
}

void SchedulerBase::feedWorkers()
{
  while(!bStopRequested)
  {
      lock_type lock(mtx_);
      cond_feed_workers.wait(lock);

      if( ptr_comm_handler_->hasJobs() )
        assignJobsToWorkers();
  }
}

void SchedulerBase::run()
{
  if(!ptr_comm_handler_)
  {
    SDPA_LOG_ERROR("The scheduler cannot be started. Invalid communication handler. ");
    stop();
    return;
  }

  while(!bStopRequested)
  {
    try
    {
      checkRequestPosted(); // eventually, post a request to the master

      if( numberOfWorkers()>0 )
        forceOldWorkerJobsTermination();

      sdpa::job_id_t jobId  = pending_jobs_queue_.pop_and_wait(m_timeout);
      const Job::ptr_t& pJob = ptr_comm_handler_->findJob(jobId);

      if( !pJob->isMasterJob() ) {
        // if it's an NRE just execute it!
        // Attention!: an NRE has no WorkerManager!
        // or has an Worker Manager and the workers are threads
        if( numberOfWorkers()>0 ) {
            schedule_remotely(jobId);
        }
        else {
            // just for testing
            if(ptr_comm_handler_->canRunTasksLocally()) {
                DLOG(TRACE, "I have no workers, but I'm able to execute myself the job "<<jobId.str()<<" ...");
                execute(jobId);
            }
            else {
                //SDPA_LOG_DEBUG("no worker available, put the job back into the scheduler's queue!");
                if( !ptr_comm_handler_->canRunTasksLocally() ) {
                    pending_jobs_queue_.push(jobId);
                    lock_type lock(mtx_);
                    cond_workers_registered.wait(lock);
                }
                else
                execute(jobId);
            }
        } // else fail
      }
      else // it's a master job
        schedule_local(jobId);
    }
    catch(const JobNotFoundException& ex)
    {
        SDPA_LOG_WARN("Could not schedule the job "<<ex.job_id().str()<<". Job not found -> possible recovery inconsistency ...)");
    }
    catch( const boost::thread_interrupted & )
    {
        DMLOG(DEBUG, "Thread interrupted ...");
        bStopRequested = true; // FIXME: can probably be removed
        break;
    }
    catch( const sdpa::daemon::QueueEmpty &)
    {
        // ignore
    }
    catch ( const std::exception &ex )
    {
        MLOG(ERROR, "exception in scheduler thread: " << ex.what());
        throw;
    }
  }
}

namespace
{
  enum ExecutionState
  {
    ACTIVITY_FINISHED,
    ACTIVITY_FAILED,
    ACTIVITY_CANCELLED,
  };
  typedef std::pair<ExecutionState, result_type> execution_result_t;
}

void SchedulerBase::execute(const sdpa::job_id_t& jobId)
{
  MLOG(TRACE, "executing activity: "<< jobId);
  const Job::ptr_t& pJob = ptr_comm_handler_->findJob(jobId);
  id_type act_id = pJob->id().str();

  execution_result_t result;
  encoded_type enc_act = pJob->description(); // assume that the NRE's workflow engine encodes the activity!

  if( !ptr_comm_handler_ )
  {
    // the comments indicate that the code is old (nre)
    // why does it still exist?
    // why don't we just have an assert(ptr_comm_handler_)?

    LOG(ERROR, "nre scheduler does not have a comm-handler!");
    result_type output_fail;
    ptr_comm_handler_->activityFailed ( ""
                                      , jobId
                                      , enc_act
                                      , fhg::error::UNEXPECTED_ERROR
                                      , "scheduler does not have a"
                                      " communication handler"
                                      );
    return;
  }

  try
  {
    pJob->Dispatch();
    //result = m_worker_.execute(enc_act, pJob->walltime());
    boost::this_thread::sleep(boost::posix_time::milliseconds(2));
    result = std::make_pair(ACTIVITY_FINISHED, enc_act);
  }
  catch( const boost::thread_interrupted &)
  {
    std::string errmsg("could not execute activity: interrupted");
    SDPA_LOG_ERROR(errmsg);
    result = std::make_pair(ACTIVITY_FAILED, enc_act);
  }
  catch (const std::exception &ex)
  {
    std::string errmsg("could not execute activity: ");
    errmsg += std::string(ex.what());
    SDPA_LOG_ERROR(errmsg);
    result = std::make_pair(ACTIVITY_FAILED, enc_act);
  }

  // check the result state and invoke the NRE's callbacks
  if( result.first == ACTIVITY_FINISHED )
  {
    DLOG(TRACE, "activity finished: " << act_id);
    // notify the gui
    // and then, the workflow engine
    ptr_comm_handler_->activityFinished("", jobId, result.second);
  }
  else if( result.first == ACTIVITY_FAILED )
  {
    DLOG(TRACE, "activity failed: " << act_id);
    // notify the gui
    // and then, the workflow engine
    ptr_comm_handler_->activityFailed ( ""
                                      , jobId
                                      , result.second
                                      , fhg::error::UNEXPECTED_ERROR
                                      , "scheduler could not dispatch job"
                                      );
  }
  else if( result.first == ACTIVITY_CANCELLED )
  {
    DLOG(TRACE, "activity cancelled: " << act_id);

    // notify the gui
    // and then, the workflow engine
    ptr_comm_handler_->activityCancelled("", jobId);
  }
  else
  {
    SDPA_LOG_ERROR("Invalid status of the executed activity received from the worker!");
    ptr_comm_handler_->activityFailed ( ""
                                      , jobId
                                      , result.second
                                      , fhg::error::UNEXPECTED_ERROR
                                      , "invalid job state received"
                                      );
  }
}

void SchedulerBase::print()
{
  if(!pending_jobs_queue_.empty())
  {
    SDPA_LOG_DEBUG("The content of agent's scheduler queue is:");
    pending_jobs_queue_.print();
  }
  else
  {
    SDPA_LOG_DEBUG("No job to be scheduled left!");
  }

  ptr_worker_man_->print();
}

void SchedulerBase::acknowledgeJob(const Worker::worker_id_t& worker_id, const sdpa::job_id_t& job_id) throw( WorkerNotFoundException, JobNotFoundException)
{
  DLOG(TRACE, "Acknowledge the job "<<job_id.str());
  try {
    // make sure that the job is erased from the scheduling queue

	// don't need this!
	//jobs_to_be_scheduled.erase( job_id );
    Worker::ptr_t ptrWorker = findWorker(worker_id);

    //put the job into the Running state: do this in acknowledge!
    if( !ptrWorker->acknowledge(job_id) )
      throw JobNotFoundException(job_id);
  }
  catch(JobNotFoundException const& ex1)
  {
    SDPA_LOG_ERROR("Could not find the job "<<job_id.str()<<"!");
    throw ex1;
  }
  catch(WorkerNotFoundException const &ex2)
  {
    throw ex2;
  }
}

void SchedulerBase::deleteWorkerJob( const Worker::worker_id_t& worker_id, const sdpa::job_id_t &jobId ) throw (JobNotDeletedException, WorkerNotFoundException)
{
  try {
    lock_type lock(mtx_);

    ///jobs_to_be_scheduled.erase( job_id );
    // check if there is an allocation list for this job
    // (assert that the head of this list id worker_id!)
    // free all the workers in this list, i.e. mark them as not reserved
    ptr_worker_man_->deleteWorkerJob(worker_id, jobId);
    cond_feed_workers.notify_one();
  }
  catch(JobNotDeletedException const& ex1)
  {
    SDPA_LOG_WARN("The job "<<jobId.str()<<" couldn't be found!");
    throw ex1;
  }
  catch(WorkerNotFoundException const &ex2 )
  {
    SDPA_LOG_WARN("The worker "<<worker_id<<" couldn't be found!");
    throw ex2;
  }
  catch(const std::exception& ex3 )
  {
    SDPA_LOG_WARN("Unexpected exception occurred when trying to delete the job "<<jobId.str()<<" from the worker "<<worker_id<<": "<< ex3.what() );
    throw ex3;
  }
}

bool SchedulerBase::has_job(const sdpa::job_id_t& job_id)
{
  return pending_jobs_queue_.has_item(job_id)|| ptr_worker_man_->has_job(job_id);
}

void SchedulerBase::getWorkerList(sdpa::worker_id_list_t& workerList)
{
  workerList.clear();
  ptr_worker_man_->getWorkerList(workerList);
}

bool SchedulerBase::addCapabilities(const sdpa::worker_id_t& worker_id, const sdpa::capabilities_set_t& cpbset)
{
  if(bStopRequested)
  {
      SDPA_LOG_DEBUG("The scheduler was requested to stop ...");
      return false;
  }

  try
  {
      return ptr_worker_man_->addCapabilities(worker_id, cpbset);
  }
  catch(const std::exception& exc)
  {
      SDPA_LOG_ERROR("Exception occured when trying to add new capabilities to the worker "<<worker_id<<": "<<exc.what());
      throw exc;
  }
}

void SchedulerBase::removeCapabilities(const sdpa::worker_id_t& worker_id, const sdpa::capabilities_set_t& cpbset) throw (WorkerNotFoundException)
{
  if(bStopRequested) {
      SDPA_LOG_DEBUG("The scheduler is requested to stop, no need to remove capabilities ...");
      return;
  }

  ptr_worker_man_->removeCapabilities(worker_id, cpbset);
}

void SchedulerBase::getAllWorkersCapabilities(sdpa::capabilities_set_t& cpbset)
{
  ptr_worker_man_->getCapabilities(ptr_comm_handler_->name(), cpbset);
}

void SchedulerBase::getWorkerCapabilities(const sdpa::worker_id_t& worker_id, sdpa::capabilities_set_t& cpbset)
{
  try {
      Worker::ptr_t ptrWorker = findWorker(worker_id);
      cpbset = ptrWorker->capabilities();
  }
  catch(WorkerNotFoundException const &ex2 )
  {
      SDPA_LOG_ERROR("The worker "<<worker_id<<" could not be found!");
      cpbset = sdpa::capabilities_set_t();
  }
}

void SchedulerBase::cancelWorkerJobs()
{
  ptr_worker_man_->cancelWorkerJobs(this);
}

void SchedulerBase::planForCancellation(const Worker::worker_id_t& workerId, const sdpa::job_id_t& jobId)
{
  cancellation_list_.push_back(sdpa::worker_job_pair_t(workerId, jobId));
}

void SchedulerBase::forceOldWorkerJobsTermination()
{
  // cannot recover the jobs produced by the workflow engine
  if(ptr_comm_handler_->hasWorkflowEngine())
  {
    sdpa::cancellation_list_t new_cancellation_list;
    while( !cancellation_list_.empty() )
    {
      worker_job_pair_t worker_job_pair = cancellation_list_.front();
      sdpa::worker_id_t workerId = worker_job_pair.first;
      sdpa::job_id_t jobId = worker_job_pair.second;

      try {
        SDPA_LOG_INFO("Tell the worker "<<workerId<<" to cancel the job "<<jobId);
        Worker::ptr_t pWorker = findWorker(workerId);

        CancelJobEvent::Ptr pEvtCancelJob (new CancelJobEvent(  ptr_comm_handler_->name()
                                                                , workerId
                                                                , jobId
                                                                , "The master recovered after a crash!") );

        ptr_comm_handler_->sendEventToSlave(pEvtCancelJob);
      }
      catch (const WorkerNotFoundException& ex)
      {
        new_cancellation_list.push_back(worker_job_pair);
        //SDPA_LOG_WARN("Couldn't find the worker "<<workerId<<"(not registered yet)!");
      }

      cancellation_list_.pop_front();
    }

    cancellation_list_ = new_cancellation_list;
  }
}

Worker::worker_id_t SchedulerBase::getWorkerId(unsigned int r)
{
  return ptr_worker_man_->getWorkerId(r);
}

void SchedulerBase::setLastTimeServed(const worker_id_t& wid, const sdpa::util::time_type& servTime)
{
  ptr_worker_man_->setLastTimeServed(wid, servTime);
}

sdpa::job_id_t SchedulerBase::getNextJobToSchedule()
{
  sdpa::job_id_t jobId("");
  try {
      jobId = pending_jobs_queue_.pop();
  }
  catch( QueueEmpty& ex)
  {
      LOG(WARN, "there is no job to be scheduled");
  }
  return jobId;
}

void SchedulerBase::schedule_first(const sdpa::job_id_t& jid)
{
  ptr_worker_man_->common_queue_.push_front(jid);
}
