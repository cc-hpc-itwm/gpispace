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
#include <seda/StageRegistry.hpp>
#include <sdpa/events/CodecStrategy.hpp>
#include <seda/EventPrioQueue.hpp>

#include <sdpa/daemon/GenericDaemon.hpp>
#include <sdpa/events/CapabilitiesGainedEvent.hpp>
#include <sdpa/events/CapabilitiesLostEvent.hpp>

#include <sdpa/com/NetworkStrategy.hpp>
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

using namespace std;
using namespace sdpa::daemon;
using namespace sdpa::events;

//constructor
GenericDaemon::GenericDaemon( const std::string name,
                              const master_info_list_t arrMasterInfo,
                              unsigned int rank
                            , const boost::optional<std::string>& guiUrl
                            , bool create_wfe
                            )
  : Strategy(name),
    SDPA_INIT_LOGGER(name),
    m_arrMasterInfo(arrMasterInfo),
    m_to_master_stage_name_(name+".net"),
    m_to_slave_stage_name_ (name+".net"),

    ptr_job_man_(new JobManager(name)),
    ptr_scheduler_(),
    ptr_workflow_engine_ ( create_wfe
                         ? new we::mgmt::layer
                           (this, boost::bind (&GenericDaemon::gen_id, this))
                         : NULL
                         ),
    m_nRank(rank),
    m_strAgentUID(id_generator<agent_id_tag>::instance().next()),
    m_guiService ( guiUrl && !guiUrl->empty()
                 ? boost::optional<NotificationService>
                   (NotificationService (*guiUrl))
                 : boost::none
                 )
  , _max_consecutive_registration_attempts (360)
  , _max_consecutive_network_faults (360)
  , _registration_timeout (boost::posix_time::seconds (1))
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

  _stages_to_remove.push_back (name);
}

void GenericDaemon::start_agent()
{
  ptr_daemon_stage_.lock()->start();


  createScheduler();
  scheduler()->start();


  const boost::tokenizer<boost::char_separator<char> > tok
    (url(), boost::char_separator<char> (":"));

  const std::vector<std::string> vec (tok.begin(), tok.end());

  if (vec.empty() || vec.size() > 2)
  {
    LOG (ERROR, "Invalid daemon url.  Please specify it in the form <hostname (IP)>:<port>!");
    throw std::runtime_error ("configuration of network failed: invalid url");
  }

  sdpa::com::NetworkStrategy::ptr_t net
    ( new sdpa::com::NetworkStrategy ( name() /*fallback stage = agent*/
                                     , name() /*name for peer*/
                                     , fhg::com::host_t (vec[0])
                                     , fhg::com::port_t (vec.size() == 2 ? vec[1] : "0")
                                     )
    );

  seda::Stage::Ptr network_stage
    (new seda::Stage (m_to_master_stage_name_, net));

  seda::StageRegistry::instance().insert (network_stage);
  _stages_to_remove.push_back (m_to_master_stage_name_);

  ptr_to_master_stage_ = ptr_to_slave_stage_ = network_stage;

  to_master_stage()->start();


  if (!isTop())
  {
    lock_type lock (mtx_master_);
    BOOST_FOREACH (sdpa::MasterInfo& masterInfo, m_arrMasterInfo)
    {
      requestRegistration (masterInfo);
    }
  }
}

/**
 * Shutdown an agent
 */
void GenericDaemon::shutdown( )
{
  DMLOG (TRACE, "Shutting down the component "<<name()<<" ...");

  BOOST_FOREACH (sdpa::MasterInfo& masterInfo, m_arrMasterInfo)
  {
    if (!masterInfo.name().empty() && masterInfo.is_registered())
    {
      sendEventToMaster
        ( ErrorEvent::Ptr ( new ErrorEvent ( name()
                                           , masterInfo.name()
                                           , ErrorEvent::SDPA_ENODE_SHUTDOWN
                                           , "node shutdown"
                                           )
                          )
        );
    }
  }

  BOOST_REVERSE_FOREACH (std::string stage, _stages_to_remove)
  {
    seda::StageRegistry::instance().lookup (stage)->stop();
    seda::StageRegistry::instance().remove (stage);
  }

  ptr_scheduler_.reset();

  delete ptr_workflow_engine_;
  ptr_workflow_engine_ = NULL;

	DMLOG (TRACE, "Succesfully shut down  "<<name()<<" ...");
}


