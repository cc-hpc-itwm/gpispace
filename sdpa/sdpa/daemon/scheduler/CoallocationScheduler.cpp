// tiberiu.rotaru@itwm.fraunhofer.de

#include <sdpa/daemon/scheduler/CoallocationScheduler.hpp>

#include <sdpa/daemon/GenericDaemon.hpp>

namespace sdpa
{
  namespace daemon
  {
    CoallocationScheduler::CoallocationScheduler
        (GenericDaemon* pCommHandler, bool TEST_without_threads)
      : ptr_comm_handler_ ( pCommHandler
                          ? pCommHandler
                          : throw std::runtime_error
                            ("CoallocationScheduler ctor with NULL ptr_comm_handler")
                          )
      , _worker_manager()
    {
      if (!TEST_without_threads)
      {
        m_thread_feed = boost::thread (&CoallocationScheduler::feedWorkers, this);
      }
    }

    CoallocationScheduler::~CoallocationScheduler()
    {
      m_thread_feed.interrupt();

      if (m_thread_feed.joinable())
      {
        m_thread_feed.join();
      }
    }

    void CoallocationScheduler::request_scheduling()
    {
      cond_feed_workers.notify_one();
    }

    bool CoallocationScheduler::addWorker
      ( const Worker::worker_id_t& workerId
      , const boost::optional<unsigned int>& capacity
      , const capabilities_set_t& cpbset
      )
    {
      boost::recursive_mutex::scoped_lock const _ (mtx_);
      return _worker_manager.addWorker (workerId, capacity, cpbset);
    }

    void CoallocationScheduler::rescheduleWorkerJob
      (const Worker::worker_id_t& worker_id, const sdpa::job_id_t& job_id)
    {
      boost::recursive_mutex::scoped_lock const _ (mtx_);

      deleteWorkerJob (worker_id, job_id);

      Job* pJob = ptr_comm_handler_->findJob (job_id);
      if (pJob && !sdpa::status::is_terminal (pJob->getStatus()))
      {
        releaseReservation (job_id);
        pJob->Reschedule(); // put the job back into the pending state
        enqueueJob (job_id);
        request_scheduling();
      }
    }

    void CoallocationScheduler::deleteWorker (const Worker::worker_id_t& worker_id)
    {
      boost::recursive_mutex::scoped_lock const _ (mtx_);
      // mark the worker dirty -> don't take it in consideration for re-scheduling
      const Worker::ptr_t pWorker = findWorker (worker_id);
      pWorker->set_disconnected (true);

      BOOST_FOREACH (sdpa::job_id_t jobId, pWorker->getJobListAndCleanQueues())
      {
        rescheduleWorkerJob (worker_id, jobId);
      }

      _worker_manager.deleteWorker (worker_id);
    }

    void CoallocationScheduler::delete_job (sdpa::job_id_t const& job)
    {
      boost::recursive_mutex::scoped_lock const _ (mtx_);
      if (!_common_queue.erase(job))
      {
        _worker_manager.deleteJob (job);
      }
    }

    void CoallocationScheduler::enqueueJob (const sdpa::job_id_t& jobId)
    {
      boost::recursive_mutex::scoped_lock const _ (mtx_);
      _common_queue.push (jobId);
    }

    Worker::ptr_t CoallocationScheduler::findWorker
      (const Worker::worker_id_t& worker_id)
    {
      return _worker_manager.findWorker (worker_id);
    }

    const boost::optional<Worker::worker_id_t>
      CoallocationScheduler::findSubmOrAckWorker
        (const sdpa::job_id_t& job_id) const
    {
      return _worker_manager.findSubmOrAckWorker (job_id);
    }

    void CoallocationScheduler::feedWorkers()
    {
      for (;;)
      {
        boost::recursive_mutex::scoped_lock lock (mtx_);
        cond_feed_workers.wait (lock);

        assignJobsToWorkers();
      }
    }

    void CoallocationScheduler::acknowledgeJob
      (const Worker::worker_id_t& worker_id, const sdpa::job_id_t& job_id)
    {
      boost::recursive_mutex::scoped_lock const _ (mtx_);
      Worker::ptr_t ptrWorker = findWorker(worker_id);

      if (!ptrWorker->acknowledge(job_id))
      {
        throw JobNotFoundException();
      }
    }

    void CoallocationScheduler::deleteWorkerJob
      (const Worker::worker_id_t& worker_id, const sdpa::job_id_t& jobId)
    {
      boost::recursive_mutex::scoped_lock const _ (mtx_);

      try
      {
        _worker_manager.findWorker (worker_id)->deleteJob (jobId);
        request_scheduling();
      }
      catch (WorkerNotFoundException const&)
      {
      }
    }

    bool CoallocationScheduler::addCapabilities
      (const sdpa::worker_id_t& worker_id, const sdpa::capabilities_set_t& cpbset)
    {
      boost::recursive_mutex::scoped_lock const _ (mtx_);
      return _worker_manager.findWorker (worker_id)->addCapabilities (cpbset);
    }

    void CoallocationScheduler::removeCapabilities
      (const sdpa::worker_id_t& worker_id, const sdpa::capabilities_set_t& cpbset)
    {
      _worker_manager.findWorker (worker_id)->removeCapabilities (cpbset);
    }

    void CoallocationScheduler::getAllWorkersCapabilities
      (sdpa::capabilities_set_t& cpbset)
    {
      _worker_manager.getCapabilities (cpbset);
    }

    sdpa::capabilities_set_t CoallocationScheduler::getWorkerCapabilities
      (const sdpa::worker_id_t& worker_id)
    {
      boost::recursive_mutex::scoped_lock const _ (mtx_);
      return findWorker (worker_id)->capabilities();
    }

