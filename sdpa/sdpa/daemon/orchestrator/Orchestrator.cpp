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

namespace sdpa {
  namespace daemon {

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
    Job::ptr_t pJob;
    try {
    	pJob = jobManager()->findJob(pEvt->job_id());
    	DMLOG (TRACE, "The current state of the job "<<pEvt->job_id()<<" is: "<<pJob->getStatus()<<". Change its status to \"SDPA::Finished\"!");
    	pJob->JobFinished(pEvt);
    	DMLOG (TRACE, "The current state of the job "<<pEvt->job_id()<<" is: "<<pJob->getStatus());
    }
    catch(JobNotFoundException const &)
    {
    	SDPA_LOG_WARN( "got finished message for old/unknown Job "<< pEvt->job_id());
    	return;
    }

    // It's an worker who has sent the message
    if( (pEvt->from() != sdpa::daemon::WE) )
    {
        Worker::worker_id_t worker_id = pEvt->from();
        id_type act_id = pEvt->job_id();

        try {
            result_type output = pEvt->result();

            DMLOG (TRACE, "Notify the subscribers that the job "<<act_id<<" finished");
            JobFinishedEvent::Ptr ptrEvtJobFinished(new JobFinishedEvent(*pEvt));
            notifySubscribers(ptrEvtJobFinished);

            try {
                DMLOG (TRACE, "Remove job "<<act_id<<" from the worker "<<worker_id);
                scheduler()->deleteWorkerJob ( worker_id, act_id );
            }
            catch(WorkerNotFoundException const &)
            {
                SDPA_LOG_WARN("Worker "<<worker_id<<" not found!");
            }
            catch(const JobNotDeletedException& ex)
            {
                SDPA_LOG_WARN( "Could not delete the job " << act_id
                                                           << " from worker "
                                                           << worker_id
                                                           << "queues: "
                                                           << ex.what() );
            }
        }catch(...) {
            SDPA_LOG_ERROR("Unexpected exception occurred!");
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
    Job::ptr_t pJob;
    try {
        pJob = jobManager()->findJob(pEvt->job_id());
        pJob->JobFailed(pEvt);
        SDPA_LOG_DEBUG("The job state is: "<<pJob->getStatus());
    }
    catch(const JobNotFoundException &)
    {
        SDPA_LOG_WARN("Job "<<pEvt->job_id()<<" not found!");
        // TODO: shouldn't we reply with an error here???
        return;
    }

    // It's an worker who has sent the message
    if( (pEvt->from() != sdpa::daemon::WE) )
    {
        Worker::worker_id_t worker_id = pEvt->from();
        id_type actId = pJob->id().str();

        try {
            result_type output = pEvt->result();

            JobFailedEvent::Ptr ptrEvtJobFailed(new JobFailedEvent(*pEvt));
            SDPA_LOG_DEBUG("Notify the subscribers that the job "<<actId<<" has failed");
            notifySubscribers(ptrEvtJobFailed);

            try {
                SDPA_LOG_DEBUG("Remove the job "<<actId<<" from the worker "<<worker_id<<"'s queues");
                scheduler()->deleteWorkerJob(worker_id, pJob->id());
            }
            catch(const WorkerNotFoundException&)
            {
                SDPA_LOG_WARN("Worker "<<worker_id<<" not found!");
            }
            catch(const JobNotDeletedException&)
            {
                SDPA_LOG_WARN("Could not delete the job "<<pJob->id()<<" from the "<<worker_id<<"'s queues ...");
            }
        }
        catch(...) {
            SDPA_LOG_ERROR("Unexpected exception occurred!");
        }
    }
    else
    {
    	SDPA_LOG_INFO("Notify the subscribers that the job "<<pEvt->job_id()<<" has failed!");
        JobFailedEvent::Ptr ptrEvtJobFailed(new JobFailedEvent(*pEvt));
        notifySubscribers(ptrEvtJobFailed);
    }
}

void Orchestrator::cancelPendingJob (const sdpa::events::CancelJobEvent& evt)
{
  try
  {
    sdpa::job_id_t jobId = evt.job_id();
    Job::ptr_t pJob(ptr_job_man_->findJob(jobId));

    DMLOG (TRACE, "Cancelling the pending job "<<jobId<<" ... ");

    sdpa::events::CancelJobEvent cae;
    pJob->CancelJob(&cae);
    ptr_scheduler_->delete_job (jobId);
  }
  catch(const JobNotFoundException &ex1)
  {
    SDPA_LOG_WARN( "The job "<< evt.job_id() << "could not be cancelled! Exception occurred: "<<ex1.what());
  }
}

void Orchestrator::handleCancelJobEvent(const CancelJobEvent* pEvt )
{
  Job::ptr_t pJob;

  try
  {
    pJob = ptr_job_man_->findJob(pEvt->job_id());

    // send immediately an acknowledgment to the component that requested the cancellation
    CancelJobAckEvent::Ptr pCancelAckEvt(new CancelJobAckEvent(name(), pJob->owner(), pEvt->job_id()));

    if(!isSubscriber(pJob->owner()))
      sendEventToMaster(pCancelAckEvt);

    notifySubscribers(pCancelAckEvt);
  }
  catch(const JobNotFoundException &)
  {
    SDPA_LOG_WARN("Job "<<pEvt->job_id()<<" not found!");
    return;
  }

  try
  {
    sdpa::worker_id_t worker_id = scheduler()->findSubmOrAckWorker(pEvt->job_id());

    SDPA_LOG_DEBUG("Tell the worker "<<worker_id<<" to cancel the job "<<pEvt->job_id());
    CancelJobEvent::Ptr pCancelEvt( new CancelJobEvent( name()
                                                      , worker_id
                                                      , pEvt->job_id()
                                                      , pEvt->reason() ) );
    sendEventToSlave(pCancelEvt);

    // change the job status to "Cancelling"
    pJob->CancelJob(pEvt);
    SDPA_LOG_DEBUG("The status of the job "<<pEvt->job_id()<<" is: "<<pJob->getStatus());
  }
  catch(const NoWorkerFoundException&)
  {
    // possible situations:
    // 1) the job wasn't yet assigned to any worker
    // 1) is in the pending queue of a certain worker
    // 1) the job was submitted to an worker but was not yet acknowledged
    cancelPendingJob(*pEvt);
  }
  catch (std::exception const & ex)
  {
    SDPA_LOG_WARN( "the workflow engine could not cancel the job "<<pEvt->job_id()<<"! Reason: "<< ex.what());
  }
}

void Orchestrator::handleCancelJobAckEvent(const CancelJobAckEvent* pEvt)
{
    assert (pEvt);

    DLOG(TRACE, "handleCancelJobAck(" << pEvt->job_id() << ")");

    try
    {
        Job::ptr_t pJob(jobManager()->findJob(pEvt->job_id()));

        // update the job status to "Cancelled"
        pJob->CancelJobAck(pEvt);
        SDPA_LOG_DEBUG("The job state is: "<<pJob->getStatus());
    }
    catch (std::exception const & ex)
    {
        LOG(WARN, "could not find job: " << ex.what());
        return;
    }

    // just send an acknowledgment to the master
    // send an acknowledgment to the component that requested the cancellation
    CancelJobAckEvent::Ptr pCancelAckEvt(new CancelJobAckEvent(name(), pEvt->from(), pEvt->job_id()));

    notifySubscribers(pCancelAckEvt);
}

void Orchestrator::handleRetrieveJobResultsEvent(const RetrieveJobResultsEvent* pEvt )
{
    try {
        Job::ptr_t pJob = jobManager()->findJob(pEvt->job_id());
        pJob->RetrieveJobResults(pEvt, this);
    }
    catch(const JobNotFoundException&)
    {
      MLOG (WARN, "The job "<<pEvt->job_id()<<" was not found by the JobManager");
      ErrorEvent::Ptr pErrorEvt(new ErrorEvent(name(), pEvt->from(), ErrorEvent::SDPA_EJOBNOTFOUND, "no such job") );
      sendEventToMaster(pErrorEvt);
    }
}

  }
}

Orchestrator::ptr_t Orchestrator::create ( const std::string& name
                                         , const std::string& url
                                         , const unsigned int capacity
                                         )
{
  Orchestrator::ptr_t pOrch (new Orchestrator (name, url, capacity));

  seda::Stage::Ptr daemon_stage (new seda::Stage (name, pOrch, 1));
  pOrch->setStage (daemon_stage);
  seda::StageRegistry::instance().insert (daemon_stage);

  return pOrch;
}