void GenericDaemon::perform(const seda::IEvent::Ptr& pEvent)
{
  if( SDPAEvent* pSdpaEvt = dynamic_cast<SDPAEvent*>(pEvent.get()) )
  {
    try
    {
      pSdpaEvt->handleBy(this);
    }
    catch (std::exception const & ex)
    {
      LOG( ERROR, "could not handle event "<< "\""  << pEvent->str() << "\""<< " : " << ex.what());
    }
  }
  else
  {
    DMLOG (TRACE, "Received unexpected event " << pEvent->str()<<". Cannot handle it!");
    //! \todo THROW
  }
}

void GenericDaemon::addJob( const sdpa::job_id_t& jid, const Job::ptr_t& pJob, const job_requirements_t& reqList)
{
  return jobManager()->addJob(jid, pJob, reqList);
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
    bool _dont;
    boost::function<void()> _what;
  };
}

//actions
void GenericDaemon::handleDeleteJobEvent (const DeleteJobEvent* evt)
{
  const DeleteJobEvent& e (*evt);

  DMLOG (TRACE, e.from() << " requesting to delete job " << e.job_id() );

  on_scope_exit _ ( boost::bind ( &GenericDaemon::sendEventToMaster, this
                                , ErrorEvent::Ptr ( new ErrorEvent ( name()
                                                                   , e.from()
                                                                   , ErrorEvent::SDPA_EUNKNOWN
                                                                   , "unknown"
                                                                   )
                                                  )
                               )
                  );

  try{
    Job::ptr_t pJob = jobManager()->findJob(e.job_id());
    pJob->DeleteJob(&e, this);

    jobManager()->deleteJob(e.job_id());
  }
  catch(JobNotFoundException const &)
  {
    DMLOG (WARN, "Job " << e.job_id() << " could not be found!");
    sendEventToMaster( ErrorEvent::Ptr( new ErrorEvent( name()
                                                        , e.from()
                                                        , ErrorEvent::SDPA_EJOBNOTFOUND
                                                        , "no such job"
                                                        )
                                        )
               );
  }
  catch(JobNotDeletedException const & ex)
  {
    DMLOG (WARN, "Job " << e.job_id() << " could not be deleted!");
    sendEventToMaster( ErrorEvent::Ptr( new ErrorEvent( name()
                                                        , e.from()
                                                        , ErrorEvent::SDPA_EUNKNOWN
                                                        , ex.what()
                                                        )
                                        )
               );
  }

  _.dont();
}

void GenericDaemon::serveJob(const Worker::worker_id_t& worker_id, const job_id_t& jobId )
{
  //take a job from the workers' queue and serve it

  try {
    // you should consume from the  worker's pending list; put the job into the worker's submitted list
    //sdpa::job_id_t jobId = scheduler()->getNextJob(worker_id, last_job_id);
	//check first if the worker exist
	//Worker::ptr_t ptrWorker(findWorker(worker_id));

    DMLOG(TRACE, "Assign the job "<<jobId<<" to the worker '"<<worker_id);

    const Job::ptr_t& ptrJob = jobManager()->findJob(jobId);

    DMLOG(TRACE, "Serving a job to the worker "<<worker_id);

    // create a SubmitJobEvent for the job job_id serialize and attach description
    sdpa::worker_id_list_t worker_list(1,worker_id);
    LOG(TRACE, "Submit the job "<<ptrJob->id()<<" to the worker " << worker_id);
    LOG(TRACE, "The job "<<ptrJob->id()<<" was assigned the following workers:"<<worker_list);
    SubmitJobEvent::Ptr pSubmitEvt(new SubmitJobEvent(name(), worker_id, ptrJob->id(),  ptrJob->description(), "", worker_list));

    // Post a SubmitJobEvent to the slave who made the request
    sendEventToSlave(pSubmitEvt);
  }
  catch(const JobNotFoundException&)
  {
      LOG (ERROR, "Couldn't find the job "<<jobId<<" when attempting to serve workers!");
  }
}

