// tiberiu.rotaru@itwm.fraunhofer.de
#include <sdpa/daemon/scheduler/SimpleScheduler.hpp>

using namespace sdpa::daemon;
using namespace sdpa::events;
using namespace std;

SimpleScheduler::SimpleScheduler(sdpa::daemon::IAgent* pCommHandler)
  : SchedulerBase (pCommHandler)
  , b_send_job_to_workers_ (true)
{}

void SimpleScheduler::assignJobsToWorkers()
{
  lock_type lock(mtx_);
  sdpa::worker_id_list_t listAvailWorkers;

  if(!schedulingAllowed())
    return;

  // get the list of workers that are not full
  getListNotFullWorkers(listAvailWorkers);

  // check if there are jobs that can already be scheduled on these workers
  JobQueue nonmatching_jobs_queue;

  // iterate over all jobs and see if there is one that prefers
  while(schedulingAllowed() && !listAvailWorkers.empty())
  {
    sdpa::job_id_t jobId(nextJobToSchedule());
    sdpa::worker_id_t matchingWorkerId;

    try {
      job_requirements_t job_reqs(ptr_comm_handler_->getJobRequirements(jobId));
      matchingWorkerId = findSuitableWorker(job_reqs, listAvailWorkers);
    }
    catch( const NoJobRequirements& ex ) { // no requirements are specified
      // we have an empty list of requirements then!
      matchingWorkerId = listAvailWorkers.front();
      listAvailWorkers.erase(listAvailWorkers.begin());
    }

    if( !matchingWorkerId.empty() ) { // matching found
        LOG(INFO, "Serve the job "<<jobId<<" to the worker "<<matchingWorkerId);

        // serve the same job to all reserved workers!!!!
        if(b_send_job_to_workers_)
          ptr_comm_handler_->serveJob(matchingWorkerId, jobId);
        //ptr_comm_handler_->resume(jobId);
    }
    else { // put it back into the common queue
        nonmatching_jobs_queue.push(jobId);
        //ptr_comm_handler_->pause(jobId);
    }
  }

  while(!nonmatching_jobs_queue.empty())
      schedule_first(nonmatching_jobs_queue.pop_back());
}

void SimpleScheduler::rescheduleJob(const sdpa::job_id_t& job_id )
{
  if(bStopRequested)
  {
      SDPA_LOG_WARN("The scheduler is requested to stop. Job re-scheduling is not anymore possible.");
      return;
  }

  try {
      Job::ptr_t pJob = ptr_comm_handler_->findJob(job_id);
      if( !pJob->completed()) {
          pJob->Reschedule(ptr_comm_handler_); // put the job back into the pending state
      }
  }
  catch(JobNotFoundException const &ex)
  {
    SDPA_LOG_WARN("Cannot re-schedule the job " << job_id << ". The job could not be found!");
  }
  catch(const std::exception& ex) {
    SDPA_LOG_WARN( "Could not re-schedule the job " << job_id << ": unexpected error!"<<ex.what() );
  }
}

void SimpleScheduler::releaseReservation(const sdpa::job_id_t& jobId) { throw std::runtime_error ("Not implemented!");}
void SimpleScheduler::workerFinished(const worker_id_t& wid, const job_id_t& jid) { throw std::runtime_error ("Not implemented!");}
void SimpleScheduler::workerFailed(const worker_id_t& wid, const job_id_t& jid) { throw std::runtime_error ("Not implemented!");}
void SimpleScheduler::workerCanceled(const worker_id_t& wid, const job_id_t& jid) { throw std::runtime_error ("Not implemented!");}
bool SimpleScheduler::allPartialResultsCollected(const job_id_t& jid) { throw std::runtime_error ("Not implemented!");}
bool SimpleScheduler::groupFinished(const sdpa::job_id_t& jid) { throw std::runtime_error ("Not implemented!");}
