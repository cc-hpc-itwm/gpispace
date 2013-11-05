/*
 * =====================================================================================
 *
 *       Filename:  SchedulerAgg.hpp
 *
 *    Description:  The aggregator's scheduler
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
#ifndef SDPA_SCHEDULERAGG_HPP
#define SDPA_SCHEDULERAGG_HPP 1

#include <sdpa/daemon/SchedulerImpl.hpp>
#include <sdpa/events/RequestJobEvent.hpp>
#include <sdpa/events/LifeSignEvent.hpp>

using namespace sdpa::events;
using namespace std;

namespace sdpa {
  namespace daemon {
    class AgentScheduler : public SchedulerImpl
    {
    public:
      typedef boost::unordered_map<sdpa::job_id_t, Reservation> allocation_table_t;

      AgentScheduler(sdpa::daemon::IAgent* pCommHandler = NULL,  bool use_request_model=true)
        : SchedulerImpl(pCommHandler, use_request_model),
          SDPA_INIT_LOGGER(pCommHandler?pCommHandler->name()+"::Scheduler":"Scheduler")
      {}

      virtual ~AgentScheduler()
      {
        try {
            LOG(TRACE, "destructing SchedulerAgg");
            stop();
        }
        catch (std::exception const & ex) {
            LOG(ERROR, "could not stop SchedulerAgg: " << ex.what());
        }
      }

      void assignJobsToWorkers()
      {
        lock_type lock(mtx_);
        sdpa::worker_id_list_t listAvailWorkers;

        if(!schedulingAllowed())
          return;

        // replace this with the list of workers not reserved
        //getListNotFullWorkers(listAvailWorkers);
        getListNotAllocatedWorkers(listAvailWorkers);

        // check if there are jobs that can already be scheduled on
        // these workers
        JobQueue nonmatching_jobs_queue;

        // iterate over all jobs and see if there is one that prefers
        while(schedulingAllowed() && !listAvailWorkers.empty())
        {
          sdpa::job_id_t jobId(nextJobToSchedule());

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
              // serve the same job to all reserved workers!!!!
              ptr_comm_handler_->serveJob(reservation);
            }
            else
              schedule_first(jobId);
          }
          else // put it back into the common queue
          {
              nonmatching_jobs_queue.push(jobId);
          }
        }

        while(!nonmatching_jobs_queue.empty())
          schedule_first(nonmatching_jobs_queue.pop_back());
      }

      void rescheduleJob(const sdpa::job_id_t& job_id )
      {
        if(bStopRequested) {
            SDPA_LOG_WARN("The scheduler is requested to stop. Job re-scheduling is not anymore possible.");
            return;
        }

        ostringstream os;
        if(!ptr_comm_handler_) {
            SDPA_LOG_ERROR("Invalid communication handler. ");
            stop();
            return;
        }

        try
        {
            Job::ptr_t pJob = ptr_comm_handler_->findJob(job_id);
            if(!pJob->completed()) {
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

      void reserveWorker(const sdpa::job_id_t& jobId, const sdpa::worker_id_t& matchingWorkerId, const size_t& cap) throw( WorkerReservationFailed)
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

      void releaseReservation(const sdpa::job_id_t& jobId)
      {
        lock_type lock_table(mtx_alloc_table_);

        // if the status is not terminal
        try {
            allocation_table_t::const_iterator it = allocation_table_.find(jobId);

            // if there are allocated resources
            if(it==allocation_table_.end()) {
                LOG(WARN, "No reservation was found for the job "<<jobId);
                return;
            }

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

      void getListNotAllocatedWorkers(sdpa::worker_id_list_t& workerList)
      {
        workerList.clear();
        ptr_worker_man_->getListWorkersNotReserved(workerList);
      }

      void printAllocationTable()
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

      sdpa::job_id_t getAssignedJob(const sdpa::worker_id_t& wid)
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

      void checkAllocations()
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

      sdpa::worker_id_list_t getListAllocatedWorkers(const sdpa::job_id_t& jobId)
      {
        return allocation_table_[jobId].getWorkerList();
      }

      void workerFinished(const worker_id_t& wid, const job_id_t& jid) {  lock_type lock_table(mtx_alloc_table_); allocation_table_[jid].workerFinished(wid); }
      void workerFailed(const worker_id_t& wid, const job_id_t& jid) {  lock_type lock_table(mtx_alloc_table_); allocation_table_[jid].workerFailed(wid); }
      void workerCanceled(const worker_id_t& wid, const job_id_t& jid) {  lock_type lock_table(mtx_alloc_table_); allocation_table_[jid].workerCanceled(wid); }
      bool allPartialResultsCollected(const job_id_t& jid) {  lock_type lock_table(mtx_alloc_table_); return allocation_table_[jid].allWorkersTerminated(); }
      bool groupFinished(const sdpa::job_id_t& jid)
      {
        lock_type lock(mtx_alloc_table_);
        Reservation reservation(allocation_table_[jid]);
        return reservation.groupFinished();
      }

      friend class boost::serialization::access;

      template <class Archive>
      void serialize(Archive& ar, const unsigned int /* file_version */)
      {
        ar & boost::serialization::base_object<SchedulerImpl>(*this);
      }

    private:
      SDPA_DECLARE_LOGGER();
      mutable mutex_type mtx_alloc_table_;
      allocation_table_t allocation_table_;
  };
}}

#endif