void GenericDaemon::serveJob(const sdpa::worker_id_list_t& worker_list, const job_id_t& jobId)
{
  //take a job from the workers' queue and serve it

  try {
    const Job::ptr_t& ptrJob = jobManager()->findJob(jobId);

    // create a SubmitJobEvent for the job job_id serialize and attach description
    LOG(TRACE, "The job "<<ptrJob->id()<<" was assigned the following workers:"<<worker_list);

    BOOST_FOREACH(const worker_id_t& worker_id, worker_list)
    {
      SubmitJobEvent::Ptr pSubmitEvt(new SubmitJobEvent(name(), worker_id, ptrJob->id(),  ptrJob->description(), "", worker_list));

      // Post a SubmitJobEvent to the slave who made the request
      sendEventToSlave(pSubmitEvt);
    }
  }
  catch(const JobNotFoundException&)
  {
    LOG (ERROR, "Couldn't find the job "<<jobId<<" when attempting to serve workers!");
  }
}

bool hasName(const sdpa::MasterInfo& masterInfo, const std::string& name)
{
  return masterInfo.name() == name;
}

void GenericDaemon::handleSubmitJobEvent (const SubmitJobEvent* evt)
{
  const SubmitJobEvent& e (*evt);

  DLOG(TRACE, "got job submission from " << e.from() << ": job-id := " << e.job_id());

  // check if the incoming event was produced by a master to which the current agent has already registered
  //BOOST_FOREACH(sdpa::MasterInfo& masterInfo, m_arrMasterInfo)
  lock_type lock(mtx_master_);
  master_info_list_t::iterator itMaster = find_if(m_arrMasterInfo.begin(), m_arrMasterInfo.end(), boost::bind(hasName, _1, e.from()));

  if( itMaster != m_arrMasterInfo.end() && !itMaster->is_registered() )
  {
    DMLOG (TRACE, "The agent "<<name()<<" is not yet registered with the master "<<itMaster->name()
          <<". No job from this master will be accepted as long as no registration confirmation has been received!");

    //send job rejected error event back to the master
    ErrorEvent::Ptr pErrorEvt(new ErrorEvent( name(),
                                              e.from(),
                                              ErrorEvent::SDPA_EPERM,
                                              "Waiting for registration confirmation. No job submission is allowed!",
                                              e.job_id()) );
    sendEventToMaster(pErrorEvt);

    return;
  }
  lock.unlock();

  static const JobId job_id_empty ("");

  // First, check if the job 'job_id' wasn't already submitted!
  try {
    jobManager()->findJob(e.job_id());
    // The job already exists -> generate an error message that the job already exists

    DMLOG (WARN, "The job with job-id: " << e.job_id()<<" does already exist! (possibly recovered)");
    if( e.from() != sdpa::daemon::WE ) //e.to())
    {
        ErrorEvent::Ptr pErrorEvt(new ErrorEvent(name(), e.from(), ErrorEvent::SDPA_EJOBEXISTS, "The job already exists!", e.job_id()) );
        sendEventToMaster(pErrorEvt);
    }

    return;
  }
  catch(const JobNotFoundException&)
  {
    DMLOG (TRACE, "Receive new job from "<<e.from() << " with job-id: " << e.job_id());
  }

  JobId job_id; //already assigns an unique job_id (i.e. the constructor calls the generator)
  if(e.job_id() != job_id_empty)  // use the job_id already  assigned by the master
    job_id = e.job_id();          // the orchestrator will assign a new job_id for the user jobs,
                                  // the Agg/NRE will use the job_id assigned by the master

  try {
    // One should parse the workflow in order to be able to create a valid job
    // if the event comes from Gwes parent_id is the owner_workflow_id
    Job::ptr_t pJob(new Job(job_id, e.description(), e.parent_id()));
    pJob->set_owner(e.from());

    // the job job_id is in the Pending state now!
    jobManager()->addJob(job_id, pJob);

    // check if the message comes from outside/slave or from WFE
    // if it comes from outside set it as local
    if( e.from() != sdpa::daemon::WE && hasWorkflowEngine() )
    {
      DMLOG (TRACE, "got new job from " << e.from() << " = " << job_id);
      pJob->setType(Job::MASTER);

      if (m_guiService)
      {
        std::list<std::string> workers; workers.push_back (name());
        const we::mgmt::type::activity_t act (pJob->description());
        const sdpa::daemon::NotificationEvent evt
          ( workers
          , pJob->id().str()
          , NotificationEvent::STATE_STARTED
          , act
          );

        m_guiService->notify (evt);
      }

      //scheduler()->schedule_local(job_id);
      submitWorkflow(job_id);
    }
    else
      scheduler()->enqueueJob(job_id);

    if( e.from() != sdpa::daemon::WE )
    {
      // send back to the user a SubmitJobAckEvent
      SubmitJobAckEvent::Ptr pSubmitJobAckEvt(new SubmitJobAckEvent(name(), e.from(), job_id));

      // There is a problem with this if uncommented
      sendEventToMaster(pSubmitJobAckEvt);
    }
  }
  catch(JobNotAddedException const &ex)
  {
    DMLOG (WARN, "job " << job_id << " could not be added: " << ex.what());
    // the worker should register first, before posting a job request
    ErrorEvent::Ptr pErrorEvt(new ErrorEvent(name(), e.from(), ErrorEvent::SDPA_EUNKNOWN, ex.what()) );
    sendEventToMaster(pErrorEvt);
  }
}

