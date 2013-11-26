/*
 * =====================================================================================
 *
 *       Filename:  JobEventHandlers.cpp
 *
 *    Description:  Implements the handling of job related events
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

#include <fhg/assert.hpp>
#include <seda/StageRegistry.hpp>

#include <sdpa/daemon/GenericDaemon.hpp>
#include <sdpa/daemon/Job.hpp>

#include <sdpa/daemon/exceptions.hpp>

using namespace std;
using namespace sdpa::daemon;
using namespace sdpa::events;

/**
 * Event SubmitJobAckEvent
 * Precondition: an acknowledgment event was received from a master
 * Action: - look for the job into the JobManager
 *         - if the job was found, put the job into the state Running
 *         - move the job from the submitted queue of the worker worker_id, into its
 *           acknowledged queue
 *         - in the case when the worker was not found, trigger an exception and send back
 *           an error message
 * Postcondition: is either into the Running state or inexistent
 */
void GenericDaemon::handleSubmitJobAckEvent(const SubmitJobAckEvent* pEvent)
{
  assert (pEvent);

  DLOG(TRACE, "handleSubmitJobAckEvent: " << pEvent->job_id() << " from " << pEvent->from());

  Worker::worker_id_t worker_id = pEvent->from();
  // Only, now should be state of the job updated to RUNNING
  // since it was not rejected, no error occurred etc ....
  //find the job ptrJob and call
  Job::ptr_t ptrJob = jobManager()->findJob(pEvent->job_id());
  if(ptrJob)
  {
      try
      {
        ptrJob->Dispatch();
        scheduler()->acknowledgeJob(worker_id, pEvent->job_id());
      }
      catch(WorkerNotFoundException const &ex1)
      {
        DMLOG ( WARN,  "job " << pEvent->job_id()
              << " could not be acknowledged:"
              << " worker " << worker_id
              << " not found!"
              );

        // the worker should register first, before posting a job request
        ErrorEvent::Ptr pErrorEvt(new ErrorEvent(name(), worker_id, ErrorEvent::SDPA_EWORKERNOTREG, "not registered") );
        sendEventToSlave(pErrorEvt);
      }
      catch(std::exception const &ex2)
      {
        SDPA_LOG_ERROR( "Unexpected exception during "
                        << " handleSubmitJobAckEvent("<< pEvent->job_id() << ")"
                        << ": "
                        << ex2.what()
                        );

        ErrorEvent::Ptr pErrorEvt(new ErrorEvent(name(), worker_id, ErrorEvent::SDPA_EUNKNOWN, ex2.what()) );
        sendEventToMaster(pErrorEvt);
      }
  }
  else
  {
    SDPA_LOG_ERROR( "job " << pEvent->job_id()
                        << " could not be acknowledged:"
                        << " the job " <<  pEvent->job_id()
                        << " not found!"
                        );

    ErrorEvent::Ptr pErrorEvt(new ErrorEvent(name(), worker_id, ErrorEvent::SDPA_EJOBNOTFOUND, "Could not acknowledge job") );
    sendEventToMaster(pErrorEvt);
  }
}

// respond to a worker that the JobFinishedEvent was received
void GenericDaemon::handleJobFinishedAckEvent(const JobFinishedAckEvent* pEvt)
{
  // The result was successfully delivered by the worker and the WE was notified
  // therefore, I can delete the job from the job map
  ostringstream os;
  Worker::worker_id_t worker_id = pEvt->from();
  DMLOG (TRACE, "Got acknowledgment for the finished job " << pEvt->job_id() << "!");

  if(jobManager()->findJob(pEvt->job_id()))
  {
    try {
      DMLOG (TRACE, "Delete the job " << pEvt->job_id() << " from the JobManager!");
      // delete it from the map when you receive a JobFinishedAckEvent!
      jobManager()->deleteJob(pEvt->job_id());
    }
    catch(JobNotDeletedException const & ex1)
    {
      SDPA_LOG_ERROR("job " << pEvt->job_id() << " could not be deleted: " << ex1.what());

      ErrorEvent::Ptr pErrorEvt(new ErrorEvent(name(), worker_id, ErrorEvent::SDPA_EJOBNOTDELETED, ex1.what()) );
      sendEventToMaster(pErrorEvt);
    }
    catch(std::exception const &ex2)
    {
      SDPA_LOG_ERROR( "Unexpected exception during "
                      << " handleJobFinishedAckEvent("<< pEvt->job_id() << ")"
                      << ": "
                      << ex2.what()
                     );

      ErrorEvent::Ptr pErrorEvt(new ErrorEvent(name(), worker_id, ErrorEvent::SDPA_EUNKNOWN, ex2.what()));
      sendEventToMaster(pErrorEvt);
    }
  }
  else
  {
     SDPA_LOG_ERROR("job " << pEvt->job_id() << " could not be found!");

     ErrorEvent::Ptr pErrorEvt(new ErrorEvent(name(), worker_id, ErrorEvent::SDPA_EJOBNOTFOUND, "Couldn't find the job!") );
     sendEventToMaster(pErrorEvt);
  }
}

