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
#include <sdpa/events/CancelJobEvent.hpp>
#include <sdpa/events/CapabilitiesGainedEvent.hpp>
#include <sdpa/events/CapabilitiesLostEvent.hpp>
#include <sdpa/events/DiscoverJobStatesEvent.hpp>
#include <sdpa/events/DiscoverJobStatesReplyEvent.hpp>
#include <sdpa/events/JobResultsReplyEvent.hpp>
#include <sdpa/events/JobStatusReplyEvent.hpp>
#include <sdpa/events/QueryJobStatusEvent.hpp>
#include <sdpa/events/RetrieveJobResultsEvent.hpp>
#include <sdpa/id_generator.hpp>
#include <sdpa/daemon/exceptions.hpp>

#include <boost/tokenizer.hpp>
#include <boost/foreach.hpp>
#include <boost/range/adaptor/filtered.hpp>

#include <sstream>
#include <algorithm>

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
                            , std::string kvs_host
                            , std::string kvs_port
                            , const master_info_list_t arrMasterInfo
                            , const boost::optional<std::string>& guiUrl
                            , bool create_wfe
                            )
  : _logger (fhg::log::Logger::get (name))
  , _name (name)
  , m_arrMasterInfo(arrMasterInfo),
    ptr_scheduler_ (new CoallocationScheduler (this))
  , _random_extraction_engine (boost::make_optional (create_wfe, boost::mt19937()))
  , ptr_workflow_engine_ ( create_wfe
                         ? new we::layer
                           ( boost::bind (&GenericDaemon::submit, this, _1, _2)
                           , boost::bind (&GenericDaemon::cancel, this, _1)
                           , boost::bind (&GenericDaemon::finished, this, _1, _2)
                           , boost::bind (&GenericDaemon::failed, this, _1, _2)
                           , boost::bind (&GenericDaemon::canceled, this, _1)
                           , boost::bind (&GenericDaemon::discover, this, _1, _2)
                           , boost::bind (&GenericDaemon::discovered, this, _1, _2)
                           , boost::bind (&GenericDaemon::gen_id, this)
                           , *_random_extraction_engine
                           )
                         : NULL
                         ),
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
  , _kvs_client
    ( new fhg::com::kvs::client::kvsc
      (kvs_host, kvs_port, true, boost::posix_time::seconds(120), 1)
    )
  , _network_strategy
    ( new sdpa::com::NetworkStrategy ( boost::bind (&GenericDaemon::sendEventToSelf, this, _1)
                                     , name /*name for peer*/
                                     , host_from_url (url)
                                     , port_from_url (url)
                                     , _kvs_client
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
}

GenericDaemon::~GenericDaemon()
{
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

  delete ptr_workflow_engine_;

  _network_strategy.reset();

  BOOST_FOREACH (const Job* const pJob, job_map_ | boost::adaptors::map_values )
  {
    delete pJob;
  }
  job_map_.clear();
}

const std::string& GenericDaemon::name() const
{
  return _name;
}

void GenericDaemon::serveJob(const sdpa::worker_id_list_t& worker_list, const job_id_t& jobId)
{
  //take a job from the workers' queue and serve it
  Job* ptrJob = findJob(jobId);
  if(ptrJob)
  {
      // create a SubmitJobEvent for the job job_id serialize and attach description
      LLOG(TRACE, _logger, "The job "<<ptrJob->id()<<" was assigned the following workers:"<<worker_list);

      BOOST_FOREACH(const worker_id_t& worker_id, worker_list)
      {
        events::SubmitJobEvent::Ptr pSubmitEvt(new events::SubmitJobEvent(name(),
                                                          worker_id,
                                                          ptrJob->id(),
                                                          ptrJob->description(),
                                                          worker_list));

        sendEventToOther(pSubmitEvt);
      }
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

  if(e.is_external())
  {
    lock_type lock(mtx_master_);
    // check if the incoming event was produced by a master to which the current agent has already registered
    master_info_list_t::iterator itMaster = find_if(m_arrMasterInfo.begin(), m_arrMasterInfo.end(), boost::bind(hasName, _1, e.from()));

    if( itMaster != m_arrMasterInfo.end() && !itMaster->is_registered() )
    {
      throw std::runtime_error
        ("job submission from master not yet registered or unknown");
    }
  }

  // First, check if the job 'job_id' wasn't already submitted!
  if(e.job_id() && findJob(*e.job_id()))
  {
    // The job already exists -> generate an error message that the job already exists

    if( e.is_external() )
    {
        events::ErrorEvent::Ptr pErrorEvt(new events::ErrorEvent(name(), e.from(), events::ErrorEvent::SDPA_EJOBEXISTS, "The job already exists!", e.job_id()) );
        sendEventToOther(pErrorEvt);
    }

    return;
  }

  const job_id_t job_id (e.job_id() ? *e.job_id() : job_id_t (gen_id()));

  try {
    // One should parse the workflow in order to be able to create a valid job
    bool b_master_job(e.is_external() && hasWorkflowEngine());
    addJob(job_id, e.description(), b_master_job, e.from(), job_requirements_t());
  }
  catch (std::runtime_error const &ex)
  {
    if( e.is_external() )
    {
      throw;
    }
    else
    {
        workflowEngine()->failed (job_id, ex.what());
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
    submitWorkflow(job_id);
  }
  else {
    scheduler()->enqueueJob(job_id);
  }
}

void GenericDaemon::handleWorkerRegistrationEvent
  (const events::WorkerRegistrationEvent* event)
{
  const worker_id_t worker_id (event->from());

  // check if the worker event->from() has already registered!
  // delete inherited capabilities that are owned by the current agent
  sdpa::capabilities_set_t workerCpbSet;

  // take the difference
  BOOST_FOREACH (const sdpa::capability_t& cpb, event->capabilities())
  {
    // own capabilities have always the depth 0 and are not inherited
    // by the descendants
    if (!isOwnCapability (cpb))
    {
      sdpa::capability_t cpbMod (cpb);
      cpbMod.incDepth();
      workerCpbSet.insert (cpbMod);
    }
  }

  const bool was_new_worker
    (scheduler()->addWorker (worker_id, event->capacity(), workerCpbSet));

  sendEventToOther
    ( events::WorkerRegistrationAckEvent::Ptr
      (new events::WorkerRegistrationAckEvent (name(), worker_id))
    );

  if (was_new_worker && !workerCpbSet.empty() && !isTop())
  {
    lock_type lock (mtx_master_);
    // send to the masters my new set of capabilities
    BOOST_FOREACH (MasterInfo const& master, m_arrMasterInfo)
    {
      if (master.is_registered() && master.name() != worker_id)
      {
        sendEventToOther
          ( events::CapabilitiesGainedEvent::Ptr
            ( new events::CapabilitiesGainedEvent ( name()
                                                  , master.name()
                                                  , workerCpbSet
                                                  )
            )
          );
      }
    }
  }
}

void GenericDaemon::handleErrorEvent (const events::ErrorEvent* evt)
{
  const sdpa::events::ErrorEvent& error (*evt);

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

      scheduler()->rescheduleWorkerJob(worker_id, jobId);
      break;
    }
    case events::ErrorEvent::SDPA_EWORKERNOTREG:
    {
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
      // do the same as when receiving a SubmitJobAckEvent

      Worker::worker_id_t worker_id = error.from();

      // Only now should be the job state machine make a transition to RUNNING
      // this means that the job was not rejected, no error occurred etc ....
      // find the job ptrJob and call
      Job* ptrJob = findJob(*error.job_id());
      if(ptrJob)
      {
        try {
            ptrJob->Dispatch();
            scheduler()->acknowledgeJob(worker_id, *error.job_id());
        }
        catch(WorkerNotFoundException const &)
        {
        }
      }

      break;
    }
    default:
    {
      LLOG ( ERROR, _logger
           , "Unhandled error (" << error.error_code() << ") :" << error.reason()
           );
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
void GenericDaemon::submit( const we::layer::id_type& job_id
                          , const we::type::activity_t& activity
                          )
try
{
  const we::type::schedule_data schedule_data
    (activity.transition().get_schedule_data<unsigned long> (activity.input(), "num_worker"));

  if (schedule_data.num_worker() && schedule_data.num_worker().get() == 0UL)
  {
    throw std::runtime_error ("invalid number of workers required: 0UL");
  }

  {
    boost::mutex::scoped_lock const _ (_job_map_and_requirements_mutex);

    job_requirements_.insert
      ( std::make_pair ( job_id
                       , job_requirements_t
                         (activity.transition().requirements(), schedule_data)
                       )
      );
  }

  sendEventToSelf ( events::SubmitJobEvent::Ptr
                    ( new events::SubmitJobEvent
                      (sdpa::daemon::WE, name(), job_id, activity.to_string())
                    )
                  );
}
catch (std::exception const& ex)
{
  workflowEngine()->failed (job_id, ex.what());
}

/**
 * Cancel an atomic activity that has previously been submitted to
 * the SDPA.
 */
void GenericDaemon::cancel(const we::layer::id_type& activityId)
{
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
void GenericDaemon::finished(const we::layer::id_type& workflowId, const we::type::activity_t& result)
{
  //put the job into the state Finished
  job_id_t id(workflowId);

  Job* pJob = findJob(id);
  if(!pJob)
  {
    throw std::runtime_error ("got finished message for old/unknown Job " + id);
  }

  // forward it up
  events::JobFinishedEvent::Ptr pEvtJobFinished
                (new events::JobFinishedEvent( name()
                                     , pJob->owner()
                                     , id
                                     , result.to_string()
                                     )
                );

  pJob->JobFinished (result.to_string());

  if(!isSubscriber(pJob->owner()))
  {
    sendEventToOther(pEvtJobFinished);
  }

  if (m_guiService)
  {
    std::list<std::string> workers; workers.push_back (name());
    const we::type::activity_t act (pJob->description());
    const sdpa::daemon::NotificationEvent evt
      ( workers
      , pJob->id()
      , NotificationEvent::STATE_FINISHED
      , act
      );

    m_guiService->notify (evt);
  }

  BOOST_FOREACH(const sdpa::subscriber_map_t::value_type& pair_subscr_joblist, m_listSubscribers )
  {
    if(subscribedFor(pair_subscr_joblist.first, id))
    {
      events::SDPAEvent::Ptr ptrEvt
        ( new events::JobFinishedEvent ( name()
                               , pair_subscr_joblist.first
                               , pEvtJobFinished->job_id()
                               , pEvtJobFinished->result()
                               )
        );

      sendEventToOther(ptrEvt);
    }
  }
}

/**
 * Notify the SDPA that a workflow failed (state transition
 * from running to failed).
 */
void GenericDaemon::failed( const we::layer::id_type& workflowId
                          , std::string const & reason
                          )
{
  job_id_t id(workflowId);
  //put the job into the state Failed

  Job* pJob = findJob(id);
  if(!pJob)
  {
    throw std::runtime_error ("got failed message for old/unknown Job " + id);
  }

  // forward it up
  events::JobFailedEvent::Ptr pEvtJobFailed
    (new events::JobFailedEvent ( name()
                        , pJob->owner()
                        , id
                        , reason
                        )
    );

  // send the event to the master
  pJob->JobFailed (reason);

  if(!isSubscriber(pJob->owner()))
    sendEventToOther(pEvtJobFailed);

  if (m_guiService)
  {
    std::list<std::string> workers; workers.push_back (name());
    const we::type::activity_t act (pJob->description());
    const sdpa::daemon::NotificationEvent evt
      ( workers
      , pJob->id()
      , NotificationEvent::STATE_FINISHED
      , act
      );

    m_guiService->notify (evt);
  }

  BOOST_FOREACH( const sdpa::subscriber_map_t::value_type& pair_subscr_joblist, m_listSubscribers )
  {
    if(subscribedFor(pair_subscr_joblist.first, id))
    {
        events::JobFailedEvent::Ptr ptrEvt
        ( new events::JobFailedEvent ( name()
                             , pair_subscr_joblist.first
                             , pEvtJobFailed->job_id()
                             , reason
                             )
        );
      sendEventToOther(ptrEvt);
    }
  }
}

/**
 * Notify the SDPA that a workflow has been canceled (state
 * transition from * to terminated.
 */
void GenericDaemon::canceled(const we::layer::id_type& workflowId)
{
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
  try {
    Job* pJob = findJob(jobId);

    // Should set the workflow_id here, or send it together with the workflow description
    pJob->Dispatch();

    const we::type::activity_t act (pJob->description());
    if (m_guiService)
    {
      std::list<std::string> workers; workers.push_back (name());
      const sdpa::daemon::NotificationEvent evt
        ( workers
        , jobId
        , NotificationEvent::STATE_STARTED
        , act
        );

      m_guiService->notify (evt);
    }

    workflowEngine()->submit (jobId, act);
  }
  catch(const JobNotFoundException& ex)
  {
    events::JobFailedEvent::Ptr pEvtJobFailed
      (new events::JobFailedEvent( sdpa::daemon::WE
                                 , name()
                                 , jobId
                                 , "job could not be found"
                                 )
      );

    sendEventToSelf(pEvtJobFailed);
  }
  catch(const std::exception& ex)
  {
    LLOG (ERROR, _logger, "Exception occurred: " << ex.what() << ". Failed to submit the job "<<jobId<<" to the workflow engine!");

     events::JobFailedEvent::Ptr pEvtJobFailed
       (new events::JobFailedEvent( sdpa::daemon::WE
                                  , name()
                                  , jobId
                                  , ex.what()
                                  )
     );

     sendEventToSelf(pEvtJobFailed);
   }
}


void GenericDaemon::handleWorkerRegistrationAckEvent(const sdpa::events::WorkerRegistrationAckEvent* pRegAckEvt)
{
  std::string masterName = pRegAckEvt->from();
  bool bFound = false;
  lock_type lock(mtx_master_);
  for( sdpa::master_info_list_t::iterator it = m_arrMasterInfo.begin(); it != m_arrMasterInfo.end() && !bFound; it++)
    if( it->name() == masterName )
    {
      it->set_registered(true);
      bFound=true;
    }

  if(!isTop())
  {
    boost::mutex::scoped_lock const _ (_job_map_and_requirements_mutex);

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
              (name(), job->owner(), job->id(), job->result())
            );
          sendEventToOther(pEvtJobFinished);
        }
        break;

      case sdpa::status::FAILED:
        {
          sdpa::events::JobFailedEvent::Ptr pEvtJobFailed
            ( new sdpa::events::JobFailedEvent
              (name(), job->owner(), job->id(), "unknown error: error event resent")
            );
          sendEventToOther(pEvtJobFailed);
        }
        break;

      case sdpa::status::CANCELED:
        {
          sdpa::events::CancelJobAckEvent::Ptr pEvtJobCanceled
            ( new sdpa::events::CancelJobAckEvent
              (name(), job->owner(), job->id())
            );
          sendEventToOther(pEvtJobCanceled);
        }
        break;

      case sdpa::status::PENDING:
        {
          sdpa::events::SubmitJobAckEvent::Ptr pSubmitJobAckEvt
            ( new sdpa::events::SubmitJobAckEvent
              (name(), job->owner(), job->id())
            );
          sendEventToOther(pSubmitJobAckEvt);
        }
        break;

      case sdpa::status::RUNNING:
      case sdpa::status::CANCELING:
        // don't send anything to the master if the job is not completed or in a pending state
        break;
      default:
        throw std::runtime_error("The job "+job->id()+" has an invalid/unknown state");
      }
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
        const sdpa::capabilities_set_t newWorkerCpbSet
          (scheduler()->getWorkerCapabilities(worker_id));

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
  }
}

void GenericDaemon::handleCapabilitiesLostEvent(const events::CapabilitiesLostEvent* pCpbLostEvt)
{
  // tell the scheduler to remove the capabilities of the worker pCpbLostEvt->from

  sdpa::worker_id_t worker_id = pCpbLostEvt->from();
  try {
    scheduler()->removeCapabilities(worker_id, pCpbLostEvt->capabilities());

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
  }
}

void GenericDaemon::handleSubscribeEvent( const events::SubscribeEvent* pEvt )
{
  subscribe(pEvt->subscriber(), pEvt->listJobIds());
}

void GenericDaemon::sendEventToSelf(const events::SDPAEvent::Ptr& pEvt)
{
  _event_queue.put (pEvt);
}
void GenericDaemon::handle_events()
{
  while (true)
  {
    const events::SDPAEvent::Ptr event (_event_queue.get());
    try
    {
      event->handleBy (this);
    }
    catch (std::exception const& ex)
    {
      sendEventToOther
        ( events::ErrorEvent::Ptr
          ( new events::ErrorEvent ( event->to() //! \todo expects name() == to. true?
                                   , event->from()
                                   , events::ErrorEvent::SDPA_EUNKNOWN
                                   , ex.what()
                                   )
          )
        );
    }
  }
}

void GenericDaemon::sendEventToOther(const events::SDPAEvent::Ptr& pEvt)
{
  _network_strategy->perform (pEvt);
}

void GenericDaemon::request_registration_soon (const MasterInfo& info)
{
  _registration_threads.start
    (boost::bind (&GenericDaemon::do_registration_after_sleep, this, info));
}

void GenericDaemon::do_registration_after_sleep (const MasterInfo info)
{
  boost::this_thread::sleep (_registration_timeout);
  requestRegistration (info);
}

void GenericDaemon::requestRegistration(const MasterInfo& masterInfo)
{
  if( !masterInfo.is_registered() )
  {
    capabilities_set_t cpbSet;
    getCapabilities(cpbSet);

    //std::cout<<cpbSet;

    events::WorkerRegistrationEvent::Ptr pEvtWorkerReg(new events::WorkerRegistrationEvent( name(), masterInfo.name(), boost::none, cpbSet));
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
  BOOST_FOREACH(const job_id_t& jobId, listJobIds)
  {
    // if the list contains at least one invalid job,
    // send back an error message
    if(!findJob(jobId))
    {
        std::ostringstream oss("Could not subscribe for the job");
        oss<<jobId<<". The job does not exist!";
        events::ErrorEvent::Ptr pErrorEvt(new events::ErrorEvent( name(),
                                                                  subscriber,
                                                                  events::ErrorEvent::SDPA_EUNKNOWN,
                                                                  oss.str()));
        sendEventToOther(pErrorEvt);
        return;
    }
  }

  // allow to subscribe multiple times with different lists of job ids
  if(isSubscriber(subscriber))
  {
    BOOST_FOREACH(const job_id_t& jobId, listJobIds)
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
  sdpa::events::SubscribeAckEvent::Ptr ptrSubscAckEvt(new sdpa::events::SubscribeAckEvent(name(), subscriber, listJobIds));
  sendEventToOther(ptrSubscAckEvt);

  // check if the subscribed jobs are already in a terminal state
  BOOST_FOREACH(const job_id_t& jobId, listJobIds)
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
        case sdpa::status::CANCELING:
          // send nothing to the master if the job is not completed
          break;
        default:
           throw std::runtime_error("The job "+jobId+" has an invalid/unknown state");
      }
    }
    else
    {
      std::string strErr("The job ");
      strErr+=jobId;
      strErr+=" could not be found!";

      sendEventToOther( events::ErrorEvent::Ptr( new events::ErrorEvent( name()
                                                          , subscriber
                                                          , events::ErrorEvent::SDPA_EUNKNOWN
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
 * Action: - if the job was found, put the job into the state Running
 *         - move the job from the submitted queue of the worker worker_id, into its
 *           acknowledged queue
 *         - in the case when the worker was not found, trigger an exception and send back
 *           an error message
 * Postcondition: is either into the Running state or inexistent
 */
void GenericDaemon::handleSubmitJobAckEvent(const events::SubmitJobAckEvent* pEvent)
{
  Worker::worker_id_t worker_id = pEvent->from();
  // Only, now should be state of the job updated to RUNNING
  // since it was not rejected, no error occurred etc ....
  //find the job ptrJob and call
  Job* ptrJob = findJob(pEvent->job_id());
  if(ptrJob)
  {
      try
      {
        ptrJob->Dispatch();
        scheduler()->acknowledgeJob(worker_id, pEvent->job_id());
      }
      catch(WorkerNotFoundException const &ex1)
      {
        // the worker should register first, before posting a job request
        events::ErrorEvent::Ptr pErrorEvt(new events::ErrorEvent(name(), worker_id, events::ErrorEvent::SDPA_EWORKERNOTREG, "not registered") );
        sendEventToOther(pErrorEvt);
      }
  }
  else
  {
    LLOG (ERROR, _logger,  "job " << pEvent->job_id()
                        << " could not be acknowledged:"
                        << " the job " <<  pEvent->job_id()
                        << " not found!"
                        );

    throw std::runtime_error ("Could not acknowledge job");
  }
}

// respond to a worker that the JobFinishedEvent was received
void GenericDaemon::handleJobFinishedAckEvent(const events::JobFinishedAckEvent* pEvt)
{
  // The result was successfully delivered by the worker and the WE was notified
  // therefore, I can delete the job from the job map
  std::ostringstream os;
  Worker::worker_id_t worker_id = pEvt->from();
  if(findJob(pEvt->job_id()))
  {
      // delete it from the map when you receive a JobFinishedAckEvent!
      deleteJob(pEvt->job_id());
  }
  else
  {
    throw std::runtime_error ("Couldn't find the job!");
  }
}

// respond to a worker that the JobFailedEvent was received
void GenericDaemon::handleJobFailedAckEvent(const events::JobFailedAckEvent* pEvt )
{
  std::ostringstream os;
  Worker::worker_id_t worker_id = pEvt->from();

  if(findJob(pEvt->job_id()))
  {
        // delete it from the map when you receive a JobFailedAckEvent!
        deleteJob(pEvt->job_id());
  }
  else
  {
    throw std::runtime_error ("Couldn't find the job!");
  }
}

void GenericDaemon::discover (we::layer::id_type discover_id, we::layer::id_type job_id)
{
  events::DiscoverJobStatesEvent::Ptr pDiscEvt( new events::DiscoverJobStatesEvent( sdpa::daemon::WE
                                                                                    , name()
                                                                                    , job_id
                                                                                    , discover_id ));

  sendEventToSelf(pDiscEvt);
}

void GenericDaemon::discovered (we::layer::id_type discover_id, sdpa::discovery_info_t discover_result)
{
  sdpa::agent_id_t master_name(m_map_discover_ids.at(discover_id).disc_issuer());
  sendEventToOther( events::DiscoverJobStatesReplyEvent::Ptr(new events::DiscoverJobStatesReplyEvent( name()
                                                                                                      , master_name
                                                                                                      , discover_id
                                                                                                      , discover_result)));
  m_map_discover_ids.erase(discover_id);
}

void GenericDaemon::handleDiscoverJobStatesReplyEvent
  (const sdpa::events::DiscoverJobStatesReplyEvent* e)
{
  const sdpa::job_info_t& job_info (m_map_discover_ids.at (e->discover_id()));
  const sdpa::discovery_info_t discovery_info
    (job_info.job_id(), job_info.job_status(), e->discover_result().children());

  sendEventToOther
    ( events::DiscoverJobStatesReplyEvent::Ptr
      ( new events::DiscoverJobStatesReplyEvent
        (name(), job_info.disc_issuer(), e->discover_id(), discovery_info)
      )
    );

  m_map_discover_ids.erase (e->discover_id());
}


void GenericDaemon::handleRetrieveJobResultsEvent(const events::RetrieveJobResultsEvent* pEvt )
{
  Job* pJob = findJob(pEvt->job_id());
  if(pJob)
  {
      if(sdpa::status::is_terminal (pJob->getStatus()))
      {
        sendEventToOther ( events::JobResultsReplyEvent::Ptr
                           ( new events::JobResultsReplyEvent ( pEvt->to()
                                                              , pEvt->from()
                                                              , pEvt->job_id()
                                                              , pJob->result()
                                                              )
                           )
                         );
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

void GenericDaemon::handleQueryJobStatusEvent(const events::QueryJobStatusEvent* pEvt )
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

}}