void GenericDaemon::handleWorkerRegistrationEvent (const WorkerRegistrationEvent* evt)
{
  const WorkerRegistrationEvent& evtRegWorker (*evt);

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
    	  const Worker::ptr_t& pWorker = findWorker(worker_id);

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
      WorkerRegistrationAckEvent::Ptr const pWorkerRegAckEvt
        (new WorkerRegistrationAckEvent ( name(), evtRegWorker.from()));

      sendEventToSlave(pWorkerRegAckEvt);
    }
  }
}

void GenericDaemon::handleErrorEvent (const ErrorEvent* evt)
{
  const sdpa::events::ErrorEvent& error (*evt);

  DMLOG(TRACE, "got error event from " << error.from() << " code: " << error.error_code() << " reason: " << error.reason());

  // if it'a communication error, inspect all jobs and
  // send results if they are in a terminal state

  switch (error.error_code())
  {
    case ErrorEvent::SDPA_ENOERROR:
    {
      // everything is fine, nothing to do
      break;
    }
    // this  should  better go  into  a  distinct  event, since  the  ErrorEvent
    // 'reason' should not be reused for important information
    case ErrorEvent::SDPA_EJOBREJECTED:
    {
      sdpa::job_id_t jobId(error.job_id());
      sdpa::worker_id_t worker_id(error.from());
      DMLOG (WARN, "The worker "<<worker_id<<" rejected the job "<<error.job_id().str()<<". Reschedule it now!");

      scheduler()->rescheduleWorkerJob(worker_id, jobId);
      break;
    }
    case ErrorEvent::SDPA_EWORKERNOTREG:
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
              DMLOG (TRACE, "Wait " << boost::posix_time::to_simple_string (_registration_timeout) << " before trying to re-register ...");
              boost::this_thread::sleep (_registration_timeout);
              requestRegistration(masterInfo);
            }
            else
              listDeadMasters.push_back( masterInfo.name() );
          }
        }
      }

      removeMasters(listDeadMasters);

      break;
    }
    case ErrorEvent::SDPA_ENODE_SHUTDOWN:
    case ErrorEvent::SDPA_ENETWORKFAILURE:
    {
      if( isSubscriber(error.from()) )
        unsubscribe(error.from());

      worker_id_t worker_id(error.from());

      try
      {
        Worker::ptr_t ptrWorker = findWorker(worker_id);

        if(ptrWorker)
        {
          DMLOG (TRACE, "worker " << worker_id << " went down (clean). Tell the WorkerManager to remove it!");

          // notify capability losses...
          lock_type lock(mtx_master_);
          BOOST_FOREACH(sdpa::MasterInfo& masterInfo, m_arrMasterInfo)
          {
            sdpa::events::CapabilitiesLostEvent::Ptr shpCpbLostEvt(
                                  new sdpa::events::CapabilitiesLostEvent( name(),
                                                                           masterInfo.name(),
                                                                           ptrWorker->capabilities()
                                                                           ));

            sendEventToMaster(shpCpbLostEvt);
          }

          // if there still are registered workers, otherwise declare the remaining
          // jobs failed
          //scheduler()->reschedule(worker_id);
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
              masterInfo.incConsecNetFailCnt();

              if( masterInfo.getConsecNetFailCnt() < _max_consecutive_network_faults)
              {
                boost::this_thread::sleep (_registration_timeout);

                masterInfo.set_registered(false);
              }
              else
                listDeadMasters.push_back( masterInfo.name() );
            }
          }
        }

        removeMasters(listDeadMasters);
      }
    }
    case ErrorEvent::SDPA_EJOBEXISTS:
    {
      DMLOG (TRACE, "The worker managed to recover the job "<<error.job_id()<<", it already has it!");
      // do the same as when receiving a SubmitJobAckEvent

      Worker::worker_id_t worker_id = error.from();
      try {
        // Only now should be the job state machine make a transition to RUNNING
        // this means that the job was not rejected, no error occurred etc ....
        // find the job ptrJob and call
        Job::ptr_t ptrJob = jobManager()->findJob(error.job_id());
        ptrJob->Dispatch();
        scheduler()->acknowledgeJob(worker_id, error.job_id());
      }
      catch(JobNotFoundException const& ex)
      {
        DMLOG (WARN, "The job " << error.job_id() << " was not found on"<<name()<<"!");
      }
      catch(WorkerNotFoundException const &)
      {
        DMLOG (WARN, "job re-submission could not be acknowledged: worker " << worker_id << " not found!");
      }

      break;
    }
    case ErrorEvent::SDPA_EPERM:
    {
    	sdpa::job_id_t jobId(error.job_id());
    	sdpa::worker_id_t worker_id(error.from());
    	DMLOG (WARN, "The worker "<<worker_id<<" rejected the job "<<error.job_id().str()<<". Reschedule it now!");

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
void GenericDaemon::submit( const id_type& activityId
                          , const encoded_type& desc
                          , const requirement_list_t& req_list
                          , const we::type::schedule_data& schedule_data
                          , const we::type::user_data& user_data
                          )
{
  // create new job with the job description = workflow (serialize it first)
  // set the parent_id to ?
  // add this job into the parent's job list (call parent_job->add_subjob( new job(workflow) ) )
  // schedule the new job to some worker
  job_requirements_t jobReqs(req_list, schedule_data);

    DMLOG(TRACE, "workflow engine submitted "<<activityId);

    job_id_t job_id(activityId);
    job_id_t parent_id(user_data.get_user_job_identification());

    on_scope_exit _ ( boost::bind ( &we::mgmt::layer::failed, workflowEngine()
                                  , activityId, desc
                                  , fhg::error::UNEXPECTED_ERROR
                                  , "Exception in GenericDaemon::submit()"
                                  )
                    );

    try {
        jobManager()->findJob(parent_id);
        jobManager()->addJobRequirements(job_id, jobReqs);

        // WORK HERE: limit number of maximum parallel jobs
        jobManager()->waitForFreeSlot ();

        // don't forget to set here the job's preferences
        SubmitJobEvent::Ptr pEvtSubmitJob( new SubmitJobEvent( sdpa::daemon::WE, name(), job_id, desc, parent_id) );
        sendEventToSelf(pEvtSubmitJob);
      }
      catch(const JobNotFoundException& ex)
      {
          DLOG(WARN, "Could not find the parent job "<<parent_id<<" indicated by the workflow engine in the last submission!");
          workflowEngine()->failed( activityId
                                  , desc
                                  , fhg::error::UNEXPECTED_ERROR
                                  , "Could not find the parent job "+parent_id.str()
                                  );
      }

  _.dont();
}

/**
 * Cancel an atomic activity that has previously been submitted to
 * the SDPA.
 */
bool GenericDaemon::cancel(const id_type& activityId, const reason_type & reason)
{
  DMLOG (TRACE, "The workflow engine requests the cancellation of the activity " << activityId << "( reason: " << reason<<")!");

  // cancel the job corresponding to that activity -> send downward a CancelJobEvent?
  // look for the job_id corresponding to the received workflowId into job_map_
  // in fact they should be the same!
  // generate const CancelJobEvent& event
  // Job& job = job_map_[job_id];
  // call job.CancelJob(event);

  job_id_t job_id(activityId);
  CancelJobEvent::Ptr pEvtCancelJob
          (new CancelJobEvent( sdpa::daemon::WE
                              , name()
                              , job_id
                              , reason )
          );
  sendEventToSelf(pEvtCancelJob);
  return true;
}

/**
 * Notify the SDPA that a workflow finished (state transition
 * from running to finished).
 */
bool GenericDaemon::finished(const id_type& workflowId, const result_type& result)
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
  JobFinishedEvent::Ptr pEvtJobFinished( new JobFinishedEvent( sdpa::daemon::WE, name(), job_id, result ));
  sendEventToSelf(pEvtJobFinished);

  // notify the GUI that the activity finished

  return true;
}

