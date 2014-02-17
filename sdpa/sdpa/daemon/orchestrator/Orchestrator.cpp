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
      sendEventToOther(ptrAckEvt);
  }

  //put the job into the state Finished or Cancelled
  Job* pJob = jobManager().findJob(pEvt->job_id());
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
      catch(const JobNotDeletedException& ex)
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
        catch(const JobNotDeletedException&)
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

  pJob = jobManager().findJob(pEvt->job_id());
  if(pJob)
  {
      if(pJob->getStatus() == sdpa::status::CANCELING)
      {
          sendEventToOther( events::ErrorEvent::Ptr( new  events::ErrorEvent( name()
                                                              , pEvt->from()
                                                              , events::ErrorEvent::SDPA_EUNKNOWN
                                                              , "A cancelation request for this job was already posted!" )
                                                   ));
          return;
      }

      if(pJob->completed())
      {
         sendEventToOther(  events::ErrorEvent::Ptr( new  events::ErrorEvent( name()
                                                             , pEvt->from()
                                                             ,  events::ErrorEvent::SDPA_EUNKNOWN
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

      _.dont();
  }
  else
  {
      sendEventToOther(  events::ErrorEvent::Ptr( new  events::ErrorEvent( name()
                                                         , pEvt->from()
                                                         ,  events::ErrorEvent::SDPA_EUNKNOWN
                                                         , "No such job found" )
                                                        ));
  }
}

void Orchestrator::handleCancelJobAckEvent(const events::CancelJobAckEvent* pEvt)
{

  Job* pJob(jobManager().findJob(pEvt->job_id()));
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
         events::ErrorEvent::Ptr pErrorEvt(
              new events::ErrorEvent( evt->to(),
                                            evt->from(),
                                            events::ErrorEvent::SDPA_EUNKNOWN,
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
          sendEventToOther(  events::ErrorEvent::Ptr( new  events::ErrorEvent( name()
                                                              , e.from()
                                                              , events::ErrorEvent::SDPA_EUNKNOWN
                                                              , ex.what()
                                                            )
                                            ));
      }
  }
  else
  {
      sendEventToOther(  events::ErrorEvent::Ptr( new  events::ErrorEvent( name()
                                                          , e.from()
                                                          ,  events::ErrorEvent::SDPA_EUNKNOWN
                                                          , "no such job"
                                                         )
                                        ));
  }

  _.dont();
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
                                                                    , events::ErrorEvent::SDPA_EUNKNOWN
                                                                    , "Not allowed to request results for a non-terminated job, its current status is : "
                                                                    +  sdpa::status::show(pJob->getStatus()) )
                                            );
          sendEventToOther(pErrorEvt);
      }
  }
  else
  {
    LLOG (ERROR, _logger, "job " << pEvt->job_id() << " could not be found!");

    events::ErrorEvent::Ptr pErrorEvt(new events::ErrorEvent(name(), pEvt->from(), events::ErrorEvent::SDPA_EUNKNOWN, "Inexistent job: "+pEvt->job_id()) );
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
                                         , pJob->error_message()
                                         )
      );

      sendEventToOther (pStatReply);
  }
  else
  {
    LLOG (ERROR, _logger, "job " << pEvt->job_id() << " could not be found!");

      events::ErrorEvent::Ptr pErrorEvt(new events::ErrorEvent(name(), pEvt->from(), events::ErrorEvent::SDPA_EUNKNOWN, "Inexistent job: "+pEvt->job_id()) );
      sendEventToOther(pErrorEvt);
  }
}

void Orchestrator::handleDiscoverJobStatesEvent (const sdpa::events::DiscoverJobStatesEvent *pEvt)
{
  Job* pJob;

  pJob = jobManager().findJob(pEvt->job_id());

  if(!pJob)
  {
      sdpa::discovery_info_t discover_result(pEvt->job_id(), boost::none, sdpa::discovery_info_set_t());

      sendEventToOther( events::DiscoverJobStatesReplyEvent::Ptr(new events::DiscoverJobStatesReplyEvent( name()
                                                                                                         , pEvt->from()
                                                                                                         , pEvt->discover_id()
                                                                                                         , discover_result)));

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
      sdpa::discovery_info_t discover_result(pEvt->job_id(),pJob->getStatus(), sdpa::discovery_info_set_t());
      events::DiscoverJobStatesReplyEvent::Ptr pDiscReplyEvt(new events::DiscoverJobStatesReplyEvent( name()
                                                                                                   , pEvt->from()
                                                                                                   , pEvt->discover_id()
                                                                                                   , discover_result ));

      sendEventToOther(pDiscReplyEvt);
  }
}

}} // end namespaces
