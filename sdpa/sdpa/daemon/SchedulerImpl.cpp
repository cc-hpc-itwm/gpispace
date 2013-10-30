/*
 * =====================================================================================
 *
 *       Filename:  SchedulerImpl.hpp
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
#include <sdpa/daemon/SchedulerImpl.hpp>
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

SchedulerImpl::SchedulerImpl(sdpa::daemon::IAgent* pCommHandler, bool bUseRequestModel )
  : ptr_worker_man_(new WorkerManager())
  , bStopRequested(false)
  , ptr_comm_handler_(pCommHandler)
  , SDPA_INIT_LOGGER((pCommHandler?pCommHandler->name().c_str():"Scheduler"))
  , m_timeout(boost::posix_time::milliseconds(100))
  , m_bUseRequestModel(bUseRequestModel)

{
}

SchedulerImpl::~SchedulerImpl()
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
    SDPA_LOG_ERROR("exception during SchedulerImpl::stop(): " << ex.what());
  }
  catch (...)
  {
    SDPA_LOG_ERROR("unexpected exception during SchedulerImpl::stop()");
  }
}

void SchedulerImpl::addWorker(  const Worker::worker_id_t& workerId,
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

void SchedulerImpl::reschedule(const sdpa::job_id_t& job_id )
{
  if(bStopRequested)
  {
      SDPA_LOG_WARN("The scheduler is requested to stop. Job re-scheduling is not anymore possible.");
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
      Job::ptr_t pJob = ptr_comm_handler_->jobManager()->findJob(job_id);
      if(!pJob->completed()) {
          // clear the allocation table

          // cancel running jobs

          releaseReservation(job_id);

          pJob->Reschedule(ptr_comm_handler_); // put the job back into the pending state
        }
  }
  catch(JobNotFoundException const &ex)
  {
      SDPA_LOG_WARN("Cannot re-schedule the job " << job_id << ". The job could not be found!");
  }
  catch(const std::exception& ex)
  {
      SDPA_LOG_WARN( "Could not re-schedule the job " << job_id << ": unexpected error!"<<ex.what() );
  }
}


void SchedulerImpl::reschedule( const Worker::worker_id_t& worker_id, const sdpa::job_id_t& job_id )
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

  reschedule(job_id);
}

void SchedulerImpl::reschedule( const Worker::worker_id_t & worker_id, sdpa::job_id_list_t& workerJobList )
{
  if(!bStopRequested) {
      while( !workerJobList.empty() ) {
          sdpa::job_id_t jobId = workerJobList.front();
	  DMLOG (TRACE, "Re-scheduling the job "<<jobId.str()<<" ... ");
	  reschedule(worker_id, jobId);
	  workerJobList.pop_front();
      }
  }
  else {
      SDPA_LOG_WARN("The scheduler is requested to stop. Job re-scheduling is not anymore possible.");
  }
}

void SchedulerImpl::reschedule( const Worker::worker_id_t& worker_id )
{
  if(bStopRequested)
  {
      SDPA_LOG_WARN("The scheduler is requested to stop. Job re-scheduling is not anymore possible.");
      return;
  }

  try {
      const Worker::ptr_t& pWorker = findWorker(worker_id);

      // The jobs submitted by the WE should have set a property
      // which indicates whether the daemon can safely re-schedule these activities or not (reason: ex global mem. alloc)
      pWorker->set_disconnected();

      sdpa::job_id_list_t workerJobList(ptr_worker_man_->getJobListAndCleanQueues(pWorker));
      reschedule(worker_id, workerJobList);

      // put the jobs back into the central queue and don't forget
      // to reset the status
  }
  catch (const WorkerNotFoundException& ex)
  {
      SDPA_LOG_WARN("Could not re-schedule the jobs of the worker "<<worker_id<<": no such worker exists!");
  }
}

void SchedulerImpl::deleteWorker( const Worker::worker_id_t& worker_id ) throw (WorkerNotFoundException)
{
  // first re-schedule the work:
  // inspect all queues and re-schedule each job
  try {

    // mark the worker dirty -> don't take it in consideration for re-scheduling
    const Worker::ptr_t& pWorker = findWorker(worker_id);
    pWorker->set_disconnected(true);

    reschedule(worker_id);
    sdpa::job_id_list_t workerJobList(ptr_worker_man_->getJobListAndCleanQueues(pWorker));

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
void SchedulerImpl::schedule_local(const sdpa::job_id_t &jobId)
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
}

void SchedulerImpl::delete_job (sdpa::job_id_t const & job)
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

void SchedulerImpl::schedule_remotely(const sdpa::job_id_t& jobId)
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
}

void SchedulerImpl::schedule(const sdpa::job_id_t& jobId)
{
  //DMLOG(TRACE, "Schedule the job " << jobId.str());
  pending_jobs_queue_.push(jobId);
}

const Worker::ptr_t& SchedulerImpl::findWorker(const Worker::worker_id_t& worker_id ) throw(WorkerNotFoundException)
{
  try {
    return ptr_worker_man_->findWorker(worker_id);
  }
  catch(WorkerNotFoundException)
  {
    throw WorkerNotFoundException(worker_id);
  }
}

const Worker::worker_id_t& SchedulerImpl::findWorker(const sdpa::job_id_t& job_id) throw (NoWorkerFoundException)
{
  try {
    return ptr_worker_man_->findWorker(job_id);
  }
  catch(const NoWorkerFoundException& ex)
  {
    throw ex;
  }
}

const Worker::worker_id_t& SchedulerImpl::findSubmOrAckWorker(const sdpa::job_id_t& job_id) throw (NoWorkerFoundException)
{
  return ptr_worker_man_->findSubmOrAckWorker(job_id);
}

void SchedulerImpl::start(IAgent* p)
{
  if(p)
    ptr_comm_handler_ = p;

  bStopRequested = false;
  if(!ptr_comm_handler_)
  {
    SDPA_LOG_ERROR("The scheduler cannot be started. Invalid communication handler. ");
    return;
  }

  m_thread_run = boost::thread(boost::bind(&SchedulerImpl::run, this));
  m_thread_feed = boost::thread(boost::bind(&SchedulerImpl::feedWorkers, this));
}

void SchedulerImpl::stop()
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

bool SchedulerImpl::postRequest(const MasterInfo& masterInfo, bool force)
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

void SchedulerImpl::checkRequestPosted()
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

void SchedulerImpl::getListNotFullWorkers(sdpa::worker_id_list_t& workerList)
{
  workerList.clear();
  ptr_worker_man_->getListNotFullWorkers(workerList);
}

void SchedulerImpl::getListNotAllocatedWorkers(sdpa::worker_id_list_t& workerList)
{
  workerList.clear();
  ptr_worker_man_->getListWorkersNotReserved(workerList);
}

void SchedulerImpl::reserveWorker(const sdpa::job_id_t& jobId, const sdpa::worker_id_t& matchingWorkerId, const size_t& cap) throw( WorkerReservationFailed)
{
  ptr_worker_man_->reserveWorker(matchingWorkerId);
  // allocate this worker to the job with the jobId

  lock_type lock_table(mtx_alloc_table_);
  allocation_table_t::iterator it(allocation_table_.find(jobId));
  if(it==allocation_table_.end()) {
      Reservation reservation(jobId, cap);
      allocation_table_t::value_type pairJobRes(jobId, reservation);
      allocation_table_.insert(pairJobRes);
  }

  allocation_table_[jobId].addWorker(matchingWorkerId);
}

sdpa::worker_id_t SchedulerImpl::findSuitableWorker(const job_requirements_t& job_reqs, sdpa::worker_id_list_t& listAvailWorkers)
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

void SchedulerImpl::assignJobsToWorkers()
{
  lock_type lock(mtx_);
  sdpa::worker_id_list_t listAvailWorkers;

  if(ptr_worker_man_->common_queue_.empty())
    return;

  // replace this with the list of workers not reserved
  //getListNotFullWorkers(listAvailWorkers);
  getListNotAllocatedWorkers(listAvailWorkers);

  // check if there are jobs that can already be scheduled on
  // these workers
  JobQueue nonmatching_jobs_queue;

  // iterate over all jobs and see if there is one that prefers
  while(!ptr_worker_man_->common_queue_.empty() && !listAvailWorkers.empty())
  {
    sdpa::job_id_t jobId(ptr_worker_man_->common_queue_.pop());

    size_t nReqWorkers(1); // default number of required workers is 1
    sdpa::worker_id_t matchingWorkerId;

    try {
      job_requirements_t job_reqs(ptr_comm_handler_->getJobRequirements(jobId));

      nReqWorkers = job_reqs.numWorkers();
      matchingWorkerId = findSuitableWorker(job_reqs, listAvailWorkers);
    }
    catch( const NoJobRequirements& ex ) // no requirements are specified
    {
      // we have an empty list of requirements then!
      matchingWorkerId = listAvailWorkers.front();
      listAvailWorkers.erase(listAvailWorkers.begin());
    }

    if( !matchingWorkerId.empty() ) // matching found
    {
      reserveWorker(jobId, matchingWorkerId, nReqWorkers);

      lock_type lock(mtx_alloc_table_);
      // attention: what to do if job_reqs.n_workers_req > total number of registered workers?
      // if all the required resources were acquired, mark the job as submitted
      Reservation& reservation(allocation_table_[jobId]);

      if( reservation.acquired() ) {
        LOG(INFO, "A resource reservation for the job "<<jobId<<" has been acquired!");
        sdpa::job_id_t headWorker(reservation.headWorker());

        // serve a job to all workers, not only to the head worker!!!
        //ptr_comm_handler_->serveJob(headWorker, jobId);

        // serve the same job to all reserved workers!!!!
        ptr_comm_handler_->serveJob(reservation);
      }
      else
        ptr_worker_man_->common_queue_.push_front(jobId);
    }
    else // put it back into the common queue
    {
        nonmatching_jobs_queue.push(jobId);
    }
  }

  // nonmatching_jobs_queue.print("The list of non-matching jobs: ");
  while(!nonmatching_jobs_queue.empty())
    ptr_worker_man_->common_queue_.push_front(nonmatching_jobs_queue.pop_back());

}

void SchedulerImpl::feedWorkers()
{
  while(!bStopRequested)
  {
      lock_type lock(mtx_);
      cond_feed_workers.wait(lock);

      if( ptr_comm_handler_->jobManager()->getNumberOfJobs() > 0 )
        assignJobsToWorkers();
  }
}

void SchedulerImpl::run()
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

void SchedulerImpl::execute(const sdpa::job_id_t& jobId)
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


void SchedulerImpl::print()
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

void SchedulerImpl::acknowledgeJob(const Worker::worker_id_t& worker_id, const sdpa::job_id_t& job_id) throw( WorkerNotFoundException, JobNotFoundException)
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

void SchedulerImpl::releaseReservation(const sdpa::job_id_t& jobId)
{
  lock_type lock_table(mtx_alloc_table_);
  // should first kill/cancel the job

  // if the status is not terminal
  try {
      Job::ptr_t pJob(ptr_comm_handler_->findJob(jobId));
      allocation_table_t::const_iterator it = allocation_table_.find(jobId);

      // if there are allocated resources
      if(it==allocation_table_.end()) {
          LOG(WARN, "No reservation was found for the job "<<jobId);
          return;
      }

      /*if(pJob->is_running()) {
          sdpa::worker_id_t head_worker_id(allocation_table_[jobId].headWorker());
          SDPA_LOG_INFO("Tell the worker "<<head_worker_id<<" to cancel the job "<<jobId);
          Worker::ptr_t pWorker = findWorker(head_worker_id);
          CancelJobEvent::Ptr pEvtCancelJob (new CancelJobEvent(  ptr_comm_handler_->name()
                                                                , head_worker_id
                                                                , jobId
                                                                , "The master recovered after a crash!") );

          ptr_comm_handler_->sendEventToSlave(pEvtCancelJob);
      }*/

      lock_type lock_worker (mtx_);
      worker_id_list_t listWorkers(allocation_table_[jobId].getWorkerList());
      BOOST_FOREACH (sdpa::worker_id_t const& workerId, listWorkers)
      {
        try
        {
          findWorker(workerId)->free();
        }
        catch (WorkerNotFoundException const& ex)
        {
          LOG(WARN, "The worker "<<workerId<<" was not found, it was already released!");
        }
      }

      allocation_table_.erase(jobId);
  }
  catch(JobNotFoundException const& ex2)
  {
      LOG(WARN, "The job "<<jobId<<" was not found!");
  }
}

