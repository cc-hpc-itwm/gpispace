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
      //getWorkerList(listAvailWorkers);

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
        catch( const NoJobRequirements& ex ) // no requirements are specified
        {
          // we have an empty list of requirements then!
          matchingWorkerId = listAvailWorkers.front();
          listAvailWorkers.erase(listAvailWorkers.begin());
        }

        if( !matchingWorkerId.empty() ) // matching found
        {
            LOG(INFO, "Serve the job "<<jobId<<" to the worker "<<matchingWorkerId);

            // serve the same job to all reserved workers!!!!
            ptr_comm_handler_->serveJob(matchingWorkerId, jobId);
        }
        else // put it back into the common queue
        {
            nonmatching_jobs_queue.push(jobId);
        }
      }

      reschedule(nonmatching_jobs_queue);
    }

    private:
      SDPA_DECLARE_LOGGER();
    };
  }
}

#endif
