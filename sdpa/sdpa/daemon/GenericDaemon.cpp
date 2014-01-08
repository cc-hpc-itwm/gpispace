/*
 * =====================================================================================
 *
 *       Filename:  GenericDaemon.cpp
 *
 *    Description:  Generic daemon implementation file
 *
 *        Version:  1.0
 *        Created:  2009
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Dr. Tiberiu Rotaru, tiberiu.rotaru@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */

#include <sdpa/daemon/Job.hpp>
#include <sdpa/events/SubscribeAckEvent.hpp>

#include <sdpa/daemon/GenericDaemon.hpp>
#include <sdpa/events/CapabilitiesGainedEvent.hpp>
#include <sdpa/events/CapabilitiesLostEvent.hpp>

#include <sdpa/id_generator.hpp>
#include <sdpa/daemon/exceptions.hpp>

#include <boost/tokenizer.hpp>
#include <boost/foreach.hpp>
#include <sstream>
#include <algorithm>

namespace
{
  struct agent_id_tag
  {
    static const char *name ()
    {
      return "agent";
    }
  };
}

namespace sdpa {
  namespace daemon {
namespace
{
  std::vector<std::string> require_proper_url (std::string url)
  {
    const boost::tokenizer<boost::char_separator<char> > tok
      (url, boost::char_separator<char> (":"));

    const std::vector<std::string> vec (tok.begin(), tok.end());

    if (vec.empty() || vec.size() > 2)
    {
      throw std::runtime_error ("configuration of network failed: invalid url: has to be of format 'host[:port]'");
    }

    return vec;
  }

