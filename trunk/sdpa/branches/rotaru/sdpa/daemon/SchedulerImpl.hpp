#ifndef SDPA_SCHEDULERIMPL_HPP
#define SDPA_SCHEDULERIMPL_HPP 1

#include <sdpa/daemon/Scheduler.hpp>
#include <sdpa/daemon/JobManager.hpp>
#include <sdpa/daemon/WorkerManager.hpp>

namespace sdpa {
	namespace daemon {
  class SchedulerImpl : public Scheduler {

  public:

	 typedef sdpa::shared_ptr<SchedulerImpl> ptr_t;

	 SchedulerImpl(const  JobManager::ptr_t, const WorkerManager::ptr_t );
	 virtual ~SchedulerImpl();

    /**
      The scheduler should somehow keep track of the jobs that he has assigned to the nodes.

      The acknowledge of a job_id means that the job has been successfully sent to a worker node.
     */
    virtual void acknowledge(const sdpa::job_id_t& job_id );

    /**
      Retrieve the next available job for the given worker node.

      The job returned depends on the last_job_id passed in as a  parameter,  if
      a prior submission to the node failed, we have to send him  the  same  job
      again, if the submission was successful (last_job_id  could  be  found  in
      the  pending  schedule),  we  are  allowed  to  send  him   a   new   job.

      Note: synchronization should be done on the worker-node entries, not the scheduler itself.

      @param worker_id the identification code of a worker that is requesting a new job
      @param last_job_id the most recent received job_id on the worker side
     */
    virtual Job::ptr_t get_next_job(const Worker::worker_id_t &worker_id, const sdpa::job_id_t &last_job);

    /**
      Scheduling a job locally means that it will not leave the executing node.

      On the Orchestrator that means the initial execution of a workflow,
      resulting activities are then scheduled via the schedule() call.

      @param job a pointer to the job that is to be scheduled locally
      */
    virtual void schedule_local(const Job::ptr_t &job);

    /**
      Scheduling a job means that it will be put into a special queue for some worker node (Aggregator or NRE).

      @param job a pointer to the job that shall be executed on a remote node
      */
    virtual void schedule(const Job::ptr_t &job);

  private:
	  JobManager::ptr_t ptr_job_man_;
	  WorkerManager::ptr_t ptr_worker_man_;
  };
}}

#endif
