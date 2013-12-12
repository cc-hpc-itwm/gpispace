/*
 * =====================================================================================
 *
 *       Filename:  SchedulerBase.hpp
 *
 *    Description:  Defines scheduler class
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
#ifndef SDPA_SchedulerBase_HPP
#define SDPA_SchedulerBase_HPP 1

#include <boost/thread.hpp>
#include <sdpa/daemon/Job.hpp>
#include <sdpa/daemon/Worker.hpp>
#include <sdpa/daemon/exceptions.hpp>
#include <sdpa/events/ErrorEvent.hpp>

#include <sdpa/engine/IWorkflowEngine.hpp>
#include <sdpa/daemon/scheduler/Reservation.hpp>

#include <boost/optional.hpp>
#include <sdpa/daemon/JobManager.hpp>
#include <sdpa/daemon/WorkerManager.hpp>
//#include <sdpa/daemon/SynchronizedQueue.hpp>

#include <boost/optional.hpp>

namespace sdpa {
  namespace daemon {
    class GenericDaemon;
    class SchedulerBase
    {
    public:
      typedef boost::shared_ptr<SchedulerBase> ptr_t;
      typedef SynchronizedQueue<std::list<sdpa::job_id_t> > JobQueue;
      typedef boost::recursive_mutex mutex_type;
      typedef boost::unique_lock<mutex_type> lock_type;
      typedef boost::condition_variable_any condition_type;

      SchedulerBase(GenericDaemon* pHandler);
      virtual ~SchedulerBase();

      void enqueueJob(const sdpa::job_id_t&);
      void schedule(const sdpa::job_id_t&);
      void delete_job(const sdpa::job_id_t&);
      virtual void assignJobsToWorkers() = 0;

      void schedule_first(const sdpa::job_id_t&);

      void rescheduleWorkerJob( const Worker::worker_id_t&, const sdpa::job_id_t&);
      virtual void rescheduleJob(const sdpa::job_id_t&) = 0;
      void reschedule( const Worker::worker_id_t&, sdpa::job_id_list_t& );
      bool has_job(const sdpa::job_id_t&);

      bool hasWorker(const Worker::worker_id_t&) const;
      Worker::ptr_t findWorker(const Worker::worker_id_t&);
      const boost::optional<Worker::worker_id_t> findSubmOrAckWorker(const sdpa::job_id_t& job_id) const;

      void addWorker( const Worker::worker_id_t& workerId,
                              const boost::optional<unsigned int>& capacity = boost::none,
			      const capabilities_set_t& cpbset = capabilities_set_t(),
			      const unsigned int& agent_rank = 0,
                    const sdpa::worker_id_t& agent_uuid = "");

      void deleteWorker( const Worker::worker_id_t& workerId);

      void getWorkerList(sdpa::worker_id_list_t&);
      void getListNotFullWorkers(sdpa::worker_id_list_t& workerList);

      size_t numberOfWorkers() { return _worker_manager.numberOfWorkers(); }

      bool addCapabilities(const sdpa::worker_id_t&, const sdpa::capabilities_set_t& cpbset);
      void removeCapabilities(const sdpa::worker_id_t&, const sdpa::capabilities_set_t& cpbset);
      void getAllWorkersCapabilities(sdpa::capabilities_set_t& cpbset);
      void getWorkerCapabilities(const sdpa::worker_id_t&, sdpa::capabilities_set_t& cpbset);

      void deleteWorkerJob(const Worker::worker_id_t& worker_id, const sdpa::job_id_t &job_id );

      sdpa::worker_id_t findSuitableWorker(const job_requirements_t&, sdpa::worker_id_list_t&);

      void acknowledgeJob(const Worker::worker_id_t& worker_id, const sdpa::job_id_t& job_id);

      bool schedulingAllowed() { return !_worker_manager.common_queue_.empty(); }
      job_id_t nextJobToSchedule() { return _worker_manager.common_queue_.pop(); }

      //! \note This is required to be called after the ctor, as the
      //! threads use virtual functions, which are pure-virtual during
      //! the ctor, thus there is a race if the ctor of derived
      //! classes or the thread run first.
      void start_threads();

    private:
      void feedWorkers();
      void run();

    protected:
      JobQueue pending_jobs_queue_;
      WorkerManager _worker_manager;

      GenericDaemon* ptr_comm_handler_;
      SDPA_DECLARE_LOGGER();

      mutable mutex_type mtx_;
      condition_type cond_feed_workers;
      condition_type cond_workers_registered;

      sdpa::agent_id_t m_agent_name;

      boost::thread m_thread_run;
      boost::thread m_thread_feed;
    };
  }
}

#endif