    void CoallocationScheduler::assignJobsToWorkers()
    {
      boost::recursive_mutex::scoped_lock const _ (mtx_);

      sdpa::worker_id_list_t listAvailWorkers
        (_worker_manager.getListWorkersNotReserved());

      std::list<sdpa::job_id_t> nonmatching_jobs_queue;

      while (!_common_queue.empty() && !listAvailWorkers.empty())
      {
        sdpa::job_id_t jobId (_common_queue.pop());

        const job_requirements_t job_reqs
          (ptr_comm_handler_->getJobRequirements (jobId));

        const boost::optional<sdpa::worker_id_t> matchingWorkerId
          ( listAvailWorkers.empty() ? boost::none
          : job_reqs.empty() ? listAvailWorkers.front()
          : _worker_manager.getBestMatchingWorker (job_reqs, listAvailWorkers)
         );

        if (matchingWorkerId)
        {
          listAvailWorkers.erase ( std::find ( listAvailWorkers.begin()
                                             , listAvailWorkers.end()
                                             , *matchingWorkerId
                                             )
                                 );

          boost::mutex::scoped_lock const _ (mtx_alloc_table_);
          {
            _worker_manager.findWorker (*matchingWorkerId)->reserve();

            allocation_table_t::iterator it (allocation_table_.find(jobId));
            if (it == allocation_table_.end())
            {
              Reservation* pReservation
                (new Reservation (jobId, job_reqs.numWorkers()));
              allocation_table_t::value_type pairJobRes (jobId, pReservation);
              allocation_table_.insert (pairJobRes);
            }

            allocation_table_[jobId]->addWorker (*matchingWorkerId);
          }

          Reservation* pReservation (allocation_table_[jobId]);

          if (pReservation->acquired())
          {
            sdpa::worker_id_list_t list_reserved_workers =
              pReservation->getWorkerList();

            sdpa::worker_id_list_t list_invalid_workers;
            BOOST_FOREACH (const Worker::worker_id_t& wid, list_reserved_workers)
            {
              if (!_worker_manager.hasWorker (wid))
              {
                list_invalid_workers.push_back (wid);
              }
            }

            if (list_invalid_workers.empty())
            {
              BOOST_FOREACH (const Worker::worker_id_t& wid, list_reserved_workers)
              {
                _worker_manager.findWorker (wid)->submit (jobId);
              }
              ptr_comm_handler_->serveJob (list_reserved_workers, jobId);
            }
            else
            {
              BOOST_FOREACH (const Worker::worker_id_t& wid, list_invalid_workers)
              {
                pReservation->delWorker (wid);
              }

              _common_queue.push_front (jobId);
            }
          }
          else
          {
            _common_queue.push_front (jobId);
          }
        }
        else
        {
          nonmatching_jobs_queue.push_back (jobId);
        }
      }

      BOOST_FOREACH (const sdpa::job_id_t& id, nonmatching_jobs_queue)
      {
        _common_queue.push (id);
      }
    }

    void CoallocationScheduler::releaseReservation (const sdpa::job_id_t& jobId)
    {
      boost::mutex::scoped_lock const _ (mtx_alloc_table_);

      const allocation_table_t::const_iterator it
        (allocation_table_.find (jobId));

      if (it != allocation_table_.end())
      {
        boost::recursive_mutex::scoped_lock const _ (mtx_);
        Reservation* pReservation (it->second);
        worker_id_list_t listWorkers (pReservation->getWorkerList());
        BOOST_FOREACH (sdpa::worker_id_t const& workerId, listWorkers)
        {
          findWorker (workerId)->free();
        }

        delete it->second;
        allocation_table_.erase (it);
      }
    }

    void CoallocationScheduler::workerFinished
      (const worker_id_t& wid, const job_id_t& jid)
    {
      boost::mutex::scoped_lock const _ (mtx_alloc_table_);
      allocation_table_t::iterator it = allocation_table_.find (jid);
      if (it != allocation_table_.end())
      {
        it->second->storeWorkerResult (wid, Reservation::FINISHED);
      }
      else
      {
        throw JobNotFoundException();
      }
    }

    void CoallocationScheduler::workerFailed
      (const worker_id_t& wid, const job_id_t& jid)
    {
      boost::mutex::scoped_lock const _ (mtx_alloc_table_);
      allocation_table_t::iterator it = allocation_table_.find (jid);
      if (it != allocation_table_.end())
      {
        it->second->storeWorkerResult (wid, Reservation::FAILED);
      }
      else
      {
        throw JobNotFoundException();
      }
    }

    void CoallocationScheduler::workerCanceled
      (const worker_id_t& wid, const job_id_t& jid)
    {
      boost::mutex::scoped_lock const _ (mtx_alloc_table_);
      allocation_table_t::iterator it = allocation_table_.find (jid);
      if (it != allocation_table_.end())
      {
        it->second->storeWorkerResult (wid, Reservation::CANCELED);
      }
      else
      {
        throw JobNotFoundException();
      }
    }

    bool CoallocationScheduler::allPartialResultsCollected (const job_id_t& jid)
    {
      boost::mutex::scoped_lock const _ (mtx_alloc_table_);
      allocation_table_t::iterator it = allocation_table_.find (jid);
      if (it != allocation_table_.end())
      {
        return it->second->allWorkersTerminated();
      }
      else
      {
        throw JobNotFoundException();
      }
    }

    bool CoallocationScheduler::groupFinished (const sdpa::job_id_t& jid)
    {
      boost::mutex::scoped_lock const _ (mtx_alloc_table_);
      allocation_table_t::iterator it = allocation_table_.find (jid);
      if (it != allocation_table_.end())
      {
        return it->second->allGroupTasksFinishedSuccessfully();
      }
      else
      {
        throw JobNotFoundException();
      }
    }
  }
}