void SchedulerImpl::deleteWorkerJob( const Worker::worker_id_t& worker_id, const sdpa::job_id_t &jobId ) throw (JobNotDeletedException, WorkerNotFoundException)
{
  try {
    lock_type lock(mtx_);

    ///jobs_to_be_scheduled.erase( job_id );
    // check if there is an allocation list for this job
    // (assert that the head of this list id worker_id!)
    // free all the workers in this list, i.e. mark them as not reserved
    ptr_worker_man_->deleteWorkerJob(worker_id, jobId);

    // free the allocated resources for this job
    // releaseAllocatedWorkers(jobId);

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

bool SchedulerImpl::has_job(const sdpa::job_id_t& job_id)
{
  return pending_jobs_queue_.has_item(job_id)|| ptr_worker_man_->has_job(job_id);
}

void SchedulerImpl::getWorkerList(sdpa::worker_id_list_t& workerList)
{
  workerList.clear();
  ptr_worker_man_->getWorkerList(workerList);
}

bool SchedulerImpl::addCapabilities(const sdpa::worker_id_t& worker_id, const sdpa::capabilities_set_t& cpbset)
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

void SchedulerImpl::removeCapabilities(const sdpa::worker_id_t& worker_id, const sdpa::capabilities_set_t& cpbset) throw (WorkerNotFoundException)
{
  if(bStopRequested) {
      SDPA_LOG_DEBUG("The scheduler is requested to stop, no need to remove capabilities ...");
      return;
  }

  ptr_worker_man_->removeCapabilities(worker_id, cpbset);
}

void SchedulerImpl::getAllWorkersCapabilities(sdpa::capabilities_set_t& cpbset)
{
  ptr_worker_man_->getCapabilities(ptr_comm_handler_->name(), cpbset);
}

void SchedulerImpl::getWorkerCapabilities(const sdpa::worker_id_t& worker_id, sdpa::capabilities_set_t& cpbset)
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

void SchedulerImpl::cancelWorkerJobs()
{
  ptr_worker_man_->cancelWorkerJobs(this);
}

void SchedulerImpl::planForCancellation(const Worker::worker_id_t& workerId, const sdpa::job_id_t& jobId)
{
  cancellation_list_.push_back(sdpa::worker_job_pair_t(workerId, jobId));
}

void SchedulerImpl::forceOldWorkerJobsTermination()
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

Worker::worker_id_t SchedulerImpl::getWorkerId(unsigned int r)
{
  return ptr_worker_man_->getWorkerId(r);
}

void SchedulerImpl::setLastTimeServed(const worker_id_t& wid, const sdpa::util::time_type& servTime)
{
  ptr_worker_man_->setLastTimeServed(wid, servTime);
}

void SchedulerImpl::printAllocationTable()
{
  lock_type lock(mtx_alloc_table_);
  ostringstream oss;
  BOOST_FOREACH(const allocation_table_t::value_type& pairJLW, allocation_table_)
  {
    oss<<pairJLW.first<<" : ";
    worker_id_list_t workerList(pairJLW.second.getWorkerList());
    BOOST_FOREACH(const sdpa::worker_id_t& wid, workerList)
      oss<<wid<<" ";
    oss<<endl;
  }

  LOG(INFO, "Content of the allocation table:\n"<<oss.str());
}

sdpa::job_id_t SchedulerImpl::getAssignedJob(const sdpa::worker_id_t& wid)
{
  lock_type lock(mtx_alloc_table_);

  allocation_table_t::iterator it = allocation_table_.begin();
  while(it != allocation_table_.end())
  {
      if(it->second.hasWorker(wid))
        return it->first;
      else
        it++;
  }

  return job_id_t("");
}

void SchedulerImpl::checkAllocations()
{
  lock_type lock(mtx_alloc_table_);
  typedef std::map<worker_id_t,int> worker_cnt_map_t;
  worker_cnt_map_t worker_cnt_map;
  worker_id_list_t worker_list;
  getWorkerList(worker_list);

  BOOST_FOREACH(const worker_id_t& worker_id, worker_list)
  {
    worker_cnt_map.insert(worker_cnt_map_t::value_type(worker_id, 0));
  }

  BOOST_FOREACH(const allocation_table_t::value_type& pairJLW, allocation_table_)
  {
    worker_id_list_t workerList(pairJLW.second.getWorkerList());
    BOOST_FOREACH(const sdpa::worker_id_t& wid, workerList)
    {
      worker_cnt_map[wid]++;
      if(worker_cnt_map[wid]>1)
        {
          LOG(FATAL, "Error! The worker "<<wid<<" was allocated to two different jobs!");
          throw;
	}
    }
  }

  ostringstream oss;
  BOOST_FOREACH(const worker_id_t& worker_id, worker_list)
  {
    oss<<worker_id<<":"<<worker_cnt_map[worker_id]<<" ";
  }
  LOG(INFO, oss.str());
}

sdpa::job_id_t SchedulerImpl::getNextJobToSchedule()
{
  sdpa::job_id_t jobId;
  try {
      jobId = pending_jobs_queue_.pop();
  }
  catch( QueueEmpty& ex)
  {
      LOG(WARN, "there is no job to be scheduled");
  }
  return jobId;
}

bool SchedulerImpl::groupFinished(const sdpa::job_id_t& jid)
{
  lock_type lock(mtx_alloc_table_);
  bool bFinished(false);

  Reservation reservation(allocation_table_[jid]);
  return reservation.groupFinished();
}

/*
void SchedulerImpl::declare_jobs_failed(const Worker::worker_id_t& worker_id, Worker::JobQueue* pQueue )
{
  assert (pQueue);

  while( !pQueue->empty() )
  {
    sdpa::job_id_t jobId = pQueue->pop();
    SDPA_LOG_INFO( "Declare the job "<<jobId.str()<<" failed!" );

    if( ptr_comm_handler_ )
    {
      Job::ptr_t pJob = ptr_comm_handler_->jobManager()->findJob(jobId);
      ptr_comm_handler_->activityFailed( worker_id
                                       , jobId
                                       , pJob->result()
                                       , fhg::error::WORKER_TIMEDOUT
                                       , "Worker timeout detected!"
                                       );
    }
    else
    {
      SDPA_LOG_ERROR("Invalid communication handler!");
    }
  }
}
*/