/**
 * Notify the SDPA that a workflow failed (state transition
 * from running to failed).
 */
bool GenericDaemon::failed( const id_type& workflowId
                          , const result_type & result
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

  JobFailedEvent::Ptr pEvtJobFailed
    (new JobFailedEvent ( sdpa::daemon::WE
                        , name()
                        , job_id
                        , result
                        , error_code
                        , reason
                        )
    );

  sendEventToSelf(pEvtJobFailed);

  return true;
}

/**
 * Notify the SDPA that a workflow has been canceled (state
 * transition from * to terminated.
 */
bool GenericDaemon::canceled(const id_type& workflowId)
{
  DMLOG (TRACE, "activity canceled: " << workflowId);
  // generate a JobCanceledEvent for self!

  job_id_t job_id(workflowId);

  CancelJobAckEvent::Ptr pEvtCancelJobAck( new CancelJobAckEvent(sdpa::daemon::WE, name(), job_id ));
  sendEventToSelf(pEvtCancelJobAck);

  return true;
}

Job::ptr_t& GenericDaemon::findJob(const sdpa::job_id_t& job_id ) const
{
  return jobManager()->findJob(job_id);
}

void GenericDaemon::deleteJob(const sdpa::job_id_t& jobId)
{
  jobManager()->deleteJob(jobId);
}

