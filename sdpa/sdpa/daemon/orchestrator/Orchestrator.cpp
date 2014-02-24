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
#include <sdpa/events/DiscoverJobStatesEvent.hpp>
#include <sdpa/events/DiscoverJobStatesReplyEvent.hpp>
#include <we/type/value/poke.hpp>


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

  if (pEvt->is_external())
  {
      // send a JobFinishedAckEvent back to the worker/slave
      events::JobFinishedAckEvent::Ptr ptrAckEvt(new  events::JobFinishedAckEvent(name(), pEvt->from(), pEvt->job_id()));

      // send ack to the slave
      sendEventToOther(ptrAckEvt);
  }

  //put the job into the state Finished or Cancelled
  Job* pJob = findJob(pEvt->job_id());
  if(pJob)
  {
      pJob->JobFinished(pEvt);
  }
  else
  {
      return;
  }

  // It's an worker who has sent the message
  if( (pEvt->is_external()) )
  {
    Worker::worker_id_t worker_id = pEvt->from();
    we::layer::id_type act_id = pEvt->job_id();

    try {
      events::JobFinishedEvent::Ptr ptrEvtJobFinished(new  events::JobFinishedEvent(*pEvt));
      notifySubscribers(ptrEvtJobFinished);

      try {
          scheduler()->deleteWorkerJob ( worker_id, act_id );
      }
      catch(WorkerNotFoundException const &)
      {
      }

    }catch(...) {
    }
  }
  else
  {
    LLOG (INFO, _logger, "Notify the subscribers that the job "<<pEvt->job_id()<<" has finished!");
    events::JobFinishedEvent::Ptr ptrEvtJobFinished(new  events::JobFinishedEvent(*pEvt));
    notifySubscribers(ptrEvtJobFinished);
  }
}

void Orchestrator::handleJobFailedEvent(const  events::JobFailedEvent* pEvt )
{
  // check if the message comes from outside/slave or from WFE
  // if it comes from a slave, one should inform WFE -> subjob
  // if it comes from WFE -> concerns the master job

  if (pEvt->is_external())
  {
      // send a JobFinishedAckEvent back to the worker/slave
      events::JobFailedAckEvent::Ptr evt
          (new  events::JobFailedAckEvent ( name()
                                   , pEvt->from()
                                   , pEvt->job_id() ) );

      // send the event to the slave
      sendEventToOther(evt);
  }

  //put the job into the state Failed or Cancelled
  Job* pJob = findJob(pEvt->job_id());
  if(pJob)
  {
      pJob->JobFailed(pEvt);
  }
  else
  {
      return;
  }

  // It's an worker who has sent the message
  if( pEvt->is_external() )
  {
      Worker::worker_id_t worker_id = pEvt->from();
      we::layer::id_type actId = pJob->id();

      try {
        events::JobFailedEvent::Ptr ptrEvtJobFailed(new  events::JobFailedEvent(*pEvt));
        notifySubscribers(ptrEvtJobFailed);

        try {
            scheduler()->deleteWorkerJob(worker_id, pJob->id());
        }
        catch(const WorkerNotFoundException&)
        {
        }
      }
      catch(...) {
      }
  }
  else
  {
    LLOG (INFO, _logger, "Notify the subscribers that the job "<<pEvt->job_id()<<" has failed!");
      events::JobFailedEvent::Ptr ptrEvtJobFailed(new  events::JobFailedEvent(*pEvt));
      notifySubscribers(ptrEvtJobFailed);
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
      events::CancelJobAckEvent::Ptr pCancelAckEvt(new  events::CancelJobAckEvent(name(), pEvt->from (), pEvt->job_id()));

      if(!isSubscriber(pEvt->from ()))
        sendEventToOther(pCancelAckEvt);

      pJob->CancelJob(pEvt);

      boost::optional<sdpa::worker_id_t> worker_id = scheduler()->findSubmOrAckWorker(pEvt->job_id());
      if(worker_id)
      {
         events::CancelJobEvent::Ptr pCancelEvt( new  events::CancelJobEvent( name()
                                                           , *worker_id
                                                           , pEvt->job_id() ) );
         sendEventToOther(pCancelEvt);

      }
      else
      {
          // the job was not yet assigned to any worker

          pJob->CancelJobAck(pCancelAckEvt.get());
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
    pJob->CancelJobAck(pEvt);

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
          sendEventToOther( events::DeleteJobAckEvent::Ptr( new events::DeleteJobAckEvent(e.to(),
                                                                                  e.from(),
                                                                                  e.job_id())) );
  }
  else
  {
    throw std::runtime_error ("no such job");
  }
}

void Orchestrator::handleRetrieveJobResultsEvent(const events::RetrieveJobResultsEvent* pEvt )
{
  Job* pJob = findJob(pEvt->job_id());
  if(pJob)
  {
      if(sdpa::status::is_terminal (pJob->getStatus()))
      {
          pJob->RetrieveJobResults(pEvt, this);
      }
      else
      {
        throw std::runtime_error
          ( "Not allowed to request results for a non-terminated job, its current status is : "
          +  sdpa::status::show(pJob->getStatus())
          );
      }
  }
  else
  {
    throw std::runtime_error ("Inexistent job: "+pEvt->job_id());
  }
}

void Orchestrator::handleQueryJobStatusEvent(const events::QueryJobStatusEvent* pEvt )
{
  sdpa::job_id_t jobId = pEvt->job_id();

  Job* pJob (findJob(jobId));
  if(pJob)
  {
      events::JobStatusReplyEvent::Ptr const pStatReply
        (new events::JobStatusReplyEvent ( pEvt->to()
                                         , pEvt->from()
                                         , pJob->id()
                                         , pJob->getStatus()
                                         , pJob->error_message()
                                         )
      );

      sendEventToOther (pStatReply);
  }
  else
  {
    throw std::runtime_error ("Inexistent job: "+pEvt->job_id());
  }
}

void Orchestrator::handleDiscoverJobStatesEvent (const sdpa::events::DiscoverJobStatesEvent *pEvt)
{
  Job* pJob = findJob(pEvt->job_id());

  if(!pJob)
  {
      sendEventToOther( events::DiscoverJobStatesReplyEvent::Ptr(new events::DiscoverJobStatesReplyEvent( name()
                                                                                                         , pEvt->from()
                                                                                                         , pEvt->discover_id()
                                                                                                         , sdpa::discovery_info_t (pEvt->job_id(), boost::none, sdpa::discovery_info_set_t()))));
      return;
  }

  boost::optional<sdpa::worker_id_t> worker_id = scheduler()->findSubmOrAckWorker(pEvt->job_id());

  if(worker_id)
  {
      m_map_discover_ids.insert( std::make_pair( pEvt->discover_id(), job_info_t(pEvt->from(), pEvt->job_id(), pJob->getStatus()) ));
      events::DiscoverJobStatesEvent::Ptr pDiscEvt( new events::DiscoverJobStatesEvent( name()
                                                                                       , *worker_id
                                                                                       , pEvt->job_id()
                                                                                       , pEvt->discover_id()) );
      sendEventToOther(pDiscEvt);
  }
  else
  {
      events::DiscoverJobStatesReplyEvent::Ptr pDiscReplyEvt(new events::DiscoverJobStatesReplyEvent( name()
                                                                                                   , pEvt->from()
                                                                                                   , pEvt->discover_id()
                                                                                                   , sdpa::discovery_info_t (pEvt->job_id(),pJob->getStatus(), sdpa::discovery_info_set_t()) ));

      sendEventToOther(pDiscReplyEvt);
  }
}

}} // end namespaces