// respond to a worker that the JobFailedEvent was received
void GenericDaemon::handleJobFailedAckEvent(const JobFailedAckEvent* pEvt )
{
  ostringstream os;
  Worker::worker_id_t worker_id = pEvt->from();

  if(jobManager()->findJob(pEvt->job_id()))
  {
    try {
        // delete it from the map when you receive a JobFailedAckEvent!
        jobManager()->deleteJob(pEvt->job_id());
    }
    catch(JobNotDeletedException const & ex1)
    {
      SDPA_LOG_ERROR("job " << pEvt->job_id() << " could not be deleted: " << ex1.what());

      ErrorEvent::Ptr pErrorEvt(new ErrorEvent(name(), worker_id, ErrorEvent::SDPA_EJOBNOTDELETED, ex1.what()) );
      sendEventToMaster(pErrorEvt);
    }
    catch(std::exception const &ex2)
    {
      SDPA_LOG_ERROR( "Unexpected exception during "
                      << " handleJobFinishedAckEvent("<< pEvt->job_id() << ")"
                      << ": "
                      << ex2.what()
                     );

      ErrorEvent::Ptr pErrorEvt(new ErrorEvent(name(), worker_id, ErrorEvent::SDPA_EUNKNOWN, ex2.what()));
      sendEventToMaster(pErrorEvt);
    }
  }
  else
  {
    SDPA_LOG_ERROR("job " << pEvt->job_id() << " could not be found!");

    ErrorEvent::Ptr pErrorEvt(new ErrorEvent(name(), worker_id, ErrorEvent::SDPA_EJOBNOTFOUND, "Couldn't find the job!") );
    sendEventToMaster(pErrorEvt);
  }
}

void GenericDaemon::handleQueryJobStatusEvent(const QueryJobStatusEvent* pEvt )
{
  sdpa::job_id_t jobId = pEvt->job_id();

  Job::ptr_t pJob (jobManager()->findJob(jobId));
  if(pJob)
  {
      sdpa::events::JobStatusReplyEvent::Ptr const pStatReply
        (new sdpa::events::JobStatusReplyEvent ( pEvt->to()
                                               , pEvt->from()
                                               , pJob->id()
                                               , pJob->getStatus()
                                               , pJob->error_code()
                                               , pJob->error_message()
                                               )
      );

      sendEventToMaster (pStatReply);
  }
  else
  {
      SDPA_LOG_ERROR("job " << pEvt->job_id() << " could not be found!");

      ErrorEvent::Ptr pErrorEvt(new ErrorEvent(name(), pEvt->from(), ErrorEvent::SDPA_EJOBNOTFOUND, "Inexistent job: "+pEvt->job_id().str()) );
      sendEventToMaster(pErrorEvt);
  }
}

void GenericDaemon::handleRetrieveJobResultsEvent(const RetrieveJobResultsEvent* pEvt )
{
  Job::ptr_t pJob = jobManager()->findJob(pEvt->job_id());
  if(pJob)
  {
    pJob->RetrieveJobResults(pEvt, this);
  }
  else
  {
    SDPA_LOG_ERROR("job " << pEvt->job_id() << " could not be found!");

    ErrorEvent::Ptr pErrorEvt(new ErrorEvent(name(), pEvt->from(), ErrorEvent::SDPA_EJOBNOTFOUND, "Inexistent job: "+pEvt->job_id().str()) );
    sendEventToMaster(pErrorEvt);
  }
}

void GenericDaemon::handleJobStalledEvent (const sdpa::events::JobStalledEvent *pEvt)
{
  pause(pEvt->job_id());
}

void GenericDaemon::handleJobRunningEvent (const sdpa::events::JobRunningEvent *pEvt)
{
  resume(pEvt->job_id());
}
