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
#include <sdpa/job_states.hpp>
#include <sdpa/events/DiscoverJobStatesEvent.hpp>
#include <sdpa/events/DiscoverJobStatesReplyEvent.hpp>
#include <we/type/value.hpp>
#include <we/type/value/poke.hpp>


namespace sdpa {
  namespace daemon {

  namespace
  {
    struct on_scope_exit
    {
      on_scope_exit (boost::function<void()> what)
        : _what (what)
        , _dont (false)
      {}
      ~on_scope_exit()
      {
        if (!_dont)
        {
          _what();
        }
      }
      void dont()
      {
        _dont = true;
      }
      boost::function<void()> _what;
      bool _dont;
    };
  }

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

      LLOG (DEBUG, _logger, "Send an event of type "<<ptrEvt->str()<<" to the subscriber "<<pair_subscr_joblist.first<<" (related to the job "<<jobId<<")");
      break;
    }
  }
}

void Orchestrator::handleJobFinishedEvent(const events::JobFinishedEvent* pEvt )
{
  assert (pEvt);

  // check if the message comes from outside/slave or from WFE
  // if it comes from a slave, one should inform WFE -> subjob
  // if it comes from WFE -> concerns the master job

  LLOG (TRACE, _logger, "The job " << pEvt->job_id() << " has finished!");

  if (pEvt->is_external())
  {
      // send a JobFinishedAckEvent back to the worker/slave
      events::JobFinishedAckEvent::Ptr ptrAckEvt(new  events::JobFinishedAckEvent(name(), pEvt->from(), pEvt->job_id()));

      // send ack to the slave
      DLLOG (TRACE, _logger, "Send JobFinishedAckEvent for the job " << pEvt->job_id() << " to the slave  "<<pEvt->from() );
      sendEventToOther(ptrAckEvt);
  }

  //put the job into the state Finished or Cancelled
  Job* pJob = jobManager().findJob(pEvt->job_id());
  if(pJob)
  {
    DLLOG (TRACE, _logger, "The current state of the job "<<pEvt->job_id()<<" is: "<<sdpa::status::show(pJob->getStatus())<<". Change its status to \"SDPA::Finished\"!");
      pJob->JobFinished(pEvt);
      DLLOG (TRACE, _logger, "The current state of the job "<<pEvt->job_id()<<" is: "<<sdpa::status::show(pJob->getStatus()));
  }
  else
  {
    DLLOG(TRACE, _logger, "got finished message for old Job "<< pEvt->job_id());
      return;
  }

  // It's an worker who has sent the message
  if( (pEvt->is_external()) )
  {
    Worker::worker_id_t worker_id = pEvt->from();
    we::layer::id_type act_id = pEvt->job_id();

    try {
      DLLOG (TRACE, _logger, "Notify the subscribers that the job "<<act_id<<" finished");
      events::JobFinishedEvent::Ptr ptrEvtJobFinished(new  events::JobFinishedEvent(*pEvt));
      notifySubscribers(ptrEvtJobFinished);

      try {
        DLLOG (TRACE, _logger, "Remove job "<<act_id<<" from the worker "<<worker_id);
          scheduler()->deleteWorkerJob ( worker_id, act_id );
      }
      catch(WorkerNotFoundException const &)
      {
        DLLOG(TRACE, _logger, "Worker "<<worker_id<<" not found!");
      }
      catch(const JobNotDeletedException& ex)
      {
        DLLOG(TRACE, _logger, "Could not delete the job " << act_id
                                                     << " from worker "
                                                     << worker_id
                                                     << "queues: "
                                                     << ex.what() );
      }

    }catch(...) {
      DLLOG(ERROR, _logger, "Unexpected exception occurred!");
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
  assert (pEvt);

  // check if the message comes from outside/slave or from WFE
  // if it comes from a slave, one should inform WFE -> subjob
  // if it comes from WFE -> concerns the master job

  //LLOG (INFO, _logger,  "handle JobFailed event (job " << pEvt->job_id() << ") received from "<<pEvt->from());

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
  Job* pJob = jobManager().findJob(pEvt->job_id());
  if(pJob)
  {
      pJob->JobFailed(pEvt);
      DLLOG(TRACE, _logger,"The job state is: "<<sdpa::status::show(pJob->getStatus()));
  }
  else
  {
    DLLOG(TRACE, _logger, "got failed message for old Job "<< pEvt->job_id());
      return;
  }

  // It's an worker who has sent the message
  if( pEvt->is_external() )
  {
      Worker::worker_id_t worker_id = pEvt->from();
      we::layer::id_type actId = pJob->id();

      try {
        events::JobFailedEvent::Ptr ptrEvtJobFailed(new  events::JobFailedEvent(*pEvt));
        DLLOG(TRACE, _logger,"Notify the subscribers that the job "<<actId<<" has failed");
        notifySubscribers(ptrEvtJobFailed);

        try {
          DLLOG(TRACE, _logger,"Remove the job "<<actId<<" from the worker "<<worker_id<<"'s queues");
            scheduler()->deleteWorkerJob(worker_id, pJob->id());
        }
        catch(const WorkerNotFoundException&)
        {
          DLLOG(TRACE, _logger,"Worker "<<worker_id<<" not found!");
        }
        catch(const JobNotDeletedException&)
        {
          DLLOG(TRACE, _logger,"Could not delete the job "<<pJob->id()<<" from the "<<worker_id<<"'s queues ...");
        }
      }
      catch(...) {
        DLLOG(ERROR, _logger, "Unexpected exception occurred!");
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

  pJob = jobManager().findJob(pEvt->job_id());
  if(pJob)
  {
      if(pJob->getStatus() == sdpa::status::CANCELING)
      {
          sendEventToOther( events::ErrorEvent::Ptr( new  events::ErrorEvent( name()
                                                              , pEvt->from()
                                                              , events::ErrorEvent::SDPA_EJOBALREADYCANCELED
                                                              , "A cancelation request for this job was already posted!" )
                                                   ));
          return;
      }

      if(pJob->completed())
      {
         sendEventToOther(  events::ErrorEvent::Ptr( new  events::ErrorEvent( name()
                                                             , pEvt->from()
                                                             ,  events::ErrorEvent::SDPA_EJOBTERMINATED
                                                             , "Cannot cancel an already terminated job, its current status is: "
                                                           + sdpa::status::show (pJob->getStatus()) )
                                                  ));
         return;
      }

        // send immediately an acknowledgment to the component that requested the cancellation
      events::CancelJobAckEvent::Ptr pCancelAckEvt(new  events::CancelJobAckEvent(name(), pEvt->from (), pEvt->job_id()));

      if(!isSubscriber(pEvt->from ()))
        sendEventToOther(pCancelAckEvt);

      on_scope_exit _ ( boost::bind ( &Orchestrator::sendEventToOther
                                     , this
                                     , events::ErrorEvent::Ptr ( new events::ErrorEvent ( name()
                                                                        , pEvt->from()
                                                                        , events::ErrorEvent::SDPA_EUNKNOWN
                                                                        , "Exception in Agent::handleCancelJobEvent"
                                                                        )
                                                       )
                                     )
                       );

      pJob->CancelJob(pEvt);

      boost::optional<sdpa::worker_id_t> worker_id = scheduler()->findSubmOrAckWorker(pEvt->job_id());
      if(worker_id)
      {
        DLLOG (TRACE, _logger, "Tell the worker "<<*worker_id<<" to cancel the job "<<pEvt->job_id());
         events::CancelJobEvent::Ptr pCancelEvt( new  events::CancelJobEvent( name()
                                                           , *worker_id
                                                           , pEvt->job_id() ) );
         sendEventToOther(pCancelEvt);

         DLLOG (TRACE, _logger, "The status of the job "<<pEvt->job_id()<<" is: "<<pJob->getStatus());
      }
      else
      {
        DLLOG (WARN, _logger, "No cancel message is to be forwarded as no worker was sent the job "<<pEvt->job_id());
          // the job was not yet assigned to any worker

          pJob->CancelJobAck(pCancelAckEvt.get());
          ptr_scheduler_->delete_job (pEvt->job_id());
      }

      _.dont();
  }
  else
  {
    DLLOG (TRACE, _logger, "Job "<<pEvt->job_id()<<" not found!");
      sendEventToOther(  events::ErrorEvent::Ptr( new  events::ErrorEvent( name()
                                                         , pEvt->from()
                                                         ,  events::ErrorEvent::SDPA_EJOBNOTFOUND
                                                         , "No such job found" )
                                                        ));
  }
}

void Orchestrator::handleCancelJobAckEvent(const events::CancelJobAckEvent* pEvt)
{
  DLLOG (TRACE, _logger, "handleCancelJobAck(" << pEvt->job_id() << ")");

  Job* pJob(jobManager().findJob(pEvt->job_id()));
  if(pJob)
  {
    // update the job status to "Canceled"
    pJob->CancelJobAck(pEvt);
    DLLOG (TRACE, _logger, "The job state is: "<<sdpa::status::show(pJob->getStatus()));

    events::CancelJobAckEvent::Ptr ptrCancelAckEvt(new events::CancelJobAckEvent(*pEvt));
    notifySubscribers(ptrCancelAckEvt);

    return;
  }

  DLLOG (WARN, _logger, "could not find job: " << pEvt->job_id());
}

void Orchestrator::handleDeleteJobEvent (const events::DeleteJobEvent* evt)
{
  const  events::DeleteJobEvent& e (*evt);

  DLLOG (TRACE, _logger, e.from() << " requesting to delete job " << e.job_id() );

  on_scope_exit _ ( boost::bind ( &GenericDaemon::sendEventToOther, this
                                ,  events::ErrorEvent::Ptr ( new  events::ErrorEvent ( name()
                                                                   , e.from()
                                                                   ,  events::ErrorEvent::SDPA_EUNKNOWN
                                                                   , "unknown"
                                                                   )
                                                  )
                               )
                  );


  Job* pJob = jobManager().findJob(e.job_id());
  if(pJob)
  {
      // the job must be in a non-terminal state
      if(!pJob->completed())
      {
        DLLOG (TRACE, _logger, "Cannot delete a job in a non-terminal state. Send back an error message to "<<evt->from()<<" from "<<name());
         events::ErrorEvent::Ptr pErrorEvt(
              new events::ErrorEvent( evt->to(),
                                            evt->from(),
                                            events::ErrorEvent::SDPA_EJOBNOTDELETED,
                                            "Cannot delete a job which is in a non-terminal state. Please, cancel it first!")
                                            );
          sendEventToOther(pErrorEvt);
          return;
      }

      try{
          jobManager().deleteJob(e.job_id());
          sendEventToOther( events::DeleteJobAckEvent::Ptr( new events::DeleteJobAckEvent(e.to(),
                                                                                  e.from(),
                                                                                  e.job_id())) );
      }
      catch(JobNotDeletedException const & ex)
      {
        DLLOG (WARN, _logger, "Job " << e.job_id() << " could not be deleted!");
          sendEventToOther(  events::ErrorEvent::Ptr( new  events::ErrorEvent( name()
                                                              , e.from()
                                                              , events::ErrorEvent::SDPA_EJOBNOTDELETED
                                                              , ex.what()
                                                            )
                                            ));
      }
  }
  else
  {
    DLLOG (WARN, _logger, "Job " << e.job_id() << " could not be found!");
      sendEventToOther(  events::ErrorEvent::Ptr( new  events::ErrorEvent( name()
                                                          , e.from()
                                                          ,  events::ErrorEvent::SDPA_EJOBNOTFOUND
                                                          , "no such job"
                                                         )
                                        ));
  }

  _.dont();
}

void Orchestrator::pause(const job_id_t& jobId)
{
  Job* pJob(jobManager().findJob(jobId));
  if(pJob)
  {
    pJob->Pause(NULL);
    return;
  }

  DLLOG (WARN, _logger, "Couldn't mark the worker job "<<jobId<<" as STALLED. The job was not found!");
}

void Orchestrator::resume(const job_id_t& jobId)
{
  Job* pJob(jobManager().findJob(jobId));
  if(pJob)
  {
      pJob->Resume(NULL);
      return;
  }

  DLLOG (WARN, _logger, "Couldn't mark the worker job "<<jobId<<" as STALLED. The job was not found!");
}

void Orchestrator::handleRetrieveJobResultsEvent(const events::RetrieveJobResultsEvent* pEvt )
{
  Job* pJob = jobManager().findJob(pEvt->job_id());
  if(pJob)
  {
      if(pJob->completed())
      {
          pJob->RetrieveJobResults(pEvt, this);
      }
      else
      {
          events::ErrorEvent::Ptr pErrorEvt(new events::ErrorEvent( name()
                                                                    , pEvt->from()
                                                                    , events::ErrorEvent::SDPA_EJOBTERMINATED
                                                                    , "Not allowed to request results for a non-terminated job, its current status is : "
                                                                    +  sdpa::status::show(pJob->getStatus()) )
                                            );
          sendEventToOther(pErrorEvt);
      }
  }
  else
  {
    LLOG (ERROR, _logger, "job " << pEvt->job_id() << " could not be found!");

    events::ErrorEvent::Ptr pErrorEvt(new events::ErrorEvent(name(), pEvt->from(), events::ErrorEvent::SDPA_EJOBNOTFOUND, "Inexistent job: "+pEvt->job_id()) );
    sendEventToOther(pErrorEvt);
  }
}

void Orchestrator::handleQueryJobStatusEvent(const events::QueryJobStatusEvent* pEvt )
{
  sdpa::job_id_t jobId = pEvt->job_id();

  Job* pJob (jobManager().findJob(jobId));
  if(pJob)
  {
      events::JobStatusReplyEvent::Ptr const pStatReply
        (new events::JobStatusReplyEvent ( pEvt->to()
                                         , pEvt->from()
                                         , pJob->id()
                                         , pJob->getStatus()
                                         , pJob->error_code()
                                         , pJob->error_message()
                                         )
      );

      sendEventToOther (pStatReply);
  }
  else
  {
    LLOG (ERROR, _logger, "job " << pEvt->job_id() << " could not be found!");

      events::ErrorEvent::Ptr pErrorEvt(new events::ErrorEvent(name(), pEvt->from(), events::ErrorEvent::SDPA_EJOBNOTFOUND, "Inexistent job: "+pEvt->job_id()) );
      sendEventToOther(pErrorEvt);
  }
}

void Orchestrator::handleDiscoverJobStatesEvent (const sdpa::events::DiscoverJobStatesEvent *pEvt)
{
  Job* pJob;

  pJob = jobManager().findJob(pEvt->job_id());
  if(!pJob)
  {
    DMLOG(TRACE, "Job "<<pEvt->job_id()<<" not found!");
    sendEventToOther( events::ErrorEvent::Ptr( new events::ErrorEvent( name()
                                                     , pEvt->from()
                                                     , events::ErrorEvent::SDPA_EJOBNOTFOUND
                                                     , "No such job found" )
                                                    ));

    return;
  }

  boost::optional<sdpa::worker_id_t> worker_id = scheduler()->findSubmOrAckWorker(pEvt->job_id());

  if(worker_id)
  {
      _umap_disc_id_ag_id.insert(umap_disc_id_ag_id_t::value_type(pEvt->discover_id(), pEvt->from()));
      DMLOG(TRACE, "Tell the worker "<<*worker_id<<" to collect the states of all child job/activities related to the job "<<pEvt->job_id());
      events::DiscoverJobStatesEvent::Ptr pDiscEvt( new events::DiscoverJobStatesEvent( name()
                                                                                       , *worker_id
                                                                                       , pEvt->job_id()
                                                                                       , pEvt->discover_id()) );
      sendEventToOther(pDiscEvt);
  }
  else // the job is in pending, it hsn't been submitted to any worker, yet
  {
      pnet::type::value::value_type discover_result;
      pnet::type::value::poke ("id", discover_result, pJob->id());
      pnet::type::value::poke ("state", discover_result, "PENDING");
      pnet::type::value::poke ("children", discover_result, std::set<pnet::type::value::value_type>());

      events::DiscoverJobStatesReplyEvent::Ptr pDiscReplyEvt(new events::DiscoverJobStatesReplyEvent( name()
                                                                                                   , pEvt->from()
                                                                                                   , pEvt->job_id()
                                                                                                   , pEvt->discover_id()
                                                                                                   , discover_result ));

      sendEventToOther(pDiscReplyEvt);
  }
}

void Orchestrator::handleDiscoverJobStatestReplyEvent (const sdpa::events::DiscoverJobStatesReplyEvent *pEvt)
{
   DMLOG(TRACE, "handleDiscoverJobStatestReplyEvent( discovery_id: " << pEvt->discover_id() << ")");

   sdpa::agent_id_t issuer(_umap_disc_id_ag_id.at( pEvt->discover_id()));
   events::DiscoverJobStatesReplyEvent::Ptr pDiscReplyEvt(new events::DiscoverJobStatesReplyEvent( name()
                                                                                                 , issuer
                                                                                                 , pEvt->job_id()
                                                                                                 , pEvt->discover_id()
                                                                                                 , pEvt->discover_result() ));
   _umap_disc_id_ag_id.erase(issuer);
   sendEventToOther(pDiscReplyEvt);
}

}} // end namespaces
