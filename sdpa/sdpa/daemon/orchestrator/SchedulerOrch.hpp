// tiberiu.rotaru@itwm.fraunhofer.de
#ifndef SDPA_SCHEDULERORCH_HPP
#define SDPA_SCHEDULERORCH_HPP 1

#include <sdpa/daemon/SchedulerImpl.hpp>

using namespace sdpa::events;
using namespace std;

namespace sdpa {
  namespace daemon {
    class SchedulerOrch : public SchedulerImpl {

    public:
    SchedulerOrch(sdpa::daemon::IAgent* pCommHandler = NULL,  bool bUseReqModel = true):
           SchedulerImpl(pCommHandler, bUseReqModel),
           SDPA_INIT_LOGGER(pCommHandler?pCommHandler->name()+"::Scheduler":"Scheduler")
    {}

    virtual ~SchedulerOrch()
    {
      try
      {
          stop();
      }
      catch (std::exception const & ex)
      {
          SDPA_LOG_ERROR("could not stop SchedulerOrch: " << ex.what());
      }
    }

    bool postRequest( bool ) { return false; }
    void checkRequestPosted() { /*do nothing*/ }

    template <class Archive>
    void serialize(Archive& ar, const unsigned int)
    {
      ar & boost::serialization::base_object<SchedulerImpl>(*this);
    }

    // the orchestrator needs no reservation!
    void assignJobsToWorkers()
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
            ptr_comm_handler_->serveJob(matchingWorkerId, jobId);
        }
        else { // put it back into the common queue
            nonmatching_jobs_queue.push(jobId);
        }
      }

      while(!nonmatching_jobs_queue.empty())
          schedule_first(nonmatching_jobs_queue.pop_back());
    }

    void rescheduleJob(const sdpa::job_id_t& job_id )
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

      try {

          Job::ptr_t pJob = ptr_comm_handler_->findJob(job_id);
          std::string status = pJob->getStatus();
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

    void releaseReservation(const sdpa::job_id_t& jobId) {LOG(WARN, "Not implemented!");}
    void workerFinished(const worker_id_t& wid, const job_id_t& jid) {LOG(WARN, "Not implemented!");}
    void workerFailed(const worker_id_t& wid, const job_id_t& jid) {LOG(WARN, "Not implemented!");}
    void workerCanceled(const worker_id_t& wid, const job_id_t& jid) {LOG(WARN, "Not implemented!");}
    bool allPartialResultsCollected(const job_id_t& jid) {LOG(WARN, "Not implemented!"); return false;}
    bool groupFinished(const sdpa::job_id_t& jid) {LOG(WARN, "Not implemented!"); return false;}

    private:
      SDPA_DECLARE_LOGGER();
    };
  }
}

#endif