const job_requirements_t GenericDaemon::getJobRequirements(const sdpa::job_id_t& jobId) const
{
  return jobManager()->getJobRequirements(jobId);
}

/*void GenericDaemon::submitWorkflow(const id_type& wf_id, const encoded_type& desc )
{
  if(!hasWorkflowEngine())
    throw NoWorkflowEngine();

  workflowEngine()->submit (wf_id, desc, we::type::user_data ());
}*/

/*
        Schedule a job locally, send the job to WE
*/
void GenericDaemon::submitWorkflow(const sdpa::job_id_t &jobId)
{
  DMLOG (TRACE, "Schedule the job "<<jobId.str()<<" to the workflow engine!");

  id_type wf_id = jobId.str();

  try {
    const Job::ptr_t& pJob = findJob(jobId);

    // Should set the workflow_id here, or send it together with the workflow description
    DMLOG (TRACE, "The status of the job "<<jobId<<" is "<<pJob->getStatus());
    DMLOG (TRACE, "Submit the workflow attached to the job "<<jobId<<" to WE. ");
    pJob->Dispatch();
    DMLOG (TRACE, "The status of the job "<<jobId<<" is "<<pJob->getStatus());

    if(pJob->description().empty() )
    {
        SDPA_LOG_ERROR("Empty Workflow!");
        // declare job as failed
        JobFailedEvent::Ptr pEvtJobFailed
              (new JobFailedEvent( sdpa::daemon::WE
                                 , name()
                                 , jobId
                                 , ""
                                 , fhg::error::UNEXPECTED_ERROR
                                 , "The job has an empty workflow attached!"
                                 )
              );

        sendEventToSelf(pEvtJobFailed);
    }

    we::type::user_data job_data;
    job_data.set_user_job_identification(jobId);
    // actually, this information redundant because wf_id == job_data.get_user_job_identification()!
    workflowEngine()->submit (wf_id, pJob->description(), job_data);
  }
  catch(const NoWorkflowEngine& ex)
  {
    SDPA_LOG_ERROR("No workflow engine!");
    sdpa::job_result_t result(ex.what());

    JobFailedEvent::Ptr pEvtJobFailed
      (new JobFailedEvent( sdpa::daemon::WE
                         , name()
                         , jobId
                         , result
                         , fhg::error::UNEXPECTED_ERROR
                         , "no workflow engine attached!"
                         )
      );
    sendEventToSelf(pEvtJobFailed);
  }
  catch(const JobNotFoundException& ex)
  {
    SDPA_LOG_ERROR("Job not found! Could not schedule locally the job "<<ex.job_id().str());
    sdpa::job_result_t result(ex.what());

    JobFailedEvent::Ptr pEvtJobFailed
      (new JobFailedEvent( sdpa::daemon::WE
                         , name()
                         , jobId
                         , result
                         , fhg::error::UNEXPECTED_ERROR
                         , "job could not be found"
                         )
      );

    sendEventToSelf(pEvtJobFailed);
  }
  catch (std::exception const & ex)
  {
    SDPA_LOG_ERROR("Exception occurred when trying to submit the workflow "<<wf_id<<" to WE: "<<ex.what());

    //send a JobFailed event
    sdpa::job_result_t result(ex.what());

    JobFailedEvent::Ptr pEvtJobFailed
      (new JobFailedEvent( sdpa::daemon::WE
                         , name()
                         , jobId
                         , result
                         , fhg::error::UNEXPECTED_ERROR
                         , ex.what()
                         )
      );
    sendEventToSelf(pEvtJobFailed);
  }
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

  // for all jobs that are in a terminal state and not yet acknowledged by the  master
  // re-submit  them to the master, after registration

  if(!isTop())
    jobManager()->resubmitResults(this);
}

