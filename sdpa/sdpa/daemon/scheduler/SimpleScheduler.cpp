// tiberiu.rotaru@itwm.fraunhofer.de
#include <sdpa/daemon/scheduler/SimpleScheduler.hpp>
#include <sdpa/daemon/GenericDaemon.hpp>

namespace sdpa {
  namespace daemon {

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
  std::list<sdpa::job_id_t> nonmatching_jobs_queue;

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
      DLLOG (TRACE, _logger, "Serve the job "<<jobId<<" to the worker "<<matchingWorkerId);

        try {
           Worker::ptr_t pWorker(findWorker(matchingWorkerId));
           //ptr_comm_handler_->resume(jobId);
           ptr_comm_handler_->serveJob(sdpa::worker_id_list_t (1, matchingWorkerId), jobId);
           pWorker->submit(jobId);
        }
        catch(const WorkerNotFoundException&) {
          DLLOG (TRACE, _logger, "The worker " << matchingWorkerId << " is not registered! Sending a notification ...");
           sdpa::events::ErrorEvent::Ptr pErrorEvt(
               new sdpa::events::ErrorEvent(m_agent_name,
                                             matchingWorkerId,
                                             sdpa::events::ErrorEvent::SDPA_EWORKERNOTREG,
                                             "not registered") );
           ptr_comm_handler_->sendEventToOther(pErrorEvt);
           ptr_comm_handler_->pause(jobId);
        }
    }
    else { // put it back into the common queue
        ptr_comm_handler_->pause(jobId);
        nonmatching_jobs_queue.push_back(jobId);
    }
  }

  BOOST_REVERSE_FOREACH (const sdpa::job_id_t& id, nonmatching_jobs_queue)
  {
    schedule_first (id);
  }
}

void SimpleScheduler::rescheduleJob(const sdpa::job_id_t& job_id )
{
  Job* pJob = ptr_comm_handler_->findJob(job_id);
  if(pJob)
  {
    if( !pJob->completed())
    {
      pJob->Reschedule(this); // put the job back into the pending state
    }
  }
  else
  {
    LLOG (WARN, _logger, "Cannot re-schedule the job " << job_id << ". The job could not be found!");
  }
}

boost::optional<sdpa::worker_id_t> SimpleScheduler::getAssignedWorker(const sdpa::job_id_t& jid)
{
  return _worker_manager.getAssignedWorker(jid);
}

}}
