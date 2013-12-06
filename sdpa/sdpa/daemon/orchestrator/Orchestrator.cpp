/*
 * =====================================================================================
 *
 *       Filename:  Orchestrator.hpp
 *
 *    Description:  Contains the Orchestrator class
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

#include <sdpa/daemon/orchestrator/Orchestrator.hpp>

#include <fhg/assert.hpp>
#include <sdpa/daemon/Job.hpp>

#include <seda/StageRegistry.hpp>

using namespace std;
using namespace sdpa::daemon;
using namespace sdpa::events;


template <typename T>
void Orchestrator::notifySubscribers(const T& ptrEvt)
{
  sdpa::job_id_t jobId = ptrEvt->job_id();

  BOOST_FOREACH(const sdpa::subscriber_map_t::value_type& pair_subscr_joblist, m_listSubscribers )
  {
    sdpa::job_id_list_t listSubscrJobs = pair_subscr_joblist.second;

    for( sdpa::job_id_list_t::iterator it = listSubscrJobs.begin(); it != listSubscrJobs.end(); it++ )
    if( *it == jobId )
    {
      //! \todo eliminate, do not use non-const getter
      ptrEvt->to() = pair_subscr_joblist.first;
      sendEventToMaster (ptrEvt);

      SDPA_LOG_DEBUG ("Send an event of type "<<ptrEvt->str()<<" to the subscriber "<<pair_subscr_joblist.first<<" (related to the job "<<jobId<<")");
      break;
    }
  }
}

void Orchestrator::handleJobFinishedEvent(const JobFinishedEvent* pEvt )
{
  assert (pEvt);

  // check if the message comes from outside/slave or from WFE
  // if it comes from a slave, one should inform WFE -> subjob
  // if it comes from WFE -> concerns the master job

  MLOG (TRACE, "The job " << pEvt->job_id() << " has finished!");

  if (pEvt->from() != sdpa::daemon::WE)
  {
      // send a JobFinishedAckEvent back to the worker/slave
      JobFinishedAckEvent::Ptr ptrAckEvt(new JobFinishedAckEvent(name(), pEvt->from(), pEvt->job_id()));

      // send ack to the slave
      DMLOG (TRACE, "Send JobFinishedAckEvent for the job " << pEvt->job_id() << " to the slave  "<<pEvt->from() );
      sendEventToSlave(ptrAckEvt);
  }

  //put the job into the state Finished or Cancelled
  Job* pJob = jobManager().findJob(pEvt->job_id());
  if(pJob)
  {
      DMLOG (TRACE, "The current state of the job "<<pEvt->job_id()<<" is: "<<sdpa::status::show(pJob->getStatus())<<". Change its status to \"SDPA::Finished\"!");
      pJob->JobFinished(pEvt);
      DMLOG (TRACE, "The current state of the job "<<pEvt->job_id()<<" is: "<<sdpa::status::show(pJob->getStatus()));
  }
  else
  {
      DMLOG(TRACE,  "got finished message for old Job "<< pEvt->job_id());
      return;
  }

  // It's an worker who has sent the message
  if( (pEvt->from() != sdpa::daemon::WE) )
  {
    Worker::worker_id_t worker_id = pEvt->from();
    we::mgmt::layer::id_type act_id = pEvt->job_id();

    try {
      DMLOG (TRACE, "Notify the subscribers that the job "<<act_id<<" finished");
      JobFinishedEvent::Ptr ptrEvtJobFinished(new JobFinishedEvent(*pEvt));
      notifySubscribers(ptrEvtJobFinished);

      try {
          DMLOG (TRACE, "Remove job "<<act_id<<" from the worker "<<worker_id);
          scheduler()->deleteWorkerJob ( worker_id, act_id );
      }
      catch(WorkerNotFoundException const &)
      {
          DMLOG(TRACE, "Worker "<<worker_id<<" not found!");
      }
      catch(const JobNotDeletedException& ex)
      {
          DMLOG(TRACE,  "Could not delete the job " << act_id
                                                     << " from worker "
                                                     << worker_id
                                                     << "queues: "
                                                     << ex.what() );
      }

    }catch(...) {
        DMLOG(ERROR, "Unexpected exception occurred!");
    }
  }
  else
  {
    SDPA_LOG_INFO("Notify the subscribers that the job "<<pEvt->job_id()<<" has finished!");
    JobFinishedEvent::Ptr ptrEvtJobFinished(new JobFinishedEvent(*pEvt));
    notifySubscribers(ptrEvtJobFinished);
  }
}

void Orchestrator::handleJobFailedEvent(const JobFailedEvent* pEvt )
{
  assert (pEvt);

  // check if the message comes from outside/slave or from WFE
  // if it comes from a slave, one should inform WFE -> subjob
  // if it comes from WFE -> concerns the master job

  //SDPA_LOG_INFO( "handle JobFailed event (job " << pEvt->job_id() << ") received from "<<pEvt->from());

  if (pEvt->from() != sdpa::daemon::WE)
  {
      // send a JobFinishedAckEvent back to the worker/slave
      JobFailedAckEvent::Ptr evt
          (new JobFailedAckEvent ( name()
                                   , pEvt->from()
                                   , pEvt->job_id() ) );

      // send the event to the slave
      sendEventToSlave(evt);
  }

  //put the job into the state Failed or Cancelled
  Job* pJob = jobManager().findJob(pEvt->job_id());
  if(pJob)
  {
      pJob->JobFailed(pEvt);
      DMLOG(TRACE, "The job state is: "<<sdpa::status::show(pJob->getStatus()));
  }
  else
  {
      DMLOG(TRACE,  "got failed message for old Job "<< pEvt->job_id());
      return;
  }

  // It's an worker who has sent the message
  if( (pEvt->from() != sdpa::daemon::WE) )
  {
      Worker::worker_id_t worker_id = pEvt->from();
      we::mgmt::layer::id_type actId = pJob->id().str();

      try {
        JobFailedEvent::Ptr ptrEvtJobFailed(new JobFailedEvent(*pEvt));
        DMLOG(TRACE, "Notify the subscribers that the job "<<actId<<" has failed");
        notifySubscribers(ptrEvtJobFailed);

        try {
            DMLOG(TRACE, "Remove the job "<<actId<<" from the worker "<<worker_id<<"'s queues");
            scheduler()->deleteWorkerJob(worker_id, pJob->id());
        }
        catch(const WorkerNotFoundException&)
        {
            DMLOG(TRACE, "Worker "<<worker_id<<" not found!");
        }
        catch(const JobNotDeletedException&)
        {
            DMLOG(TRACE, "Could not delete the job "<<pJob->id()<<" from the "<<worker_id<<"'s queues ...");
        }
      }
      catch(...) {
          DMLOG(ERROR, "Unexpected exception occurred!");
      }
  }
  else
  {
      SDPA_LOG_INFO("Notify the subscribers that the job "<<pEvt->job_id()<<" has failed!");
      JobFailedEvent::Ptr ptrEvtJobFailed(new JobFailedEvent(*pEvt));
      notifySubscribers(ptrEvtJobFailed);
  }
}

void Orchestrator::handleCancelJobEvent(const CancelJobEvent* pEvt )
{
  Job* pJob;

  pJob = jobManager().findJob(pEvt->job_id());
  if(pJob)
  {
      if(pJob->getStatus() == sdpa::status::CANCELED)
      {
          sendEventToMaster( ErrorEvent::Ptr( new ErrorEvent( name()
                                                              , pEvt->from()
                                                              , ErrorEvent::SDPA_EJOBALREADYCANCELED
                                                              , "Job already canceled" )
                                                   ));
          return;
      }

      if(pJob->completed())
      {
         sendEventToMaster( ErrorEvent::Ptr( new ErrorEvent( name()
                                                             , pEvt->from()
                                                             , ErrorEvent::SDPA_EJOBTERMINATED
                                                             , "Cannot cancel an already terminated job, its current status is: "
                                                                + pJob->getStatus() )
                                                  ));
         return;
      }

      // send immediately an acknowledgment to the component that requested the cancellation
      CancelJobAckEvent::Ptr pCancelAckEvt(new CancelJobAckEvent(name(), pEvt->from(), pEvt->job_id()));

        if(!isSubscriber(pJob->owner()))
          sendEventToMaster(pCancelAckEvt);

        notifySubscribers(pCancelAckEvt);

      // if the job is in pending or stalled, put it already on canceled
      if(!pJob->is_running())
      {
          pJob->CancelJob(pEvt);
          DMLOG(TRACE, "The job status is: "<<pJob->getStatus());
          return;
      }

      pJob->CancelJob(pEvt);
      pJob->CancelJobAck(pCancelAckEvt.get());
  }
  else
  {
      DMLOG(TRACE, "Job "<<pEvt->job_id()<<" not found!");
      sendEventToMaster( ErrorEvent::Ptr( new ErrorEvent( name()
                                                         , pEvt->from()
                                                         , ErrorEvent::SDPA_EJOBNOTFOUND
                                                         , "No such job found" )
                                                        ));
      return;
  }

  try
  {
    sdpa::worker_id_t worker_id = scheduler()->findSubmOrAckWorker(pEvt->job_id());

    DMLOG(TRACE, "Tell the worker "<<worker_id<<" to cancel the job "<<pEvt->job_id());
    CancelJobEvent::Ptr pCancelEvt( new CancelJobEvent( name()
                                                      , worker_id
                                                      , pEvt->job_id()
                                                      , pEvt->reason() ) );
    sendEventToSlave(pCancelEvt);

    DMLOG(TRACE, "The status of the job "<<pEvt->job_id()<<" is: "<<pJob->getStatus());
  }
  catch(const NoWorkerFoundException&)
  {
      DMLOG (WARN, "No cancel message is to be forwarded as no worker was sent the job "<<pEvt->job_id());
  }
}

void Orchestrator::handleCancelJobAckEvent(const CancelJobAckEvent* pEvt)
{
  DLOG(TRACE, "handleCancelJobAck(" << pEvt->job_id() << ")");

  Job* pJob(jobManager().findJob(pEvt->job_id()));
  if(pJob)
  {
    // update the job status to "Canceled"
    pJob->CancelJobAck(pEvt);
    DMLOG(TRACE, "The job state is: "<<pJob->getStatus());
    // just send an acknowledgment to the master
    // send an acknowledgment to the component that requested the cancellation
    CancelJobAckEvent::Ptr pCancelAckEvt(new CancelJobAckEvent(name(), pEvt->from(), pEvt->job_id()));
    notifySubscribers(pCancelAckEvt);
    return;
  }

  DMLOG(WARN, "could not find job: " << pEvt->job_id());
}

void Orchestrator::pause(const job_id_t& jobId)
{
  Job* pJob(jobManager().findJob(jobId));
  if(pJob)
  {
    pJob->Pause(NULL);
    return;
  }

  DMLOG (WARN, "Couldn't mark the worker job "<<jobId<<" as STALLED. The job was not found!");
}

void Orchestrator::resume(const job_id_t& jobId)
{
  Job* pJob(jobManager().findJob(jobId));
  if(pJob)
  {
      pJob->Resume(NULL);
      return;
  }

  DMLOG (WARN, "Couldn't mark the worker job "<<jobId<<" as STALLED. The job was not found!");
}

Orchestrator::ptr_t Orchestrator::create
  (const std::string& name, const std::string& url)
{
  Orchestrator::ptr_t pOrch (new Orchestrator (name, url));

  seda::Stage::Ptr daemon_stage (new seda::Stage (name, pOrch, 1));
  pOrch->setStage (daemon_stage);
  seda::StageRegistry::instance().insert (daemon_stage);

  pOrch->start_agent();
  return pOrch;
}
