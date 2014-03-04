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
#include <sdpa/events/delayed_function_call.hpp>
#include <sdpa/events/DiscoverJobStatesEvent.hpp>
#include <sdpa/events/DiscoverJobStatesReplyEvent.hpp>
#include <sdpa/events/ErrorEvent.hpp>
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
  , m_arrMasterInfo(arrMasterInfo)
  , ptr_scheduler_ (new CoallocationScheduler (this))
  , _scheduling_thread (&GenericDaemon::scheduling_thread, this)
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
                         )
  , m_guiService ( guiUrl && !guiUrl->empty()
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

  if (!isTop())
  {
    lock_type lock (mtx_master_);
    BOOST_FOREACH (sdpa::MasterInfo& masterInfo, m_arrMasterInfo)
    {
      requestRegistration (masterInfo);
    }
  }
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

  delete ptr_workflow_engine_;

  _event_handler_thread.interrupt();
  if (_event_handler_thread.joinable())
  {
    _event_handler_thread.join();
  }

  _scheduling_thread.interrupt();
  if (_scheduling_thread.joinable())
  {
    _scheduling_thread.join();
  }
  ptr_scheduler_.reset();

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
        child_proxy (this, worker_id).submit_job
          (ptrJob->id(), ptrJob->description(), worker_list);
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

        events::ErrorEvent::Ptr pErrorEvt(new events::ErrorEvent(name(), e.from(), events::ErrorEvent::SDPA_EJOBEXISTS, "The job already exists!", e.job_id()) );
        sendEventToOther(pErrorEvt);

    return;
  }

  const job_id_t job_id (e.job_id() ? *e.job_id() : job_id_t (gen_id()));

  // One should parse the workflow in order to be able to create a valid job
  Job* pJob (addJob(job_id, e.description(), hasWorkflowEngine(), e.from(), job_requirements_t()));

  //! \todo Don't ack before we know that we can: may fail 20 lines
  //! below. add Nack event of some sorts to not need
  //! submitack+jobfailed to fail submission. this would also resolve
  //! the race in handleDiscoverJobStatesEvent.
  parent_proxy (this, e.from()).submit_job_ack (job_id);

  // check if the message comes from outside or from WFE
  // if it comes from outside and the agent has an WFE, submit it to it
  if(hasWorkflowEngine() )
  {
    try
    {
      const we::type::activity_t act (pJob->description());
      workflowEngine()->submit (job_id, act);

      // Should set the workflow_id here, or send it together with the workflow description
      pJob->Dispatch();

      if (m_guiService)
      {
        std::list<std::string> workers; workers.push_back (name());
        const sdpa::daemon::NotificationEvent evt
          ( workers
          , job_id
          , NotificationEvent::STATE_STARTED
          , act
          );

        m_guiService->notify (evt);
      }
    }
    catch(const std::exception& ex)
    {
      LLOG (ERROR, _logger, "Exception occurred: " << ex.what() << ". Failed to submit the job "<<job_id<<" to the workflow engine!");

      failed (job_id, ex.what());
    }
  }
  else {
    scheduler()->enqueueJob(job_id);
    request_scheduling();
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
    (scheduler()->worker_manager().addWorker (worker_id, event->capacity(), workerCpbSet));

  child_proxy (this, worker_id).worker_registration_ack();

  request_scheduling();

  if (was_new_worker && !workerCpbSet.empty() && !isTop())
  {
    lock_type lock (mtx_master_);
    // send to the masters my new set of capabilities
    BOOST_FOREACH (MasterInfo const& master, m_arrMasterInfo)
    {
      if (master.is_registered() && master.name() != worker_id)
      {
        parent_proxy (this, master.name()).capabilities_gained (workerCpbSet);
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

      //! \todo ignore if worker no longer exists?
      scheduler()->rescheduleWorkerJob(worker_id, jobId);
      request_scheduling();
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
        Worker::ptr_t ptrWorker = scheduler()->worker_manager().findWorker(worker_id);

          // notify capability losses...
          lock_type lock(mtx_master_);
          BOOST_FOREACH(sdpa::MasterInfo& masterInfo, m_arrMasterInfo)
          {
            parent_proxy (this, masterInfo.name()).capabilities_lost
              (ptrWorker->capabilities());
          }

          // if there still are registered workers, otherwise declare the remaining
          // jobs failed
          scheduler()->deleteWorker(worker_id); // do a re-scheduling here
          request_scheduling();
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
            scheduler()->worker_manager().findWorker (worker_id)->acknowledge (*error.job_id());
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

  addJob ( job_id
         , activity.to_string()
         , false
         , name()
         , job_requirements_t (activity.transition().requirements(), schedule_data)
         );

  scheduler()->enqueueJob (job_id);
  request_scheduling();
}
catch (std::exception const& ex)
{
  workflowEngine()->failed (job_id, ex.what());
}

void GenericDaemon::cancel (const we::layer::id_type& job_id)
{
  delay (boost::bind (&GenericDaemon::delayed_cancel, this, job_id));
}
void GenericDaemon::delayed_cancel(const we::layer::id_type& job_id)
{
  Job* pJob (findJob (job_id));
  if (!pJob)
  {
    //! \note Job may have been removed between wfe requesting cancel
    //! and event thread handling this, which is not an error: wfe
    //! correctly handles that situation and expects us to ignore it.
    return;
  }

  const boost::optional<sdpa::worker_id_t> worker_id
    (scheduler()->worker_manager().findSubmOrAckWorker (job_id));

  pJob->CancelJob();

  if (worker_id)
  {
    child_proxy (this, *worker_id).cancel_job (job_id);
  }
  else
  {
    workflowEngine()->canceled (job_id);

    pJob->CancelJobAck();
    ptr_scheduler_->delete_job (job_id);

    deleteJob (job_id);
  }
}

void GenericDaemon::finished(const we::layer::id_type& id, const we::type::activity_t& result)
{
  Job* pJob = findJob(id);
  if(!pJob)
  {
    throw std::runtime_error ("got finished message for old/unknown Job " + id);
  }

  pJob->JobFinished (result.to_string());

  if(!isSubscriber(pJob->owner()))
  {
    parent_proxy (this, pJob->owner()).job_finished (id, result.to_string());
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

  BOOST_FOREACH(const subscriber_map_t::value_type& pair_subscr_joblist, m_listSubscribers )
  {
    if(subscribedFor(pair_subscr_joblist.first, id))
    {
      events::SDPAEvent::Ptr ptrEvt
        ( new events::JobFinishedEvent ( name()
                               , pair_subscr_joblist.first
                               , id
                               , result.to_string()
                               )
        );

      sendEventToOther(ptrEvt);
    }
  }
}

void GenericDaemon::failed( const we::layer::id_type& id
                          , std::string const & reason
                          )
{
  Job* pJob = findJob(id);
  if(!pJob)
  {
    throw std::runtime_error ("got failed message for old/unknown Job " + id);
  }

  pJob->JobFailed (reason);

  if(!isSubscriber(pJob->owner()))
  {
    parent_proxy (this, pJob->owner()).job_failed (id, reason);
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

  BOOST_FOREACH( const subscriber_map_t::value_type& pair_subscr_joblist, m_listSubscribers )
  {
    if(subscribedFor(pair_subscr_joblist.first, id))
    {
        events::JobFailedEvent::Ptr ptrEvt
        ( new events::JobFailedEvent ( name()
                             , pair_subscr_joblist.first
                             , id
                             , reason
                             )
        );
      sendEventToOther(ptrEvt);
    }
  }
}

void GenericDaemon::canceled (const we::layer::id_type& job_id)
{
  Job* pJob (findJob (job_id));
  if (!pJob)
  {
    throw std::runtime_error ("rts_canceled (unknown job)");
  }

  pJob->CancelJobAck();

  //! \todo Should be if (job-has-owner), i.e. was-submitted-to-this-daemon
  if (!isTop())
  {
    parent_proxy (this, pJob->owner()).cancel_job_ack (job_id);

    deleteJob (job_id);
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
          parent_proxy (this, job->owner()).job_finished (job->id(), job->result());
        }
        break;

      case sdpa::status::FAILED:
        {
          parent_proxy (this, job->owner()).job_failed
            (job->id(), "unknown error: error event resent");
        }
        break;

      case sdpa::status::CANCELED:
        {
          parent_proxy (this, job->owner()).cancel_job_ack (job->id());
        }
        break;

      case sdpa::status::PENDING:
        {
          parent_proxy (this, job->owner()).submit_job_ack (job->id());
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

    bool bModified = scheduler()->worker_manager().findWorker (worker_id)->addCapabilities (workerCpbSet);

    if(bModified)
    {
      request_scheduling();
      if( !isTop() )
      {
        const sdpa::capabilities_set_t newWorkerCpbSet
          (scheduler()->worker_manager().findWorker (worker_id)->capabilities());

        if( !newWorkerCpbSet.empty() )
        {
          lock_type lock(mtx_master_);
          for( sdpa::master_info_list_t::iterator it = m_arrMasterInfo.begin(); it != m_arrMasterInfo.end(); it++ )
            if( it->is_registered() && it->name() != worker_id  )
            {
              parent_proxy (this, it->name()).capabilities_gained
                (newWorkerCpbSet);
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
    scheduler()->worker_manager().findWorker (worker_id)->removeCapabilities(pCpbLostEvt->capabilities());

    lock_type lock(mtx_master_);
    for( sdpa::master_info_list_t::iterator it = m_arrMasterInfo.begin(); it != m_arrMasterInfo.end(); it++)
      if (it->is_registered() && it->name() != worker_id )
      {
        parent_proxy (this, it->name()).capabilities_lost
          (pCpbLostEvt->capabilities());
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

void GenericDaemon::delay (boost::function<void()> fun)
{
  sendEventToSelf
    (events::SDPAEvent::Ptr (new events::delayed_function_call (fun)));
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
    lock_type lock(mtx_cpb_);
    capabilities_set_t cpbSet (m_capabilities);

    scheduler()->worker_manager().getCapabilities (cpbSet);

    //std::cout<<cpbSet;

    parent_proxy (this, masterInfo.name()).worker_registration
      (boost::none, cpbSet);
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
    m_listSubscribers.insert(subscriber_map_t::value_type(subscriber, listJobIds));
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
        scheduler()->worker_manager().findWorker (worker_id)->acknowledge (pEvent->job_id());
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
      delay ( boost::bind ( &GenericDaemon::delayed_discover, this
                          , discover_id, job_id
                          )
            );
    }
    void GenericDaemon::delayed_discover
      (we::layer::id_type discover_id, we::layer::id_type job_id)
    {
      Job* pJob (findJob (job_id));

      const boost::optional<worker_id_t> worker_id
        (scheduler()->worker_manager().findSubmOrAckWorker(job_id));

      if (pJob && worker_id)
      {
        child_proxy (this, *worker_id).discover_job_states
          (job_id, discover_id);
      }
      else if (pJob)
      {
        workflowEngine()->discovered
          ( discover_id
          , discovery_info_t (job_id, pJob->getStatus(), discovery_info_set_t())
          );
      }
      else
      {
        workflowEngine()->discovered
          ( discover_id
          , discovery_info_t (job_id, boost::none, discovery_info_set_t())
          );
      }
    }

    void GenericDaemon::handleDiscoverJobStatesEvent
      (const sdpa::events::DiscoverJobStatesEvent *pEvt)
    {
      Job* pJob (findJob (pEvt->job_id()));

      const boost::optional<worker_id_t> worker_id
        (scheduler()->worker_manager().findSubmOrAckWorker(pEvt->job_id()));

      if (pJob && worker_id)
      {
        _discover_sources.insert
          ( std::make_pair ( std::make_pair (pEvt->discover_id(), pEvt->job_id())
                           , pEvt->from()
                           )
          );

        child_proxy (this, *worker_id).discover_job_states
          (pEvt->job_id(), pEvt->discover_id());
      }
      //! \todo Other criteria to know it was submitted to the
      //! wfe. All jobs are regarded as going to the wfe and the only
      //! way to prevent a loop is to check whether the discover comes
      //! out of the wfe. Special "worker" id?
      else if (pJob && workflowEngine())
      {
        _discover_sources.insert
          ( std::make_pair ( std::make_pair (pEvt->discover_id(), pEvt->job_id())
                           , pEvt->from()
                           )
          );

        //! \todo There is a race here: between SubmitJobAck and
        //! we->submit(), there's still a lot of stuff. We can't
        //! guarantee, that the job is already submitted to the wfe!
        //! We need to handle the "pending" state.
        workflowEngine()->discover (pEvt->discover_id(), pEvt->job_id());
      }
      else if (pJob)
      {
        parent_proxy (this, pEvt->from()).discover_job_states_reply
          ( pEvt->discover_id()
          , discovery_info_t (pEvt->job_id(), pJob->getStatus(), discovery_info_set_t())
          );
      }
      else
      {
        parent_proxy (this, pEvt->from()).discover_job_states_reply
          ( pEvt->discover_id()
          , discovery_info_t (pEvt->job_id(), boost::none, discovery_info_set_t())
          );
      }
    }

    void GenericDaemon::discovered
      (we::layer::id_type discover_id, sdpa::discovery_info_t discover_result)
    {
      const std::pair<job_id_t, job_id_t> source_id
        (discover_id, discover_result.job_id());

      parent_proxy (this, _discover_sources.at (source_id))
        .discover_job_states_reply (discover_id, discover_result);

      _discover_sources.erase (source_id);
    }

    void GenericDaemon::handleDiscoverJobStatesReplyEvent
      (const sdpa::events::DiscoverJobStatesReplyEvent* e)
    {
      const std::pair<job_id_t, job_id_t> source_id
        (e->discover_id(), e->discover_result().job_id());
      const boost::unordered_map<std::pair<job_id_t, job_id_t>, std::string>::iterator
        source (_discover_sources.find (source_id));

      if (source == _discover_sources.end())
      {
        workflowEngine()->discovered (e->discover_id(), e->discover_result());
      }
      else
      {
        parent_proxy (this, source->second).discover_job_states_reply
          (e->discover_id(), e->discover_result());

        _discover_sources.erase (source);
      }
    }


void GenericDaemon::handleRetrieveJobResultsEvent(const events::RetrieveJobResultsEvent* pEvt )
{
  Job* pJob = findJob(pEvt->job_id());
  if(pJob)
  {
      if(sdpa::status::is_terminal (pJob->getStatus()))
      {
        parent_proxy (this, pEvt->from()).retrieve_job_results_reply
          (pEvt->job_id(), pJob->result());
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
    parent_proxy (this, pEvt->from()).query_job_status_reply
      (pJob->id(), pJob->getStatus(), pJob->error_message());
  }
  else
  {
    throw std::runtime_error ("Inexistent job: "+pEvt->job_id());
  }
}

}}

namespace sdpa
{
  namespace daemon
  {
    void GenericDaemon::scheduling_thread()
    {
      for (;;)
      {
        boost::mutex::scoped_lock lock (_scheduling_thread_mutex);
        _scheduling_thread_notifier.wait (lock);

        scheduler()->assignJobsToWorkers();
      }
    }

    void GenericDaemon::request_scheduling()
    {
      boost::mutex::scoped_lock const _ (_scheduling_thread_mutex);
      _scheduling_thread_notifier.notify_one();
    }

    GenericDaemon::child_proxy::child_proxy
        (GenericDaemon* that, worker_id_t name)
      : _that (that)
      , _name (name)
    {}

    void GenericDaemon::child_proxy::worker_registration_ack() const
    {
      _that->sendEventToOther
        ( events::WorkerRegistrationAckEvent::Ptr
          (new events::WorkerRegistrationAckEvent (_that->name(), _name))
        );
    }

    void GenericDaemon::child_proxy::submit_job ( boost::optional<job_id_t> id
                                                , job_desc_t description
                                                , sdpa::worker_id_list_t workers
                                                ) const
    {
      _that->sendEventToOther
        ( events::SubmitJobEvent::Ptr
          ( new events::SubmitJobEvent
            (_that->name(), _name, id, description, workers)
          )
        );
    }

    void GenericDaemon::child_proxy::cancel_job (job_id_t id) const
    {
      _that->sendEventToOther
        ( events::CancelJobEvent::Ptr
          (new events::CancelJobEvent (_that->name(), _name, id))
        );
    }

    void GenericDaemon::child_proxy::job_failed_ack (job_id_t id) const
    {
      _that->sendEventToOther
        ( events::JobFailedAckEvent::Ptr
          (new events::JobFailedAckEvent (_that->name(), _name, id))
        );
    }

    void GenericDaemon::child_proxy::job_finished_ack (job_id_t id) const
    {
      _that->sendEventToOther
        ( events::JobFinishedAckEvent::Ptr
          (new events::JobFinishedAckEvent (_that->name(), _name, id))
        );
    }

    void GenericDaemon::child_proxy::discover_job_states
      (job_id_t job_id, job_id_t discover_id) const
    {
      _that->sendEventToOther
        ( events::DiscoverJobStatesEvent::Ptr
          ( new events::DiscoverJobStatesEvent
            (_that->name(), _name, job_id, discover_id)
          )
        );
    }

    GenericDaemon::parent_proxy::parent_proxy
        (GenericDaemon* that, worker_id_t name)
      : _that (that)
      , _name (name)
    {}

    void GenericDaemon::parent_proxy::worker_registration
      ( boost::optional<unsigned int> capacity
      , capabilities_set_t capabilities
      ) const
    {
      _that->sendEventToOther
        ( events::WorkerRegistrationEvent::Ptr
          ( new events::WorkerRegistrationEvent
            (_that->name(), _name, capacity, capabilities)
          )
        );
    }

    void GenericDaemon::parent_proxy::job_failed
      (job_id_t id, std::string message) const
    {
      _that->sendEventToOther
        ( events::JobFailedEvent::Ptr
          (new events::JobFailedEvent (_that->name(), _name, id, message))
        );
    }

    void GenericDaemon::parent_proxy::job_finished
      (job_id_t id, job_result_t result) const
    {
      _that->sendEventToOther
        ( events::JobFinishedEvent::Ptr
          (new events::JobFinishedEvent (_that->name(), _name, id, result))
        );
    }

    void GenericDaemon::parent_proxy::cancel_job_ack (job_id_t id) const
    {
      _that->sendEventToOther
        ( events::CancelJobAckEvent::Ptr
          (new events::CancelJobAckEvent (_that->name(), _name, id))
        );
    }

    void GenericDaemon::parent_proxy::delete_job_ack (job_id_t id) const
    {
      _that->sendEventToOther
        ( events::DeleteJobAckEvent::Ptr
          (new events::DeleteJobAckEvent (_that->name(), _name, id))
        );
    }

    void GenericDaemon::parent_proxy::submit_job_ack (job_id_t id) const
    {
      _that->sendEventToOther
        ( events::SubmitJobAckEvent::Ptr
          (new events::SubmitJobAckEvent (_that->name(), _name, id))
        );
    }

    void GenericDaemon::parent_proxy::capabilities_gained
      (capabilities_set_t capabilities) const
    {
      _that->sendEventToOther
        ( events::CapabilitiesGainedEvent::Ptr
          ( new events::CapabilitiesGainedEvent
            (_that->name(), _name, capabilities)
          )
        );
    }

    void GenericDaemon::parent_proxy::capabilities_lost
      (capabilities_set_t capabilities) const
    {
      _that->sendEventToOther
        ( events::CapabilitiesLostEvent::Ptr
          ( new events::CapabilitiesLostEvent
            (_that->name(), _name, capabilities)
          )
        );
    }

    void GenericDaemon::parent_proxy::discover_job_states_reply
      (job_id_t discover_id, discovery_info_t info) const
    {
      _that->sendEventToOther
        ( events::DiscoverJobStatesReplyEvent::Ptr
          ( new events::DiscoverJobStatesReplyEvent
            (_that->name(), _name, discover_id, info)
          )
        );
    }

    void GenericDaemon::parent_proxy::query_job_status_reply
      (job_id_t id, status::code status, std::string error_message) const
    {
      _that->sendEventToOther
        ( events::JobStatusReplyEvent::Ptr
          ( new events::JobStatusReplyEvent
            (_that->name(), _name, id, status, error_message)
          )
        );
    }

    void GenericDaemon::parent_proxy::retrieve_job_results_reply
      (job_id_t id, job_result_t result) const
    {
      _that->sendEventToOther
        ( events::JobResultsReplyEvent::Ptr
          (new events::JobResultsReplyEvent (_that->name(), _name, id, result))
        );
    }
  }
}