  fhg::com::host_t host_from_url (std::string url)
  {
    return fhg::com::host_t (require_proper_url (url)[0]);
  }
  fhg::com::port_t port_from_url (std::string url)
  {
    const std::vector<std::string> vec (require_proper_url (url));
    return fhg::com::port_t (vec.size() == 2 ? vec[1] : "0");
  }
}

GenericDaemon::GenericDaemon( const std::string name
                            , const std::string url
                            , const master_info_list_t arrMasterInfo
                            , unsigned int rank
                            , const boost::optional<std::string>& guiUrl
                            , bool create_wfe
                            )
  : SDPA_INIT_LOGGER(name)
  , _name (name)
  , m_arrMasterInfo(arrMasterInfo),
    _job_manager(),
    ptr_scheduler_()
  , _random_extraction_engine (boost::make_optional (create_wfe, boost::mt19937()))
  , ptr_workflow_engine_ ( create_wfe
                         ? new we::mgmt::layer
                           ( boost::bind (&GenericDaemon::submit, this, _1, _2, _3)
                           , boost::bind (&GenericDaemon::cancel, this, _1)
                           , boost::bind (&GenericDaemon::finished, this, _1, _2)
                           , boost::bind (&GenericDaemon::failed, this, _1, _2, _3)
                           , boost::bind (&GenericDaemon::canceled, this, _1)
                           , boost::bind (&GenericDaemon::gen_id, this)
                           , *_random_extraction_engine
                           )
                         : NULL
                         ),
    m_nRank(rank),
    m_strAgentUID(id_generator::instance<agent_id_tag>().next()),
    m_guiService ( guiUrl && !guiUrl->empty()
                 ? boost::optional<NotificationService>
                   (NotificationService (*guiUrl))
                 : boost::none
                 )
  , _max_consecutive_registration_attempts (360)
  , _max_consecutive_network_faults (360)
  , _registration_timeout (boost::posix_time::seconds (1))
  , _event_queue()
  , _event_handler_thread (&GenericDaemon::handle_events, this)
  , _network_strategy
    ( new sdpa::com::NetworkStrategy ( boost::bind (&GenericDaemon::sendEventToSelf, this, _1)
                                     , name /*name for peer*/
                                     , host_from_url (url)
                                     , port_from_url (url)
                                     )
    )
{
  // ask kvs if there is already an entry for (name.id = m_strAgentUID)
  //     e.g. kvs::get ("sdpa.daemon.<name>")
  //          if exists: throw
  //          else:
  //             (fhg::com::)kvs::put ("sdpa.daemon.<name>.id", m_strAgentUID)
  //             kvs::put ("sdpa.daemon.<name>.pid", getpid())
  //                - remove them in destructor

  // application gui service
  if (guiUrl && !guiUrl->empty())
  {
    DMLOG (TRACE, "Application GUI service at " << *guiUrl << " attached...");
  }
}

GenericDaemon::~GenericDaemon()
{
  DMLOG (TRACE, "Shutting down the component "<<name()<<" ...");

  BOOST_FOREACH (sdpa::MasterInfo& masterInfo, m_arrMasterInfo)
  {
    if (!masterInfo.name().empty() && masterInfo.is_registered())
    {
      sendEventToOther
        ( events::ErrorEvent::Ptr ( new events::ErrorEvent ( name()
                                           , masterInfo.name()
                                           , events::ErrorEvent::SDPA_ENODE_SHUTDOWN
                                           , "node shutdown"
                                           )
                          )
        );
    }
  }

  _registration_threads.stop_all();

  _event_handler_thread.interrupt();
  if (_event_handler_thread.joinable())
  {
    _event_handler_thread.join();
  }

  ptr_scheduler_.reset();

  ptr_workflow_engine_.reset();

  _network_strategy.reset();

	DMLOG (TRACE, "Succesfully shut down  "<<name()<<" ...");
}

const std::string& GenericDaemon::name() const
{
  return _name;
}


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


void GenericDaemon::serveJob(const sdpa::worker_id_list_t& worker_list, const job_id_t& jobId)
{
  //take a job from the workers' queue and serve it
  Job* ptrJob = jobManager().findJob(jobId);
  if(ptrJob)
  {
      // create a SubmitJobEvent for the job job_id serialize and attach description
      LOG(TRACE, "The job "<<ptrJob->id()<<" was assigned the following workers:"<<worker_list);

      BOOST_FOREACH(const worker_id_t& worker_id, worker_list)
      {
        events::SubmitJobEvent::Ptr pSubmitEvt(new events::SubmitJobEvent(name(),
                                                          worker_id,
                                                          ptrJob->id(),
                                                          ptrJob->description(),
                                                                         //! \todo There is a parent known for this job: ptrJob->parent()
                                                                         boost::none,
                                                          worker_list));

        sendEventToOther(pSubmitEvt);
      }
  }
  else
  {
    DMLOG (WARN, "Couldn't find the job "<<jobId<<" when attempting to serve workers!");
  }
}

std::string GenericDaemon::gen_id()
{
  static id_generator generator ("job");
  return generator.next();
}

bool hasName(const sdpa::MasterInfo& masterInfo, const std::string& name)
{
  return masterInfo.name() == name;
}

void GenericDaemon::handleSubmitJobEvent (const events::SubmitJobEvent* evt)
{
  const events::SubmitJobEvent& e (*evt);

  DLOG(TRACE, "got job submission from " << e.from() << ": job-id := " << e.job_id());

  if(e.is_external())
  {
    lock_type lock(mtx_master_);
    // check if the incoming event was produced by a master to which the current agent has already registered
    master_info_list_t::iterator itMaster = find_if(m_arrMasterInfo.begin(), m_arrMasterInfo.end(), boost::bind(hasName, _1, e.from()));

    if( itMaster != m_arrMasterInfo.end() && !itMaster->is_registered() )
    {
      DMLOG (TRACE, "The agent "<<name()<<" is not yet registered with the master "<<itMaster->name()
            <<". No job from this master will be accepted as long as no registration confirmation has been received!");

      //send job rejected error event back to the master
      events::ErrorEvent::Ptr pErrorEvt(new events::ErrorEvent( name(),
                                                e.from(),
                                                events::ErrorEvent::SDPA_EPERM,
                                                "Waiting for registration confirmation. No job submission is allowed!",
                                                e.job_id()) );
      sendEventToOther(pErrorEvt);

      return;
    }
  }

  // First, check if the job 'job_id' wasn't already submitted!
  if(jobManager().findJob(e.job_id()))
  {
    // The job already exists -> generate an error message that the job already exists

    DMLOG (WARN, "The job with job-id: " << e.job_id()<<" does already exist! (possibly recovered)");
    if( e.is_external() )
    {
        events::ErrorEvent::Ptr pErrorEvt(new events::ErrorEvent(name(), e.from(), events::ErrorEvent::SDPA_EJOBEXISTS, "The job already exists!", e.job_id()) );
        sendEventToOther(pErrorEvt);
    }

    return;
  }

  DMLOG (TRACE, "Receive new job from "<<e.from() << " with job-id: " << e.job_id());

  const JobId job_id
    (JobId::is_invalid_job_id (e.job_id()) ? JobId (gen_id()) : e.job_id());

  try {
    // One should parse the workflow in order to be able to create a valid job
    bool b_master_job(e.is_external() && hasWorkflowEngine());
    jobManager().addJob(job_id, e.description(), e.parent_id(), b_master_job, e.from());
  }
  catch(std::bad_alloc const &ex)
  {
    if( e.is_external() )
    {
        DMLOG (WARN, "Couldn't allocate memory for a new job!");
        // couldn't allocate memory for a new job; the job is either too large or there are probably too many jobs submitted,
        // either by the user or by the wfe, one may try to submit later, after some of the exiting jobs have terminated and some space is freed
        events::ErrorEvent::Ptr pErrorEvt(new events::ErrorEvent(name(), e.from(), events::ErrorEvent::SDPA_EJOBNOTADDED, ex.what()) );
        sendEventToOther(pErrorEvt);
    }
    else
    {
        workflowEngine()->failed (job_id, fhg::error::UNEXPECTED_ERROR, ex.what());
    }
    return;
  }

  if( e.is_external())
  {
    events::SubmitJobAckEvent::Ptr pSubmitJobAckEvt(new events::SubmitJobAckEvent(name(), e.from(), job_id));
    sendEventToOther(pSubmitJobAckEvt);
  }

  // check if the message comes from outside or from WFE
  // if it comes from outside and the agent has an WFE, submit it to it
  if( e.is_external() && hasWorkflowEngine() )
  {
    DMLOG (TRACE, "got new job from " << e.from() << " = " << job_id);
    submitWorkflow(job_id);
  }
  else {
    scheduler()->enqueueJob(job_id);
  }
}

void GenericDaemon::handleWorkerRegistrationEvent (const events::WorkerRegistrationEvent* evt)
{
  const events::WorkerRegistrationEvent& evtRegWorker (*evt);

  worker_id_t worker_id (evtRegWorker.from());

  // check if the worker evtRegWorker.from() has already registered!
  try
  {
    registerWorker(evtRegWorker);
  }
  catch(WorkerAlreadyExistException& ex)
  {
    if( evtRegWorker.agent_uuid() != ex.agent_uuid() )
    {
      // TODO: maybe just disallow registration, it is an error, if we have two workers with the same name still active...

      DMLOG (TRACE, "The worker manager already contains an worker with the same id (="<<ex.worker_id()<<") but with a different agent_uuid!" );

      try {
    	  const Worker::ptr_t pWorker = findWorker(worker_id);

    	  // mark the worker as disconnected
    	  pWorker->set_disconnected();
    	  scheduler()->deleteWorker(worker_id);
      }
      catch (const WorkerNotFoundException& ex)
      {
        DMLOG (WARN, "New worker find the worker "<<worker_id);
      }

      DMLOG (TRACE, "Add worker"<<worker_id );
      registerWorker(evtRegWorker);
    }
    else
    {
      DMLOG (TRACE, "A worker with the same id (" << worker_id << ") and uuid ("<<evtRegWorker.agent_uuid()<<" is already registered!");

      // just answer back with an acknowledgment
      DMLOG (TRACE, "Send registration ack to the agent " << worker_id );
      events::WorkerRegistrationAckEvent::Ptr const pWorkerRegAckEvt
        (new events::WorkerRegistrationAckEvent ( name(), evtRegWorker.from()));

      sendEventToOther(pWorkerRegAckEvt);
    }
  }
}

void GenericDaemon::handleErrorEvent (const events::ErrorEvent* evt)
{
  const sdpa::events::ErrorEvent& error (*evt);

  DMLOG(TRACE, "got error event from " << error.from() << " code: " << error.error_code() << " reason: " << error.reason());

  // if it'a communication error, inspect all jobs and
  // send results if they are in a terminal state

  switch (error.error_code())
  {
    // this  should  better go  into  a  distinct  event, since  the  ErrorEvent
    // 'reason' should not be reused for important information
    case events::ErrorEvent::SDPA_EJOBREJECTED:
    {
      sdpa::job_id_t jobId(*error.job_id());
      sdpa::worker_id_t worker_id(error.from());
      DMLOG (WARN, "The worker "<<worker_id<<" rejected the job "<<error.job_id()->str()<<". Reschedule it now!");

      scheduler()->rescheduleWorkerJob(worker_id, jobId);
      break;
    }
    case events::ErrorEvent::SDPA_EWORKERNOTREG:
    {
      DMLOG (TRACE, "New instance of the master is up, sending new registration request!");
      // mark the agen as not-registered

      worker_id_list_t listDeadMasters;
      {
        lock_type lock(mtx_master_);
        BOOST_FOREACH(sdpa::MasterInfo & masterInfo, m_arrMasterInfo)
        {
          if( error.from() == masterInfo.name() )
          {
            // we should not put the event handler thread to sleep, but delegate the event sending to some timer thing
            masterInfo.set_registered(false);
            masterInfo.incConsecRegAttempts();

            if(masterInfo.getConsecRegAttempts()< _max_consecutive_registration_attempts)
            {
              request_registration_soon (masterInfo);
            }
            else
              listDeadMasters.push_back( masterInfo.name() );
          }
        }
      }

      removeMasters(listDeadMasters);
      break;
    }
    case events::ErrorEvent::SDPA_ENODE_SHUTDOWN:
    case events::ErrorEvent::SDPA_ENETWORKFAILURE:
    {
      if( isSubscriber(error.from()) )
        unsubscribe(error.from());

      worker_id_t worker_id(error.from());

      try
      {
        Worker::ptr_t ptrWorker = findWorker(worker_id);

        if(ptrWorker)
        {
          DMLOG (TRACE, "worker " << worker_id << " went down (clean).");

          // notify capability losses...
          lock_type lock(mtx_master_);
          BOOST_FOREACH(sdpa::MasterInfo& masterInfo, m_arrMasterInfo)
          {
            events::CapabilitiesLostEvent::Ptr shpCpbLostEvt(
                                  new events::CapabilitiesLostEvent( name(),
                                                                           masterInfo.name(),
                                                                           ptrWorker->capabilities()
                                                                           ));

            sendEventToOther(shpCpbLostEvt);
          }

          // if there still are registered workers, otherwise declare the remaining
          // jobs failed
          scheduler()->deleteWorker(worker_id); // do a re-scheduling here
        }
      }
      catch (WorkerNotFoundException const& /*ignored*/)
      {
        worker_id_list_t listDeadMasters;
        {
          lock_type lock(mtx_master_);
          // check if the message comes from a master
          BOOST_FOREACH(sdpa::MasterInfo & masterInfo, m_arrMasterInfo)
          {
            if( error.from() == masterInfo.name() )
            {
              DMLOG (WARN, "The connection to the master " << masterInfo.name() << " is broken!");
              masterInfo.set_registered(false);
              masterInfo.incConsecNetFailCnt();

              if( masterInfo.getConsecNetFailCnt() < _max_consecutive_network_faults)
              {
                request_registration_soon (masterInfo);
              }
              else
                listDeadMasters.push_back( masterInfo.name() );
            }
          }
        }

        removeMasters(listDeadMasters);
      }
      break;
    }
    case events::ErrorEvent::SDPA_EJOBEXISTS:
    {
      DMLOG (TRACE, "The worker managed to recover the job "<<*error.job_id()<<", it already has it!");
      // do the same as when receiving a SubmitJobAckEvent

      Worker::worker_id_t worker_id = error.from();

      // Only now should be the job state machine make a transition to RUNNING
      // this means that the job was not rejected, no error occurred etc ....
      // find the job ptrJob and call
      Job* ptrJob = jobManager().findJob(*error.job_id());
      if(ptrJob)
      {
        try {
            ptrJob->Dispatch();
            scheduler()->acknowledgeJob(worker_id, *error.job_id());
        }
        catch(WorkerNotFoundException const &)
        {
          DMLOG (WARN, "job re-submission could not be acknowledged: worker " << worker_id << " not found!");
        }
      }
      else
      {
          DMLOG (WARN, "The job " << *error.job_id() << " was not found on"<<name()<<"!");
      }

      break;
    }
    case events::ErrorEvent::SDPA_EPERM:
    {
    	sdpa::job_id_t jobId(*error.job_id());
    	sdpa::worker_id_t worker_id(error.from());
    	DMLOG (WARN, "The worker "<<worker_id<<" rejected the job "<<error.job_id()->str()<<". Reschedule it now!");

    	scheduler()->rescheduleWorkerJob(worker_id, jobId);
    	break;
    }
    default:
    {
      DMLOG(WARN, "got an ErrorEvent back (ignoring it): code=" << error.error_code() << " reason=" << error.reason());
    }
  }
}

/* Implements Gwes2Sdpa */
/**
 * Submit an atomic activity to the SDPA.
 * This method is to be called by the GS in order to delegate
 * the execution of activities.
 * The SDPA will use the callback handler SdpaGwes in order
 * to notify the GS about activity status transitions.
 */
void GenericDaemon::submit( const we::mgmt::layer::id_type& activityId
                          , const we::mgmt::type::activity_t& activity
                          , const we::mgmt::layer::id_type& parent_id
                          )
{
  const we::type::schedule_data schedule_data
    ( activity.transition().get_schedule_data<long> (activity.input(), "num_worker")
    , activity.transition().get_schedule_data<long> (activity.input(), "vmem")
    );
  job_requirements_t jobReqs(activity.transition().requirements(), schedule_data);

  DMLOG(TRACE, "workflow engine submitted "<<activityId);

  job_id_t job_id(activityId);

  if( schedule_data.num_worker() && schedule_data.num_worker().get()<=0)
  {
      workflowEngine()->failed (  activityId
                               , fhg::error::UNEXPECTED_ERROR
                               , "Invalid number of workers required: "
                               +boost::lexical_cast<std::string>( schedule_data.num_worker().get()));


        return;
    }

  on_scope_exit _ ( boost::bind ( &we::mgmt::layer::failed, workflowEngine()
                              , activityId
                              , fhg::error::UNEXPECTED_ERROR
                              , "Exception in GenericDaemon::submit()"
                              )
                );


  jobManager().addJobRequirements(job_id, jobReqs);

  // don't forget to set here the job's preferences
  events::SubmitJobEvent::Ptr pEvtSubmitJob( new events::SubmitJobEvent( sdpa::daemon::WE, name(), job_id, activity.to_string(), sdpa::job_id_t (parent_id)) );
  sendEventToSelf(pEvtSubmitJob);

  _.dont();
}

/**
 * Cancel an atomic activity that has previously been submitted to
 * the SDPA.
 */
void GenericDaemon::cancel(const we::mgmt::layer::id_type& activityId)
{
  DMLOG (TRACE, "The workflow engine requests the cancellation of the activity " << activityId);

  // cancel the job corresponding to that activity -> send downward a CancelJobEvent?
  // look for the job_id corresponding to the received workflowId into job_map_
  // in fact they should be the same!
  // generate const CancelJobEvent& event
  // Job& job = job_map_[job_id];
  // call job.CancelJob(event);

  job_id_t job_id(activityId);
  events::CancelJobEvent::Ptr pEvtCancelJob
          (new events::CancelJobEvent( sdpa::daemon::WE
                              , name()
                              , job_id )
          );
  sendEventToSelf(pEvtCancelJob);
}

/**
 * Notify the SDPA that a workflow finished (state transition
 * from running to finished).
 */
void GenericDaemon::finished(const we::mgmt::layer::id_type& workflowId, const we::mgmt::type::activity_t& result)
{
  DMLOG (TRACE, "activity finished: " << workflowId);
  // generate a JobFinishedEvent for self!
  // cancel the job corresponding to that activity -> send downward a CancelJobEvent?
  // look for the job_id corresponding to the received workflowId into job_map_
  // in fact they should be the same!
  // generate const JobFinishedEvent& event
  // Job& job = job_map_[job_id];
  // call job.JobFinished(event);

  job_id_t job_id(workflowId);
  events::JobFinishedEvent::Ptr pEvtJobFinished( new events::JobFinishedEvent( sdpa::daemon::WE, name(), job_id, result.to_string()));
  sendEventToSelf(pEvtJobFinished);

  // notify the GUI that the activity finished
}

/**
 * Notify the SDPA that a workflow failed (state transition
 * from running to failed).
 */
void GenericDaemon::failed( const we::mgmt::layer::id_type& workflowId
                          , int error_code
                          , std::string const & reason
                          )
{
  MLOG( WARN
      , "job failed: " << workflowId
      << " error := " << fhg::error::show(error_code)
      << " (" << error_code << ")"
      << " message := " << reason
      );

  job_id_t job_id(workflowId);

  events::JobFailedEvent::Ptr pEvtJobFailed
    (new events::JobFailedEvent ( sdpa::daemon::WE
                        , name()
                        , job_id
                        , error_code
                        , reason
                        )
    );

  sendEventToSelf(pEvtJobFailed);
}

/**
 * Notify the SDPA that a workflow has been canceled (state
 * transition from * to terminated.
 */
void GenericDaemon::canceled(const we::mgmt::layer::id_type& workflowId)
{
  DMLOG (TRACE, "activity canceled: " << workflowId);
  // generate a JobCanceledEvent for self!

  job_id_t job_id(workflowId);

  events::CancelJobAckEvent::Ptr pEvtCancelJobAck( new events::CancelJobAckEvent(sdpa::daemon::WE, name(), job_id ));
  sendEventToSelf(pEvtCancelJobAck);
}

/*
   Submit a workflow to the workflow engine
*/
void GenericDaemon::submitWorkflow(const sdpa::job_id_t &jobId)
{
  DMLOG (TRACE, "Submit the job "<<jobId<<" to the workflow engine");

  we::mgmt::layer::id_type wf_id = jobId.str();

  on_scope_exit _ ( boost::bind ( &GenericDaemon::sendEventToSelf
                                , this
                                , events::JobFailedEvent::Ptr ( new events::JobFailedEvent ( sdpa::daemon::WE
                                                                           , name()
                                                                           , jobId
                                                                           , fhg::error::UNEXPECTED_ERROR
                                                                           , "Exception in GenericDaemon::submitWorkflow()"
                                                                           )
                                                      )
                                )
                  );

  try {
    Job* pJob = findJob(jobId);

    // Should set the workflow_id here, or send it together with the workflow description
    pJob->Dispatch();
    DMLOG (TRACE, "The status of the job "<<jobId<<" is "<<sdpa::status::show(pJob->getStatus()));

    if(pJob->description().empty() )
    {
        SDPA_LOG_ERROR("Empty Workflow!");
        // declare job as failed
        events::JobFailedEvent::Ptr pEvtJobFailed
              (new events::JobFailedEvent( sdpa::daemon::WE
                                 , name()
                                 , jobId
                                 , fhg::error::UNEXPECTED_ERROR
                                 , "the job has an empty workflow attached!"
                                 )
              );

        sendEventToSelf(pEvtJobFailed);
    }
    else
    {
      const we::mgmt::type::activity_t act (pJob->description());
      if (m_guiService)
      {
       std::list<std::string> workers; workers.push_back (name());
       const sdpa::daemon::NotificationEvent evt
       ( workers
          , jobId.str()
          , NotificationEvent::STATE_STARTED
          , act
       );

       m_guiService->notify (evt);
      }

      workflowEngine()->submit (wf_id, act);
    }
  }
  catch(const NoWorkflowEngine& ex)
  {
    SDPA_LOG_ERROR("No workflow engine is available!");

    events::JobFailedEvent::Ptr pEvtJobFailed
      (new events::JobFailedEvent( sdpa::daemon::WE
                                 , name()
                                 , jobId
                                 , fhg::error::UNEXPECTED_ERROR
                                 , "no workflow engine attached!"
                                 )
      );
    sendEventToSelf(pEvtJobFailed);
  }
  catch(const JobNotFoundException& ex)
  {
    SDPA_LOG_ERROR("Couldn't find the job "<<ex.job_id());

    events::JobFailedEvent::Ptr pEvtJobFailed
      (new events::JobFailedEvent( sdpa::daemon::WE
                                 , name()
                                 , jobId
                                 , fhg::error::UNEXPECTED_ERROR
                                 , "job could not be found"
                                 )
      );

    sendEventToSelf(pEvtJobFailed);
  }
  catch(const std::exception& ex)
  {
     SDPA_LOG_ERROR("Exception occurred: " << ex.what() << ". Failed to submit the job "<<jobId<<" to the workflow engine!");

     events::JobFailedEvent::Ptr pEvtJobFailed
       (new events::JobFailedEvent( sdpa::daemon::WE
                                  , name()
                                  , jobId
                                  , fhg::error::UNEXPECTED_ERROR
                                  , ex.what()
                                  )
     );

     sendEventToSelf(pEvtJobFailed);
   }


  _.dont();
}


void GenericDaemon::handleWorkerRegistrationAckEvent(const sdpa::events::WorkerRegistrationAckEvent* pRegAckEvt)
{
  std::string masterName = pRegAckEvt->from();
  DMLOG (TRACE, "Received registration acknowledgment from "<<masterName);

  bool bFound = false;
  lock_type lock(mtx_master_);
  for( sdpa::master_info_list_t::iterator it = m_arrMasterInfo.begin(); it != m_arrMasterInfo.end() && !bFound; it++)
    if( it->name() == masterName )
    {
      DMLOG (TRACE, "Mark the agent "<<name()<<" as registered within the corresponding MasterInfo object ... ");
      it->set_registered(true);
      bFound=true;
    }

  if(!isTop())
    jobManager().resubmitResults(this);
}

void GenericDaemon::registerWorker(const events::WorkerRegistrationEvent& evtRegWorker)
{
  worker_id_t worker_id (evtRegWorker.from());

  DMLOG (TRACE, "****************Got new registration request from: " << worker_id << ", capacity = "<<evtRegWorker.capacity()<<", capabilities:" );

  // delete inherited capabilities that are owned by the current agent
  sdpa::capabilities_set_t workerCpbSet;

  // take the difference
  BOOST_FOREACH( const sdpa::capability_t& cpb, evtRegWorker.capabilities() )
  {
    // own capabilities have always the depth 0 and are not inherited by the descendants
    if( !isOwnCapability(cpb) )
    {
      sdpa::capability_t cpbMod(cpb);
      cpbMod.incDepth();
      workerCpbSet.insert(cpbMod);
    }
  }

  scheduler()->addWorker( worker_id, evtRegWorker.capacity(), workerCpbSet, evtRegWorker.rank(), evtRegWorker.agent_uuid() );

  // send back an acknowledgment
  DMLOG (TRACE, "Send back to the worker " << worker_id << " a registration acknowledgment!" );
  events::WorkerRegistrationAckEvent::Ptr pWorkerRegAckEvt(new events::WorkerRegistrationAckEvent(name(), worker_id));

  sendEventToOther(pWorkerRegAckEvt);

  if( !workerCpbSet.empty() && !isTop() )
  {
    lock_type lock(mtx_master_);
    // send to the masters my new set of capabilities
    for( sdpa::master_info_list_t::iterator it = m_arrMasterInfo.begin(); it != m_arrMasterInfo.end(); it++ )
      if (it->is_registered() && it->name() != worker_id  )
      {
        events::CapabilitiesGainedEvent::Ptr shpCpbGainEvt(new events::CapabilitiesGainedEvent(name(), it->name(), workerCpbSet));
        sendEventToOther(shpCpbGainEvt);
      }
  }
}

void GenericDaemon::handleCapabilitiesGainedEvent(const events::CapabilitiesGainedEvent* pCpbGainEvt)
{
	// tell the scheduler to add the capabilities of the worker pCpbGainEvt->from
  sdpa::worker_id_t worker_id = pCpbGainEvt->from();

  if (pCpbGainEvt->capabilities().empty())
  {
     return;
   }

  DMLOG (TRACE, "The worker \""<<worker_id<<"\" reported its capabilities: "<<pCpbGainEvt->capabilities());

  try
  {
    sdpa::capabilities_set_t workerCpbSet;

    BOOST_FOREACH(const sdpa::capability_t& cpb,  pCpbGainEvt->capabilities() )
    {
      // own capabilities have always the depth 0
      if( !isOwnCapability(cpb) )
      {
        sdpa::capability_t cpbMod(cpb);
        cpbMod.incDepth();
        workerCpbSet.insert(cpbMod);
      }
    }

    bool bModified = scheduler()->addCapabilities(worker_id, workerCpbSet);

    if(bModified)
    {
      if( !isTop() )
      {
        sdpa::capabilities_set_t newWorkerCpbSet;
        getWorkerCapabilities(worker_id, newWorkerCpbSet);

        if( !newWorkerCpbSet.empty() )
        {
          lock_type lock(mtx_master_);
          for( sdpa::master_info_list_t::iterator it = m_arrMasterInfo.begin(); it != m_arrMasterInfo.end(); it++ )
            if( it->is_registered() && it->name() != worker_id  )
            {
                events::CapabilitiesGainedEvent::Ptr shpCpbGainEvt(new events::CapabilitiesGainedEvent(name(), it->name(), newWorkerCpbSet));
              sendEventToOther(shpCpbGainEvt);
            }
        }
      }
    }
  }
  catch( const WorkerNotFoundException& ex )
  {
    DMLOG (WARN, "Could not add new capabilities. The worker "<<worker_id<<" was not found!");
  }
}

void GenericDaemon::handleCapabilitiesLostEvent(const events::CapabilitiesLostEvent* pCpbLostEvt)
{
  DMLOG(TRACE, "Received CapabilitiesLostEvent!");
  // tell the scheduler to remove the capabilities of the worker pCpbLostEvt->from

  sdpa::worker_id_t worker_id = pCpbLostEvt->from();
  try {
    scheduler()->removeCapabilities(worker_id, pCpbLostEvt->capabilities());

    DMLOG (TRACE, "lost capabilities from: " << worker_id << ": "<<pCpbLostEvt->capabilities());

    lock_type lock(mtx_master_);
    for( sdpa::master_info_list_t::iterator it = m_arrMasterInfo.begin(); it != m_arrMasterInfo.end(); it++)
      if (it->is_registered() && it->name() != worker_id )
      {
        events::CapabilitiesLostEvent::Ptr shpCpbLostEvt(new events::CapabilitiesLostEvent(name(), it->name(), pCpbLostEvt->capabilities()));
        sendEventToOther(shpCpbLostEvt);
      }
  }
  catch( const WorkerNotFoundException& ex)
  {
    DMLOG (WARN, "Could not remove the specified capabilities. The worker "<<worker_id<<" was not found!");
  }
}

void GenericDaemon::handleSubscribeEvent( const events::SubscribeEvent* pEvt )
{
  DMLOG(TRACE, "Received subscribe event!");
  subscribe(pEvt->subscriber(), pEvt->listJobIds());
}

void GenericDaemon::sendEventToSelf(const events::SDPAEvent::Ptr& pEvt)
{
  _event_queue.put (pEvt);
  DLOG(TRACE, "Sent " <<pEvt->str()<<" to "<<pEvt->to());
}
void GenericDaemon::handle_events()
{
  while (true)
  {
    _event_queue.get()->handleBy (this);
  }
}

void GenericDaemon::sendEventToOther(const events::SDPAEvent::Ptr& pEvt)
{
  _network_strategy->perform (pEvt);
  DLOG(TRACE, "Sent " <<pEvt->str()<<" to "<<pEvt->to());
}

void GenericDaemon::request_registration_soon (const MasterInfo& info)
{
  _registration_threads.start
    (boost::bind (&GenericDaemon::do_registration_after_sleep, this, info));
}

void GenericDaemon::do_registration_after_sleep (const MasterInfo info)
{
  DMLOG ( TRACE
        , "Wait " << boost::posix_time::to_simple_string (_registration_timeout)
        << " before trying to register with master " << info.name()
        );
  boost::this_thread::sleep (_registration_timeout);
  requestRegistration (info);
}

void GenericDaemon::requestRegistration(const MasterInfo& masterInfo)
{
  if( !masterInfo.is_registered() )
  {
    DMLOG (TRACE, "The agent \"" << name()
               << "\" is sending a registration event to master \"" << masterInfo.name()
          );

    capabilities_set_t cpbSet;
    getCapabilities(cpbSet);

    //std::cout<<cpbSet;

    events::WorkerRegistrationEvent::Ptr pEvtWorkerReg(new events::WorkerRegistrationEvent( name(), masterInfo.name(), boost::none, cpbSet,  rank(), agent_uuid()));
    sendEventToOther(pEvtWorkerReg);
  }
}

bool hasId(sdpa::MasterInfo& info, sdpa::agent_id_t& agId)
{
  return info.name() == agId;
}

void GenericDaemon::removeMasters(const agent_id_list_t& listMasters)
{
  lock_type lock(mtx_master_);
  BOOST_FOREACH(const sdpa::agent_id_t& id, listMasters)
  {
    master_info_list_t::iterator it = find_if( m_arrMasterInfo.begin(), m_arrMasterInfo.end(), boost::bind(hasId, _1, id) );
    if( it != m_arrMasterInfo.end() )
      m_arrMasterInfo.erase(it);
  }
}

void GenericDaemon::getCapabilities(sdpa::capabilities_set_t& cpbset)
{
  lock_type lock(mtx_cpb_);
  BOOST_FOREACH ( const sdpa::capabilities_set_t::value_type & capability
                , m_capabilities
                )
  {
    cpbset.insert (capability);
  }

   scheduler()->getAllWorkersCapabilities(cpbset);
}

void GenericDaemon::addCapability(const capability_t& cpb)
{
  lock_type lock(mtx_cpb_);
  m_capabilities.insert(cpb);
}

void GenericDaemon::unsubscribe(const sdpa::agent_id_t& id)
{
  lock_type lock(mtx_subscriber_);
  DLOG(TRACE, "Unsubscribe "<<id<<" ...");
  m_listSubscribers.erase(id);
}

bool GenericDaemon::subscribedFor(const sdpa::agent_id_t& agId, const sdpa::job_id_t& jobId)
{
  return std::find
    (m_listSubscribers[agId].begin(), m_listSubscribers[agId].end(), jobId)
    != m_listSubscribers[agId].end();
}

void GenericDaemon::subscribe(const sdpa::agent_id_t& subscriber, const sdpa::job_id_list_t& listJobIds)
{
  lock_type lock(mtx_subscriber_);

  // check if all the request jobs still exist
  BOOST_FOREACH(const sdpa::JobId& jobId, listJobIds)
  {
    // if the list contains at least one invalid job,
    // send back an error message
    if(!jobManager().findJob(jobId))
    {
        std::ostringstream oss("Could not subscribe for the job");
        oss<<jobId<<". The job does not exist!";
        events::ErrorEvent::Ptr pErrorEvt(new events::ErrorEvent( name(),
                                                                  subscriber,
                                                                  events::ErrorEvent::SDPA_EJOBNOTFOUND,
                                                                  oss.str()));
        sendEventToOther(pErrorEvt);
        return;
    }
  }

  // allow to subscribe multiple times with different lists of job ids
  if(isSubscriber(subscriber))
  {
    BOOST_FOREACH(const sdpa::JobId& jobId, listJobIds)
    {
      if( !subscribedFor(subscriber, jobId) )
        m_listSubscribers[subscriber].push_back(jobId);
    }
  }
  else
  {
    m_listSubscribers.insert(sdpa::subscriber_map_t::value_type(subscriber, listJobIds));
  }

  // TODO: we should only send an ack *if* the job actually exists....
  DMLOG (TRACE, "reply immediately with a SubscribeAckEvent");
  sdpa::events::SubscribeAckEvent::Ptr ptrSubscAckEvt(new sdpa::events::SubscribeAckEvent(name(), subscriber, listJobIds));
  sendEventToOther(ptrSubscAckEvt);

  // check if the subscribed jobs are already in a terminal state
  BOOST_FOREACH(const sdpa::JobId& jobId, listJobIds)
  {
    Job* pJob = findJob(jobId);
    if(pJob)
    {
      switch (pJob->getStatus())
      {
        case sdpa::status::FINISHED:
        {
          events::JobFinishedEvent::Ptr pEvtJobFinished
            (new events::JobFinishedEvent( name()
                                         , subscriber
                                         , pJob->id()
                                         , pJob->result()
                                         ));
          sendEventToOther(pEvtJobFinished);
        }
        break;

        case sdpa::status::FAILED:
        {
          events::JobFailedEvent::Ptr pEvtJobFailed
            (new events::JobFailedEvent( name()
                                       , subscriber
                                       , pJob->id()
                                       , fhg::error::UNASSIGNED_ERROR
                                       , "TODO: take the error message from the job pointer somehow"
                                       )
            );
          sendEventToOther(pEvtJobFailed);
        }
        break;

        case sdpa::status::CANCELED:
        {
          events::CancelJobAckEvent::Ptr pEvtCancelJobAck( new events::CancelJobAckEvent( name(), subscriber, pJob->id() ));
          sendEventToOther(pEvtCancelJobAck);
        }
        break;

        case sdpa::status::PENDING:
        case sdpa::status::RUNNING:
        case sdpa::status::STALLED:
        case sdpa::status::CANCELING:
          // send nothing to the master if the job is not completed
          break;
        default:
           throw std::runtime_error("The job "+jobId.str()+" has an invalid/unknown state");
      }
    }
    else
    {
      std::string strErr("The job ");
      strErr+=jobId.str();
      strErr+=" could not be found!";

      DMLOG (ERROR, strErr);
      sendEventToOther( events::ErrorEvent::Ptr( new events::ErrorEvent( name()
                                                          , subscriber
                                                          , events::ErrorEvent::SDPA_EJOBNOTFOUND
                                                          , strErr
                                                          )
      ));
    }
  }
}

bool GenericDaemon::isSubscriber(const sdpa::agent_id_t& agentId)
{
  lock_type lock(mtx_subscriber_);
  return m_listSubscribers.find (agentId) != m_listSubscribers.end();
}

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
void GenericDaemon::handleSubmitJobAckEvent(const events::SubmitJobAckEvent* pEvent)
{
  assert (pEvent);

  DLOG(TRACE, "handleSubmitJobAckEvent: " << pEvent->job_id() << " from " << pEvent->from());

  Worker::worker_id_t worker_id = pEvent->from();
  // Only, now should be state of the job updated to RUNNING
  // since it was not rejected, no error occurred etc ....
  //find the job ptrJob and call
  Job* ptrJob = jobManager().findJob(pEvent->job_id());
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
        events::ErrorEvent::Ptr pErrorEvt(new events::ErrorEvent(name(), worker_id, events::ErrorEvent::SDPA_EWORKERNOTREG, "not registered") );
        sendEventToOther(pErrorEvt);
      }
      catch(std::exception const &ex2)
      {
        SDPA_LOG_ERROR( "Unexpected exception during "
                        << " handleSubmitJobAckEvent("<< pEvent->job_id() << ")"
                        << ": "
                        << ex2.what()
                        );

        events::ErrorEvent::Ptr pErrorEvt(new events::ErrorEvent(name(), worker_id, events::ErrorEvent::SDPA_EUNKNOWN, ex2.what()) );
        sendEventToOther(pErrorEvt);
      }
  }
  else
  {
    SDPA_LOG_ERROR( "job " << pEvent->job_id()
                        << " could not be acknowledged:"
                        << " the job " <<  pEvent->job_id()
                        << " not found!"
                        );

    events::ErrorEvent::Ptr pErrorEvt(new events::ErrorEvent(name(), worker_id, events::ErrorEvent::SDPA_EJOBNOTFOUND, "Could not acknowledge job") );
    sendEventToOther(pErrorEvt);
  }
}