void GenericDaemon::registerWorker(const WorkerRegistrationEvent& evtRegWorker)
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
  WorkerRegistrationAckEvent::Ptr pWorkerRegAckEvt(new WorkerRegistrationAckEvent(name(), worker_id));

  sendEventToSlave(pWorkerRegAckEvt);

  if( !workerCpbSet.empty() && !isTop() )
  {
    lock_type lock(mtx_master_);
    // send to the masters my new set of capabilities
    for( sdpa::master_info_list_t::iterator it = m_arrMasterInfo.begin(); it != m_arrMasterInfo.end(); it++ )
      if (it->is_registered() && it->name() != worker_id  )
      {
        CapabilitiesGainedEvent::Ptr shpCpbGainEvt(new CapabilitiesGainedEvent(name(), it->name(), workerCpbSet));
        sendEventToMaster(shpCpbGainEvt);
      }
  }
}

void GenericDaemon::handleCapabilitiesGainedEvent(const sdpa::events::CapabilitiesGainedEvent* pCpbGainEvt)
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
        //getCapabilities(newWorkerCpbSet);

        if( !newWorkerCpbSet.empty() )
        {
          lock_type lock(mtx_master_);
          for( sdpa::master_info_list_t::iterator it = m_arrMasterInfo.begin(); it != m_arrMasterInfo.end(); it++ )
            if( it->is_registered() && it->name() != worker_id  )
            {
              CapabilitiesGainedEvent::Ptr shpCpbGainEvt(new CapabilitiesGainedEvent(name(), it->name(), newWorkerCpbSet));
              sendEventToMaster(shpCpbGainEvt);
            }
        }
      }
    }
  }
  catch( const WorkerNotFoundException& ex )
  {
    DMLOG (WARN, "Could not add new capabilities. The worker "<<worker_id<<" was not found!");
  }
  catch( const AlreadyHasCpbException& ex )
  {
  }
}

void GenericDaemon::handleCapabilitiesLostEvent(const sdpa::events::CapabilitiesLostEvent* pCpbLostEvt)
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
        CapabilitiesLostEvent::Ptr shpCpbLostEvt(new CapabilitiesLostEvent(name(), it->name(), pCpbLostEvt->capabilities()));
        sendEventToMaster(shpCpbLostEvt);
      }
  }
  catch( const WorkerNotFoundException& ex)
  {
    DMLOG (WARN, "Could not remove the specified capabilities. The worker "<<worker_id<<" was not found!");
  }
}

void GenericDaemon::handleSubscribeEvent( const sdpa::events::SubscribeEvent* pEvt )
{
  DMLOG(TRACE, "Received subscribe event!");
  subscribe(pEvt->subscriber(), pEvt->listJobIds());
}

void GenericDaemon::sendEventToSelf(const SDPAEvent::Ptr& pEvt)
{
  ptr_daemon_stage_.lock()->send(pEvt);
  DLOG(TRACE, "Sent " <<pEvt->str()<<" to "<<pEvt->to());
}

