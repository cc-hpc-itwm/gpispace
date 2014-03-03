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
#include <sdpa/daemon/Job.hpp>
#include <sdpa/job_states.hpp>
#include <sdpa/events/CancelJobEvent.hpp>
#include <sdpa/events/DiscoverJobStatesEvent.hpp>
#include <sdpa/events/DiscoverJobStatesReplyEvent.hpp>
#include <we/type/value/poke.hpp>


namespace sdpa {
  namespace daemon {
    Orchestrator::Orchestrator ( const std::string& name
                               , const std::string& url
                               , std::string kvs_host, std::string kvs_port
                               )
      : GenericDaemon (name, url, kvs_host, kvs_port)
    {}


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
      sendEventToOther (ptrEvt);

      break;
    }
  }
}

void Orchestrator::handleJobFinishedEvent(const events::JobFinishedEvent* pEvt )
{
  // check if the message comes from outside/slave or from WFE
  // if it comes from a slave, one should inform WFE -> subjob
  // if it comes from WFE -> concerns the master job

  LLOG (TRACE, _logger, "The job " << pEvt->job_id() << " has finished!");

  child_proxy (this, pEvt->from()).job_finished_ack (pEvt->job_id());

  //put the job into the state Finished or Cancelled
  Job* pJob = findJob(pEvt->job_id());
  if(pJob)
  {
      pJob->JobFinished (pEvt->result());
  }
  else
  {
      return;
  }

    Worker::worker_id_t worker_id = pEvt->from();
    we::layer::id_type act_id = pEvt->job_id();

    try {
      events::JobFinishedEvent::Ptr ptrEvtJobFinished(new  events::JobFinishedEvent(*pEvt));
      notifySubscribers(ptrEvtJobFinished);

      try {
          scheduler()->deleteWorkerJob ( worker_id, act_id );
          request_scheduling();
      }
      catch(WorkerNotFoundException const &)
      {
      }

    }catch(...) {
    }
}

void Orchestrator::handleJobFailedEvent(const  events::JobFailedEvent* pEvt )
{
  // check if the message comes from outside/slave or from WFE
  // if it comes from a slave, one should inform WFE -> subjob
  // if it comes from WFE -> concerns the master job

  child_proxy (this, pEvt->from()).job_failed_ack (pEvt->job_id());

  //put the job into the state Failed or Cancelled
  Job* pJob = findJob(pEvt->job_id());
  if(pJob)
  {
      pJob->JobFailed (pEvt->error_message());
  }
  else
  {
      return;
  }

      Worker::worker_id_t worker_id = pEvt->from();
      we::layer::id_type actId = pJob->id();

      try {
        events::JobFailedEvent::Ptr ptrEvtJobFailed(new  events::JobFailedEvent(*pEvt));
        notifySubscribers(ptrEvtJobFailed);

        try {
            scheduler()->deleteWorkerJob(worker_id, pJob->id());
            request_scheduling();
        }
        catch(const WorkerNotFoundException&)
        {
        }
      }
      catch(...) {
      }
}

void Orchestrator::handleCancelJobEvent(const  events::CancelJobEvent* pEvt )
{
  Job* pJob;

  pJob = findJob(pEvt->job_id());
  if(pJob)
  {
      if(pJob->getStatus() == sdpa::status::CANCELING)
      {
        throw std::runtime_error
          ("A cancelation request for this job was already posted!");
      }

      if(sdpa::status::is_terminal (pJob->getStatus()))
      {
        throw std::runtime_error
          ( "Cannot cancel an already terminated job, its current status is: "
          + sdpa::status::show (pJob->getStatus())
          );
      }

        // send immediately an acknowledgment to the component that requested the cancellation
      if(!isSubscriber(pEvt->from ()))
      {
        parent_proxy (this, pEvt->from()).cancel_job_ack (pEvt->job_id());
      }

      pJob->CancelJob();

      boost::optional<sdpa::worker_id_t> worker_id = scheduler()->findSubmOrAckWorker(pEvt->job_id());
      if(worker_id)
      {
        child_proxy (this, *worker_id).cancel_job (pEvt->job_id());
      }
      else
      {
          // the job was not yet assigned to any worker

          pJob->CancelJobAck();
          ptr_scheduler_->delete_job (pEvt->job_id());
      }
  }
  else
  {
    throw std::runtime_error ("No such job found" );
  }
}

void Orchestrator::handleCancelJobAckEvent(const events::CancelJobAckEvent* pEvt)
{

  Job* pJob(findJob(pEvt->job_id()));
  if(pJob)
  {
    // update the job status to "Canceled"
    pJob->CancelJobAck();

    events::CancelJobAckEvent::Ptr ptrCancelAckEvt(new events::CancelJobAckEvent(*pEvt));
    notifySubscribers(ptrCancelAckEvt);

    return;
  }

}

void Orchestrator::handleDeleteJobEvent (const events::DeleteJobEvent* evt)
{
  const  events::DeleteJobEvent& e (*evt);


  Job* pJob = findJob(e.job_id());
  if(pJob)
  {
      if(!sdpa::status::is_terminal (pJob->getStatus()))
      {
        throw std::runtime_error
          ("Cannot delete a job which is in a non-terminal state. Please, cancel it first!");
      }

          deleteJob(e.job_id());
    parent_proxy (this, e.from()).delete_job_ack (e.job_id());
  }
  else
  {
    throw std::runtime_error ("no such job");
  }
}

}} // end namespaces