// respond to a worker that the JobFinishedEvent was received
void GenericDaemon::handleJobFinishedAckEvent(const events::JobFinishedAckEvent* pEvt)
{
  // The result was successfully delivered by the worker and the WE was notified
  // therefore, I can delete the job from the job map
  std::ostringstream os;
  Worker::worker_id_t worker_id = pEvt->from();
  DMLOG (TRACE, "Got acknowledgment for the finished job " << pEvt->job_id() << "!");

  if(jobManager().findJob(pEvt->job_id()))
  {
    try {
      DMLOG (TRACE, "Delete the job " << pEvt->job_id() << " from the JobManager!");
      // delete it from the map when you receive a JobFinishedAckEvent!
      jobManager().deleteJob(pEvt->job_id());
    }
    catch(JobNotDeletedException const & ex1)
    {
      SDPA_LOG_ERROR("job " << pEvt->job_id() << " could not be deleted: " << ex1.what());

      events::ErrorEvent::Ptr pErrorEvt(new events::ErrorEvent(name(), worker_id, events::ErrorEvent::SDPA_EJOBNOTDELETED, ex1.what()) );
      sendEventToOther(pErrorEvt);
    }
    catch(std::exception const &ex2)
    {
      SDPA_LOG_ERROR( "Unexpected exception during "
                      << " handleJobFinishedAckEvent("<< pEvt->job_id() << ")"
                      << ": "
                      << ex2.what()
                     );

      events::ErrorEvent::Ptr pErrorEvt(new events::ErrorEvent(name(), worker_id, events::ErrorEvent::SDPA_EUNKNOWN, ex2.what()));
      sendEventToOther(pErrorEvt);
    }
  }
  else
  {
     SDPA_LOG_ERROR("job " << pEvt->job_id() << " could not be found!");

     events::ErrorEvent::Ptr pErrorEvt(new events::ErrorEvent(name(), worker_id, events::ErrorEvent::SDPA_EJOBNOTFOUND, "Couldn't find the job!") );
     sendEventToOther(pErrorEvt);
  }
}