void GenericDaemon::sendEventToMaster(const sdpa::events::SDPAEvent::Ptr& pEvt)
{
  to_master_stage()->send(pEvt);
  DLOG(TRACE, "Sent " <<pEvt->str()<<" to "<<pEvt->to());
}

void GenericDaemon::sendEventToSlave(const sdpa::events::SDPAEvent::Ptr& pEvt)
{
  to_slave_stage()->send(pEvt);
  DLOG(TRACE, "Sent " <<pEvt->str()<<" to "<<pEvt->to());
}

Worker::ptr_t const & GenericDaemon::findWorker(const Worker::worker_id_t& worker_id ) const
{
  return scheduler()->findWorker(worker_id);
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

    WorkerRegistrationEvent::Ptr pEvtWorkerReg(new WorkerRegistrationEvent( name(), masterInfo.name(), boost::none, cpbSet,  rank(), agent_uuid()));
    sendEventToMaster(pEvtWorkerReg);
  }
}

void GenericDaemon::addMaster(const agent_id_t& newMasterId )
{
  lock_type lock(mtx_master_);
  MasterInfo mInfo(newMasterId);
  m_arrMasterInfo.push_back(mInfo);
  requestRegistration(mInfo);
}

void GenericDaemon::addMasters(const agent_id_list_t& listMasters )
{
  lock_type lock(mtx_master_);
  for( sdpa::agent_id_list_t::const_iterator it = listMasters.begin(); it != listMasters.end(); it ++ )
  {
    MasterInfo mInfo(*it);
    m_arrMasterInfo.push_back(mInfo);
    requestRegistration(mInfo);
  }
}

bool hasId(sdpa::MasterInfo& info, sdpa::agent_id_t& agId)
{
  return info.name() == agId;
}

void GenericDaemon::removeMaster( const agent_id_t& id )
{
  lock_type lock(mtx_master_);

  master_info_list_t::iterator it = find_if( m_arrMasterInfo.begin(), m_arrMasterInfo.end(), boost::bind(hasId, _1, id) );
  if( it != m_arrMasterInfo.end() )
    m_arrMasterInfo.erase(it);
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

void GenericDaemon::getWorkerCapabilities(const Worker::worker_id_t& worker_id, sdpa::capabilities_set_t& wCpbset)
{
  scheduler()->getWorkerCapabilities(worker_id, wCpbset);
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
  sendEventToMaster(ptrSubscAckEvt);

  // check if the subscribed jobs are already in a terminal state
  BOOST_FOREACH(const sdpa::JobId& jobId, listJobIds)
  {
	  try {
		  Job::ptr_t& pJob = findJob(jobId);
      switch (pJob->getStatus())
      {
      case sdpa::status::FINISHED:
        {
          JobFinishedEvent::Ptr pEvtJobFinished
            (new JobFinishedEvent( name()
                                 , subscriber
                                 , pJob->id()
                                 , pJob->result()
                                 ));
          sendEventToMaster(pEvtJobFinished);
        }
        break;

      case sdpa::status::FAILED:
        {
          JobFailedEvent::Ptr pEvtJobFailed
            (new JobFailedEvent( name()
                               , subscriber
                               , pJob->id()
                               , pJob->result()
                               , fhg::error::UNASSIGNED_ERROR
                               , "TODO: take the error message from the job pointer somehow"
                               )
            );
          sendEventToMaster(pEvtJobFailed);
        }
        break;

      case sdpa::status::CANCELED:
        {
          CancelJobAckEvent::Ptr pEvtCancelJobAck( new CancelJobAckEvent( name(), subscriber, pJob->id() ));
          sendEventToMaster(pEvtCancelJobAck);
        }
        break;
      }
	  }
	  catch(JobNotFoundException const &)
	  {
		  std::string strErr("The job ");
		  strErr+=jobId.str();
		  strErr+=" could not be found!";

		  DMLOG (ERROR, strErr);
		  sendEventToMaster( ErrorEvent::Ptr( new ErrorEvent( name()
				  	  	  	  	  	  	  	  	  	  	  	  , subscriber
				  	  	  	  	  	  	  	  	  	  	  	  , ErrorEvent::SDPA_EJOBNOTFOUND
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

Worker::worker_id_t GenericDaemon::getWorkerId(unsigned int r)
{
  return scheduler()->getWorkerId(r);
}
