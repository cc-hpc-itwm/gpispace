// tiberiu.rotaru@itwm.fraunhofer.de

#include <sdpa/daemon/scheduler/CoallocationScheduler.hpp>

#include <sdpa/daemon/GenericDaemon.hpp>

namespace sdpa
{
  namespace daemon
  {
    CoallocationScheduler::CoallocationScheduler (GenericDaemon* pCommHandler)
      : ptr_comm_handler_ ( pCommHandler
                          ? pCommHandler
                          : throw std::runtime_error
                            ("CoallocationScheduler ctor with NULL ptr_comm_handler")
                          )
      , _worker_manager()
    {}

    const WorkerManager& CoallocationScheduler::worker_manager() const
    {
      return _worker_manager;
    }
    WorkerManager& CoallocationScheduler::worker_manager()
    {
      return _worker_manager;
    }

    bool CoallocationScheduler::delete_job (sdpa::job_id_t const& job)
    {
      return _common_queue.erase (job);
    }

    void CoallocationScheduler::enqueueJob (const sdpa::job_id_t& jobId)
    {
      _common_queue.push (jobId);
    }

    void CoallocationScheduler::assignJobsToWorkers()
    {
      sdpa::worker_id_list_t listAvailWorkers
        (worker_manager().getListWorkersNotReserved());

      std::list<sdpa::job_id_t> nonmatching_jobs_queue;

      while (!_common_queue.empty() && !listAvailWorkers.empty())
      {
        sdpa::job_id_t jobId (_common_queue.pop());

        const job_requirements_t job_reqs
          (ptr_comm_handler_->getJobRequirements (jobId));

        const boost::optional<sdpa::worker_id_t> matchingWorkerId
          ( listAvailWorkers.empty() ? boost::none
          : job_reqs.empty() ? listAvailWorkers.front()
          : worker_manager().getBestMatchingWorker (job_reqs, listAvailWorkers)
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
            worker_manager().findWorker (*matchingWorkerId)->reserve();

            allocation_table_t::iterator it (allocation_table_.find(jobId));
            if (it == allocation_table_.end())
            {
              Reservation* pReservation (new Reservation (job_reqs.numWorkers()));
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
              if (!worker_manager().hasWorker (wid))
              {
                list_invalid_workers.push_back (wid);
              }
            }

            if (list_invalid_workers.empty())
            {
              BOOST_FOREACH (const Worker::worker_id_t& wid, list_reserved_workers)
              {
                worker_manager().findWorker (wid)->submit (jobId);
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
        BOOST_FOREACH ( sdpa::worker_id_t const& workerId
                      , it->second->getWorkerList()
                      )
        {
          worker_manager().findWorker (workerId)->free();
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
