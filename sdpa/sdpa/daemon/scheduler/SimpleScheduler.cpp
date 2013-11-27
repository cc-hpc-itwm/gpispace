// tiberiu.rotaru@itwm.fraunhofer.de
#include <sdpa/daemon/scheduler/SimpleScheduler.hpp>

#include <sdpa/daemon/GenericDaemon.hpp>

using namespace sdpa::daemon;
using namespace sdpa::events;
using namespace std;

SimpleScheduler::SimpleScheduler(GenericDaemon* pCommHandler)
  : SchedulerBase (pCommHandler)
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

    const job_requirements_t job_reqs
      (ptr_comm_handler_->getJobRequirements (jobId));

    const sdpa::worker_id_t matchingWorkerId
      (findSuitableWorker(job_reqs, listAvailWorkers));

    if( !matchingWorkerId.empty() )
    { // matching found
        LOG(INFO, "Serve the job "<<jobId<<" to the worker "<<matchingWorkerId);

        try {
           Worker::ptr_t pWorker(findWorker(matchingWorkerId));
           ptr_comm_handler_->serveJob(matchingWorkerId, jobId);
           pWorker->submit(jobId);
           ptr_comm_handler_->resume(jobId);
        }
        catch(const WorkerNotFoundException&) {
           DMLOG (TRACE, "The worker " << matchingWorkerId << " is not registered! Sending a notification ...");
           ErrorEvent::Ptr pErrorEvt(new ErrorEvent(m_agent_name, matchingWorkerId, ErrorEvent::SDPA_EWORKERNOTREG, "not registered") );
           ptr_comm_handler_->sendEventToSlave(pErrorEvt);
           ptr_comm_handler_->pause(jobId);
        }
    }
    else { // put it back into the common queue
        nonmatching_jobs_queue.push(jobId);
        ptr_comm_handler_->pause(jobId);
    }
  }

  while(!nonmatching_jobs_queue.empty())
      schedule_first(nonmatching_jobs_queue.pop_back());
}

void SimpleScheduler::rescheduleJob(const sdpa::job_id_t& job_id )
{
  Job::ptr_t pJob = ptr_comm_handler_->findJob(job_id);
  if(pJob)
  {
    if( !pJob->completed())
    {
      pJob->Reschedule(this); // put the job back into the pending state
    }
  }
  else
  {
      SDPA_LOG_WARN("Cannot re-schedule the job " << job_id << ". The job could not be found!");
  }
}

boost::optional<sdpa::worker_id_t> SimpleScheduler::getAssignedWorker(const sdpa::job_id_t& jid)
{
  try {
      return boost::optional<sdpa::worker_id_t>(findWorker(jid));
  }
  catch(const NoWorkerFoundException& )
  {
      return boost::optional<sdpa::worker_id_t>();
  }
}