// respond to a worker that the JobFailedEvent was received
void GenericDaemon::handleJobFailedAckEvent(const events::JobFailedAckEvent* pEvt )
{
  std::ostringstream os;
  Worker::worker_id_t worker_id = pEvt->from();

  if(jobManager().findJob(pEvt->job_id()))
  {
    try {
        // delete it from the map when you receive a JobFailedAckEvent!
        jobManager().deleteJob(pEvt->job_id());
    }
    catch(JobNotDeletedException const & ex1)
    {
      SDPA_LOG_ERROR("job " << pEvt->job_id() << " could not be deleted: " << ex1.what());

      events::ErrorEvent::Ptr pErrorEvt(new events::ErrorEvent(name(), worker_id, events::ErrorEvent::SDPA_EJOBNOTDELETED, ex1.what()) );
      sendEventToOther(pErrorEvt);
    }
    catch(std::exception const &ex2)
    {
      SDPA_LOG_ERROR( "Unexpected exception during "
                      << " handleJobFinishedAckEvent("<< pEvt->job_id() << ")"
                      << ": "
                      << ex2.what()
                     );

      events::ErrorEvent::Ptr pErrorEvt(new events::ErrorEvent(name(), worker_id, events::ErrorEvent::SDPA_EUNKNOWN, ex2.what()));
      sendEventToOther(pErrorEvt);
    }
  }
  else
  {
    SDPA_LOG_ERROR("job " << pEvt->job_id() << " could not be found!");

    events::ErrorEvent::Ptr pErrorEvt(new events::ErrorEvent(name(), worker_id, events::ErrorEvent::SDPA_EJOBNOTFOUND, "Couldn't find the job!") );
    sendEventToOther(pErrorEvt);
  }
}

void GenericDaemon::handleJobStalledEvent (const events::JobStalledEvent *pEvt)
{
  pause(pEvt->job_id());
}

void GenericDaemon::handleJobRunningEvent (const events::JobRunningEvent *pEvt)
{
  resume(pEvt->job_id());
}

bool GenericDaemon::noChildJobStalled(const sdpa::job_id_t& jobId) const
{
  return jobManager().noChildJobStalled(jobId);
}

bool GenericDaemon::noChildJobRunning(const sdpa::job_id_t& jobId) const
{
  return jobManager().noChildJobRunning(jobId);
}

}}
