#include <sdpa/daemon/JobManager.hpp>

#include <sdpa/daemon/GenericDaemon.hpp>
#include <sdpa/daemon/Job.hpp>
#include <sdpa/daemon/exceptions.hpp>

#include <boost/foreach.hpp>
#include <boost/range/adaptor/filtered.hpp>
#include <boost/range/adaptor/map.hpp>

namespace sdpa
{
  namespace daemon
  {
    JobManager::~JobManager()
    {
      BOOST_FOREACH (const Job* const pJob, job_map_ | boost::adaptors::map_values )
      {
        delete pJob;
      }

      job_map_.clear();
    }

    Job* JobManager::findJob (const sdpa::job_id_t& job_id) const
    {
      lock_type _ (_job_map_and_requirements_mutex);

      const job_map_t::const_iterator it (job_map_.find( job_id ));
      return it != job_map_.end() ? it->second : NULL;
    }

    void JobManager::addJob ( const sdpa::job_id_t& job_id
                            , const job_desc_t desc
                            , const boost::optional<job_id_t> &parent
                            , bool is_master_job
                            , const worker_id_t& owner
                            , const job_requirements_t& job_req_list
                            )
    {
      lock_type _ (_job_map_and_requirements_mutex);

      DMLOG (TRACE, "Add new job into the job manager");

      Job* pJob = new Job( job_id, desc, parent, is_master_job, owner );

      job_map_.insert(std::make_pair (job_id, pJob));

      if (!job_req_list.empty())
        job_requirements_.insert(std::make_pair(job_id, job_req_list));
    }

    void JobManager::deleteJob (const sdpa::job_id_t& job_id)
    {
      lock_type _ (_job_map_and_requirements_mutex);

      job_requirements_.erase (job_id);

      delete findJob(job_id );
      if (!job_map_.erase (job_id))
      {
        throw JobNotDeletedException(job_id);
      }
    }

    //! \todo Why doesn't every job have an entry here?
    const job_requirements_t JobManager::getJobRequirements
      (const sdpa::job_id_t& jobId) const
    {
      lock_type _ (_job_map_and_requirements_mutex);

      const requirements_map_t::const_iterator it (job_requirements_.find (jobId));
      return it != job_requirements_.end() ? it->second : job_requirements_t();
    }

    void JobManager::addJobRequirements
      (const sdpa::job_id_t& job_id, const job_requirements_t& job_req_list)
    {
      lock_type _ (_job_map_and_requirements_mutex);

      job_requirements_.insert (std::make_pair (job_id, job_req_list));
    }

    void JobManager::resubmitResults (GenericDaemon* pComm) const
    {
      lock_type _ (_job_map_and_requirements_mutex);

      BOOST_FOREACH ( Job* job
                    , job_map_
                    | boost::adaptors::map_values
                    | boost::adaptors::filtered (boost::mem_fn (&Job::isMasterJob))
                    )
      {
        switch (job->getStatus())
        {
        case sdpa::status::FINISHED:
          {
            sdpa::events::JobFinishedEvent::Ptr pEvtJobFinished
              ( new sdpa::events::JobFinishedEvent
                (pComm->name(), job->owner(), job->id(), job->result())
              );
            pComm->sendEventToOther(pEvtJobFinished);
          }
          break;

        case sdpa::status::FAILED:
          {
            sdpa::events::JobFailedEvent::Ptr pEvtJobFailed
              ( new sdpa::events::JobFailedEvent
                (pComm->name(), job->owner(), job->id())
              );
            pComm->sendEventToOther(pEvtJobFailed);
          }
          break;

        case sdpa::status::CANCELED:
          {
            sdpa::events::CancelJobAckEvent::Ptr pEvtJobCanceled
              ( new sdpa::events::CancelJobAckEvent
                (pComm->name(), job->owner(), job->id())
              );
            pComm->sendEventToOther(pEvtJobCanceled);
          }
          break;

        case sdpa::status::PENDING:
          {
            sdpa::events::SubmitJobAckEvent::Ptr pSubmitJobAckEvt
              ( new sdpa::events::SubmitJobAckEvent
                (pComm->name(), job->owner(), job->id())
              );
            pComm->sendEventToOther(pSubmitJobAckEvt);
          }
          break;

        case sdpa::status::RUNNING:
        case sdpa::status::STALLED:
        case sdpa::status::CANCELING:
          // don't send anything to the master if the job is not completed or in a pending state
          break;
        default:
          throw std::runtime_error("The job "+job->id()+" has an invalid/unknown state");
        }
      }
    }

    bool JobManager::hasJobs() const
    {
      lock_type _ (_job_map_and_requirements_mutex);

      return !job_map_.empty();
    }

    bool JobManager::noChildJobStalled(const sdpa::job_id_t& jobId) const
    {
      lock_type lock(_job_map_and_requirements_mutex);
      BOOST_FOREACH (const Job* const pJob, job_map_ | boost::adaptors::map_values )
      {
        assert(pJob);
        if( pJob->parent()==jobId && pJob->getStatus()==sdpa::status::STALLED )
            return false;
      }

      BOOST_FOREACH (const Job* const pJob, job_map_ | boost::adaptors::map_values )
      {
        assert(pJob);
        if( pJob->parent()==jobId && !pJob->completed()  )
          return true;
      }

      return false;
    }

    bool JobManager::noChildJobRunning(const sdpa::job_id_t& jobId) const
    {
     lock_type lock(_job_map_and_requirements_mutex);
     BOOST_FOREACH (const Job* const pJob, job_map_ | boost::adaptors::map_values )
     {
       assert(pJob);
       if( pJob->parent()==jobId && pJob->getStatus()==sdpa::status::RUNNING )
           return false;
     }

     BOOST_FOREACH (const Job* const pJob, job_map_ | boost::adaptors::map_values )
     {
       assert(pJob);
       if( pJob->parent()==jobId && !pJob->completed()  )
         return true;
     }

     return false;
    }
  }
}
