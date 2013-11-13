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

#include <sdpa/daemon/JobFSM.hpp>
#include <seda/StageRegistry.hpp>
#include <sdpa/events/CodecStrategy.hpp>
#include <seda/EventPrioQueue.hpp>

#include <sdpa/daemon/GenericDaemon.hpp>
#include <sdpa/daemon/JobImpl.hpp>
#include <sdpa/events/CapabilitiesGainedEvent.hpp>
#include <sdpa/events/CapabilitiesLostEvent.hpp>

#include <sdpa/com/NetworkStrategy.hpp>
#include <sdpa/id_generator.hpp>
#include <sdpa/daemon/exceptions.hpp>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
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
                              unsigned int cap,
                              unsigned int rank,
                              std::string const &guiUrl)
  : Strategy(name),
    SDPA_INIT_LOGGER(name),
    m_arrMasterInfo(arrMasterInfo),
    m_to_master_stage_name_(name+".net"),
    m_to_slave_stage_name_ (name+".net"),

    ptr_job_man_(new JobManager(name)),
    //ptr_scheduler_(),
    ptr_workflow_engine_(NULL),
    m_nRank(rank),
    m_nCap(cap),
    m_strAgentUID(id_generator<agent_id_tag>::instance().next()),
    m_nExternalJobs(0),
    m_ullPollingInterval(100000),
    m_bRequestsAllowed(false),
    m_bStopped(false),
    m_threadBkpService(this),
    m_last_request_time(0),
    m_guiService ("GSPC", guiUrl)
{
  // ask kvs if there is already an entry for (name.id = m_strAgentUID)
  //     e.g. kvs::get ("sdpa.daemon.<name>")
  //          if exists: throw
  //          else:
  //             (fhg::com::)kvs::put ("sdpa.daemon.<name>.id", m_strAgentUID)
  //             kvs::put ("sdpa.daemon.<name>.pid", getpid())
  //                - remove them in destructor

  //daemon_cfg_ = new sdpa::util::Config;

  // application gui service
  if (not guiUrl.empty())
  {
    try
    {
      m_guiService.open ();
      DMLOG (TRACE, "Application GUI service at " << guiUrl << " attached...");
    }
    catch (std::exception const &ex)
    {
      MLOG (WARN, "could not attach GUI at " << guiUrl << ": " << ex.what ());
    }
  }
}

void GenericDaemon::start_agent( bool bUseReqModel, const bfs::path& bkpFile)
{
  bfs::ifstream ifs (bkpFile);
  if (!ifs)
  {
    throw std::runtime_error ("backup file does not exist");
  }

  startup_step1 (bUseReqModel);

  recover (ifs);

  startup_step2();

  m_threadBkpService.start(bkpFile);

  startup_step3();
}

void GenericDaemon::start_agent( bool bUseReqModel, std::string strBackup)
{
  if (strBackup.empty())
  {
    throw std::runtime_error ("backup string is empty");
  }
  std::stringstream iostr(strBackup);

  startup_step1 (bUseReqModel);

  recover (iostr);

  startup_step2();

  m_threadBkpService.start();

  startup_step3();
}

void GenericDaemon::start_agent(bool bUseReqModel)
{
  startup_step1 (bUseReqModel);

  startup_step2();

  m_threadBkpService.start();

  startup_step3();
}

void GenericDaemon::startup_step1 (bool bUseReqModel)
{
  if(!scheduler())
  {
    createScheduler(bUseReqModel);
  }
}
void GenericDaemon::startup_step2()
{
  ptr_daemon_stage_.lock()->start();

  try
  {
    action_configure();
  }
  catch (...)
  {
    perform_ConfigNokEvent();

    m_bStopped = true;

    throw;
  }

  DMLOG (TRACE, "Starting the scheduler...");
  scheduler()->start(this);

  // start the network stage
  to_master_stage()->start();

  perform_ConfigOkEvent();

  m_bRequestsAllowed = true;
}
void GenericDaemon::startup_step3()
{
  if (!isTop())
  {
    requestRegistration();
  }

  scheduler()->cancelWorkerJobs();
  eworknotreg();

  reScheduleAllMasterJobs();
}

void GenericDaemon::eworknotreg()
{
  sdpa::worker_id_list_t workerList;
  scheduler()->getWorkerList (workerList);

  if (workerList.empty())
  {
    DMLOG (TRACE, "The worker list is empty. No worker to be notified exist!");
    return;
  }

  BOOST_FOREACH (const worker_id_t& workerId, workerList)
  {
    SDPA_LOG_INFO("Send notification to the worker "<<workerId);

    ErrorEvent::Ptr const pErrEvt
      (new ErrorEvent ( name()
                      , ""
                      , ErrorEvent::SDPA_EWORKERNOTREG
                      ,  "worker notification"
                      )
      );

    sendEventToMaster (pErrEvt);
  }

  // remove workers
  scheduler()->removeWorkers();
}

std::string GenericDaemon::last_backup() const
{
	return m_threadBkpService.getLastBackup();
}
/**
 * Shutdown an agent
 */
void GenericDaemon::shutdown( )
{
  DMLOG (TRACE, "Shutting down the component "<<name()<<" ...");
  if (!m_bStopped)
  {
    BOOST_FOREACH(sdpa::MasterInfo & masterInfo, m_arrMasterInfo )
    {
      if( !masterInfo.name().empty() && masterInfo.is_registered() )
        sendEventToMaster (ErrorEvent::Ptr(new ErrorEvent(name(), masterInfo.name(), ErrorEvent::SDPA_ENODE_SHUTDOWN, "node shutdown")));
    }

    DMLOG (TRACE, "Stopping the network stage "<<m_to_master_stage_name_);
    seda::StageRegistry::instance().lookup(m_to_master_stage_name_)->stop();
    seda::StageRegistry::instance().remove(m_to_master_stage_name_);

    scheduler()->stop();
    m_threadBkpService.stop();

    //! \todo OLD COMMENT, STILL VALID? TODO?
    // save the current state of the system .i.e serialize the daemon's state

    m_bRequestsAllowed = false;
    m_bStopped 	= true;

    handleInterruptEvent();

    seda::StageRegistry::instance().lookup(name())->stop();
    seda::StageRegistry::instance().remove(name());

    delete ptr_workflow_engine_;
    ptr_workflow_engine_ = NULL;
  }

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


//actions
void GenericDaemon::action_configure()
{
  // use for now as below, later read from config file
  // TODO: move this to "property" style:
  //    dot separated
  //    hierarchies / categories
  //    retrieve values maybe from kvs?
  //    no spaces

  // Read these values from a configuration file !
  // if this does not exist, use default values

  // set default configuration
  // id StartUpEvent contains a configuration file, read the config file and
  // overwrite the default vaules

  cfg().put("polling interval",             1 * 1000 * 1000);
  cfg().put("upper bound polling interval", 2 * 1000 * 1000 ); // 2s
  cfg().put("registration_timeout",         1 * 1000 * 1000);  // 1s
  cfg().put("backup_interval",              5 * 1000 * 1000);  // 5s

  m_ullPollingInterval = cfg().get<sdpa::util::time_type>("polling interval");
  m_threadBkpService.setBackupInterval( cfg().get<sdpa::util::time_type>("backup_interval") );

  DMLOG (TRACE, "Try to configure the network now ... ");

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

  ptr_to_master_stage_ = ptr_to_slave_stage_ = network_stage;
}

void GenericDaemon::action_delete_job(const DeleteJobEvent& e )
{
  DMLOG (TRACE, e.from() << " requesting to delete job " << e.job_id() );

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
  catch(JobNotMarkedException const &)
  {
    DMLOG (WARN, "Job " << e.job_id() << " not ready for deletion!");
    sendEventToMaster( ErrorEvent::Ptr( new ErrorEvent( name()
                                                        , e.from()
                                                        , ErrorEvent::SDPA_EAGAIN
                                                        , "not ready for deletion, try again later"
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
  catch(std::exception const & ex)
  {
    DMLOG (WARN, "unexpected exception during job-deletion: " << ex.what());
    sendEventToMaster( ErrorEvent::Ptr( new ErrorEvent( name()
                                                        , e.from()
                                                        , ErrorEvent::SDPA_EUNKNOWN
                                                        , ex.what()
                                                        )
                                        )
               );

    throw;
  }
  catch(...)
  {
    sendEventToMaster( ErrorEvent::Ptr( new ErrorEvent( name()
                                                      , e.from()
                                                      , ErrorEvent::SDPA_EUNKNOWN
                                                      , "unknown"
                                                      )
                                      )
               );

    DMLOG (ERROR, "unexpected exception during job-deletion!");
    throw;
  }
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

    // if everything was fine up to here, mark the job as submitted
    Worker::ptr_t pWorker(findWorker(worker_id));
    pWorker->submit(jobId);
  }
  catch(const NoJobScheduledException&)
  {
  }
  catch(const WorkerNotFoundException&)
  {
    DMLOG (TRACE, "The worker " << worker_id << " is not registered! Sending him a notification ...");

    // the worker should register first, before posting a job request
    ErrorEvent::Ptr pErrorEvt(new ErrorEvent(name(), worker_id, ErrorEvent::SDPA_EWORKERNOTREG, "not registered") );
    sendEventToSlave(pErrorEvt);
  }
  catch(const QueueFull&)
  {
    DMLOG (WARN, "Could not send event to internal stage: " << ptr_to_slave_stage_->name() << ": queue is full!");
  }
  catch(const seda::StageNotFound&)
  {
    DMLOG (WARN, "Could not lookup stage: " << ptr_to_slave_stage_->name());
  }
  catch(const std::exception &ex)
  {
    DMLOG (WARN, "Error during request-job handling: " << ex.what());
  }
  catch(...)
  {
    DMLOG (WARN, "Unknown error during request-job handling!");
  }
}


void GenericDaemon::serveJob(const sdpa::worker_id_list_t& worker_list, const job_id_t& jobId)
{
  //take a job from the workers' queue and serve it

  try {
    const Job::ptr_t& ptrJob = jobManager()->findJob(jobId);

    DMLOG(TRACE, "Serving a job to the worker "<<worker_id);

    // create a SubmitJobEvent for the job job_id serialize and attach description
    LOG(TRACE, "The job "<<ptrJob->id()<<" was assigned the following workers:"<<worker_list);

    BOOST_FOREACH(const worker_id_t& worker_id, worker_list)
    {
      SubmitJobEvent::Ptr pSubmitEvt(new SubmitJobEvent(name(), worker_id, ptrJob->id(),  ptrJob->description(), "", worker_list));

      // Post a SubmitJobEvent to the slave who made the request
      sendEventToSlave(pSubmitEvt);

      // if everything was fine up to here, mark the job as submitted
      try {
          Worker::ptr_t pWorker(findWorker(worker_id));
          pWorker->submit(ptrJob->id());
      }
      catch(const WorkerNotFoundException&) {
          DMLOG (TRACE, "The worker " << worker_id << " is not registered! Sending a notification ...");

          // the worker should register first, before posting a job request
          ErrorEvent::Ptr pErrorEvt(new ErrorEvent(name(), worker_id, ErrorEvent::SDPA_EWORKERNOTREG, "not registered") );
          sendEventToSlave(pErrorEvt);
      }
    }
  }
  catch(const NoJobScheduledException&)
  {
  }
  catch(const QueueFull&)
  {
    DMLOG (WARN, "Could not send event to internal stage: " << ptr_to_slave_stage_->name() << ": queue is full!");
  }
  catch(const seda::StageNotFound&)
  {
    DMLOG (WARN, "Could not lookup stage: " << ptr_to_slave_stage_->name());
  }
  catch(const std::exception &ex)
  {
    DMLOG (WARN, "Error during request-job handling: " << ex.what());
  }
  catch(...)
  {
    DMLOG (WARN, "Unknown error during request-job handling!");
  }
}

void GenericDaemon::action_request_job(const RequestJobEvent& e)
{
  /*
  the slave(aggregator) requests new executable jobs
  this message is sent in regular frequencies depending on the load of the slave(aggregator)
  this message can be seen as the trigger for a submitJob
  it contains the id of the last job that has been received
  the orchestrator answers to this message with a submitJob
  */

  // ATTENTION: you should submit/schedule only jobs that are in Pending state
  // A job received from the user should be automatically put into the Running state
  // after submitting the corresponding workflow to WFE

  //take a job from the workers' queue? and serve it

  //To do: replace this with schedule
  /*Worker::worker_id_t worker_id = e.from();
  try {
	  sdpa::job_id_t jobId = scheduler()->assignNewJob(worker_id,  e.last_job_id());
	  serveJob( worker_id, jobId );
  }
  catch(const NoJobScheduledException&)
  {
	  DMLOG (TRACE, "There is no job to be served to the worker "<<worker_id);
  }
  catch(const WorkerNotFoundException&)
  {
     DMLOG (TRACE, "The worker " << worker_id << " is not registered! Sending him a notification ...");

     // the worker should register first, before posting a job request
     ErrorEvent::Ptr pErrorEvt(new ErrorEvent(name(), worker_id, ErrorEvent::SDPA_EWORKERNOTREG, "not registered") );
     sendEventToSlave(pErrorEvt);
  }*/
}

bool hasName(const sdpa::MasterInfo& masterInfo, const std::string& name)
{
  return masterInfo.name() == name;
}

void GenericDaemon::action_submit_job(const SubmitJobEvent& e)
{
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

  //if my capacity is reached, refuse to take any external job until at least one of my
  //assigned jobs completes
  if( e.from() != sdpa::daemon::WE  && jobManager()->countMasterJobs() > capacity() )
  {
    //generate a reject event
    DMLOG (WARN, "Capacity exceeded! Cannot accept further jobs. Reject the job "<<e.job_id().str());
    //send job rejected error event back to the master
    ErrorEvent::Ptr pErrorEvt(new ErrorEvent( name(),
                                              e.from(),
                                              ErrorEvent::SDPA_EJOBREJECTED,
                                              "Capacity exceeded! Cannot take further jobs",
                                              e.job_id()) );
    sendEventToMaster(pErrorEvt);

    return;
  }

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
    JobFSM* ptrFSM = new JobFSM(job_id, e.description(), this, e.parent_id());
    ptrFSM->start_fsm();
    Job::ptr_t pJob(ptrFSM);
    pJob->set_owner(e.from());

    // the job job_id is in the Pending state now!
    jobManager()->addJob(job_id, pJob);

    // check if the message comes from outside/slave or from WFE
    // if it comes from outside set it as local
    if( e.from() != sdpa::daemon::WE && hasWorkflowEngine() )
    {
      DMLOG (TRACE, "got new job from " << e.from() << " = " << job_id);
      pJob->setType(Job::MASTER);

      {
        std::list<std::string> workers; workers.push_back (name());
        const we::mgmt::type::activity_t act (pJob->description());
        const sdpa::daemon::NotificationEvent evt
          ( workers
          , pJob->id().str()
          , NotificationEvent::STATE_STARTED
          , act
          );

        gui_service()->notify (evt);
      }
    }

    schedule(job_id);

    if( e.from() != sdpa::daemon::WE )
    {
      // send back to the user a SubmitJobAckEvent
      SubmitJobAckEvent::Ptr pSubmitJobAckEvt(new SubmitJobAckEvent(name(), e.from(), job_id, e.id()));

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
  catch(QueueFull const &)
  {
    DMLOG (WARN, "Failed to send to the master output stage "<<ptr_to_master_stage_->name()<<" a SubmitJobAckEvt for the job "<<job_id);
  }
  catch(seda::StageNotFound const &)
  {
    DMLOG (WARN, "Stage not found when trying to submit SubmitJobAckEvt for the job "<<job_id);
    throw;
  }
  catch(std::exception const & ex)
  {
    DMLOG (WARN, "Unexpected exception occured when calling 'action_submit_job' for the job "<<job_id<<": " << ex.what());
    throw;
  }
  catch(...)
  {
    DMLOG (WARN, "Unexpected exception occured when calling 'action_submit_job' for the job "<<job_id<<"!");
    throw;
  }
}

void GenericDaemon::action_register_worker(const WorkerRegistrationEvent& evtRegWorker)
{
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
    	  DMLOG (TRACE, "Reschedule the jobs of the worker "<<worker_id<<" and, afterwards, delete it!" );
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
        (new WorkerRegistrationAckEvent ( name()
                                        , evtRegWorker.from()
                                        , evtRegWorker.id()
                                        )
        );

      sendEventToSlave(pWorkerRegAckEvt);
    }
  }
  catch(const QueueFull& ex)
  {
    DMLOG (WARN, "could not send WorkerRegistrationAck: queue is full, this should never happen!"<<ex.what());
    throw;
  }
  catch(const seda::StageNotFound& snf)
  {
    MLOG (FATAL, "could not send WorkerRegistrationAck: locate slave-stage failed: " << snf.what());
    throw;
  }
}

void GenericDaemon::action_error_event(const sdpa::events::ErrorEvent &error)
{
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

            if(masterInfo.getConsecRegAttempts()< cfg().get<unsigned int>("max_consecutive_reg_attempts", 360) )
            {
              const unsigned long reg_timeout(cfg().get<unsigned long>("registration_timeout", 10 *1000*1000) );
              DMLOG (TRACE, "Wait " << reg_timeout/1000000 << "s before trying to re-register ...");
              boost::this_thread::sleep(boost::posix_time::microseconds(reg_timeout));
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

              if( masterInfo.getConsecNetFailCnt() < cfg().get<unsigned long>("max_consecutive_net_faults", 360) )
              {
                const unsigned long reg_timeout(cfg().get<unsigned long>("registration_timeout", 10 *1000*1000) );
                boost::this_thread::sleep(boost::posix_time::seconds(reg_timeout/1000000));

                masterInfo.set_registered(false);
              }
              else
                listDeadMasters.push_back( masterInfo.name() );
            }
          }
        }

        removeMasters(listDeadMasters);
      }
      catch (std::exception const& ex)
      {
        LOG(ERROR, "STRANGE! something went wrong during worker-lookup (" << error.from() << "): " << ex.what ());
      }
      break;
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
      catch(std::exception const &ex)
      {
        DMLOG (WARN, "Unexpected exception occurred upon receiving ErrorEvent::SDPA_EJOBEXISTS: " << ex.what());
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

  try {
    DMLOG(TRACE, "workflow engine submitted "<<activityId);

    job_id_t job_id(activityId);
    job_id_t parent_id(user_data.get_user_job_identification());

    jobManager()->addJobRequirements(job_id, jobReqs);

    // WORK HERE: limit number of maximum parallel jobs
    jobManager()->waitForFreeSlot ();

    // don't forget to set here the job's preferences
    SubmitJobEvent::Ptr pEvtSubmitJob( new SubmitJobEvent( sdpa::daemon::WE, name(), job_id, desc, parent_id) );
    sendEventToSelf(pEvtSubmitJob);
  }
  catch(QueueFull const &)
  {
    DMLOG (WARN, "could not send event to my stage: queue is full!");
    workflowEngine()->failed( activityId
                            , desc
                            , fhg::error::UNEXPECTED_ERROR
                            , "internal queue had an overflow"
                            );
  }
  catch(seda::StageNotFound const &)
  {
    DMLOG (WARN, "Stage not found when trying to deliver"
              << " SubmitJobEvent!"
          );
    workflowEngine()->failed( activityId
                            , desc
                            , fhg::error::UNEXPECTED_ERROR
                            , "internal stage could not be found"
                            );
    throw;
  }
  catch(std::exception const & ex)
  {
    DMLOG (WARN, "unexpected exception during submitJob: " << ex.what());
    workflowEngine()->failed( activityId
                            , desc
                            , fhg::error::UNEXPECTED_ERROR
                            , ex.what()
                            );
    throw;
  }
  catch(...)
  {
    DMLOG (WARN, "unexpected exception during submitJob!");
    workflowEngine()->failed( activityId
                            , desc
                            , fhg::error::UNEXPECTED_ERROR
                            , "something very strange happened"
                            );
    throw;
  }
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
bool GenericDaemon::cancelled(const id_type& workflowId)
{
  DMLOG (TRACE, "activity cancelled: " << workflowId);
  // generate a JobCancelledEvent for self!

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

void GenericDaemon::submitWorkflow(const id_type& wf_id, const encoded_type& desc )
{
  if(!hasWorkflowEngine())
    throw NoWorkflowEngine();

  workflowEngine()->submit (wf_id, desc, we::type::user_data ());
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

  addWorker( worker_id, evtRegWorker.capacity(), workerCpbSet, evtRegWorker.rank(), evtRegWorker.agent_uuid() );

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
  catch( const std::exception& ex)
  {
    DMLOG (WARN, "Unexpected exception ("<<ex.what()<<") occurred when trying to add new capabilities to the worker "<<worker_id);
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
  catch( const std::exception& ex)
  {
    DMLOG (WARN, "Unexpected exception ("<<ex.what()<<") occurred when trying to remove some capabilities of the worker "<<worker_id);
  }
}

void GenericDaemon::handleSubscribeEvent( const sdpa::events::SubscribeEvent* pEvt )
{
  DMLOG(TRACE, "Received subscribe event!");
  try {
    subscribe(pEvt->subscriber(), pEvt->listJobIds());
  }
  catch(const std::exception& exc)
  {
    DMLOG (WARN, "An exception occurred when "<<pEvt->subscriber()<<" attempted to subscribe: "<<exc.what());
  }
}

void GenericDaemon::sendEventToSelf(const SDPAEvent::Ptr& pEvt)
{
  try {
    if(ptr_daemon_stage_.lock())
    {
      ptr_daemon_stage_.lock()->send(pEvt);
      DLOG(TRACE, "Sent " <<pEvt->str()<<" to "<<pEvt->to());
    }
    else
    {
      DMLOG (ERROR, "Daemon stage not defined! ");
    }
  }
  catch(const seda::QueueFull&)
  {
    DMLOG (WARN, "Could not send event. The queue is full!");
  }
  catch(const seda::StageNotFound& ex)
  {
    DMLOG (ERROR, "Stage not found! "<<ex.what());
  }
  catch(const std::exception& ex)
  {
    DMLOG (WARN, "Could not send event. Exception occurred: "<<ex.what());
  }
}

void GenericDaemon::sendEventToMaster(const sdpa::events::SDPAEvent::Ptr& pEvt)
{
  try {
    if( to_master_stage().get() )
    {
      to_master_stage()->send(pEvt);
      DLOG(TRACE, "Sent " <<pEvt->str()<<" to "<<pEvt->to());
      // to_master_stage()->dump();
    }
    else
    {
      DMLOG (ERROR, "The master stage does not exist!");
    }
  }
  catch(const QueueFull&)
  {
    DMLOG (WARN, "Could not send event. The queue is full!");
  }
  catch(const seda::StageNotFound& )
  {
    DMLOG (ERROR, "Stage "<<to_master_stage()->name()<<" not found!");
  }
  catch(const std::exception& ex)
  {
    DMLOG (WARN, "Could not send event. Exception occurred: "<<ex.what());
  }
}

void GenericDaemon::sendEventToSlave(const sdpa::events::SDPAEvent::Ptr& pEvt)
{
  try {
    if( to_slave_stage().get() )
    {
       to_slave_stage()->send(pEvt);
      DLOG(TRACE, "Sent " <<pEvt->str()<<" to "<<pEvt->to());
    }
    else
    {
      DMLOG (ERROR, "The slave stage does not exist!");
    }
  }
  catch(const QueueFull&)
  {
    DMLOG (WARN, "Could not send event. The queue is full!");
  }
  catch(const seda::StageNotFound& )
  {
    DMLOG (ERROR, "Stage "<<to_slave_stage()->name()<<" not found!");
  }
  catch(const std::exception& ex)
  {
    DMLOG (WARN, "Could not send event. Exception occurred: "<<ex.what());
  }
}

Worker::ptr_t const & GenericDaemon::findWorker(const Worker::worker_id_t& worker_id ) const
{
  return scheduler()->findWorker(worker_id);
}

const Worker::worker_id_t& GenericDaemon::findWorker(const sdpa::job_id_t& job_id) const
{
  return scheduler()->findWorker(job_id);
}

void GenericDaemon::addWorker(  const Worker::worker_id_t& workerId,
                                unsigned int cap,
                                const capabilities_set_t& cpbset,
                                const unsigned int& agent_rank,
                                const sdpa::worker_id_t& agent_uuid )
{
  scheduler()->addWorker(workerId, cap, cpbset, agent_rank, agent_uuid);
}

void GenericDaemon::updateLastRequestTime()
{
    sdpa::util::time_type current_time = sdpa::util::now();
    sdpa::util::time_type difftime = current_time - m_last_request_time;
    m_last_request_time = difftime;
}

bool GenericDaemon::requestsAllowed()
{
  // if m_nExternalJobs is null then slow it down, i.e. increase m_ullPollingInterval
  // reset it to the value specified by config first time when m_nExternalJobs becomes positive
  // don't forget to decrement m_nExternalJobs when the job is finished !

  if(!m_bRequestsAllowed)
  {
    DMLOG (TRACE, "The flag \"m_bRequestsAllowed\" is set on false!");
    return false;
  }

  if( jobManager()->countMasterJobs() == 0 )
    if( m_ullPollingInterval < cfg().get<unsigned int>("upper bound polling interval") )
       m_ullPollingInterval = m_ullPollingInterval + 100 * 1000; //0.1s

  sdpa::util::time_type current_time = sdpa::util::now();
  sdpa::util::time_type diff_time    = current_time - m_last_request_time;

  return ( diff_time > m_ullPollingInterval ) && ( jobManager()->countMasterJobs() < cfg().get<unsigned int>("nmax_ext_job_req"));
}

void GenericDaemon::activityFailed( const Worker::worker_id_t& worker_id
                                  , const job_id_t& jobId
                                  , const std::string& result
                                  , const int error_code
                                  , const std::string& reason
                                  )
{
  if( hasWorkflowEngine() )
  {
    MLOG( WARN
        , "job " << jobId
        << " executed on " << worker_id
        << " failed:" << " error := " << fhg::error::show(error_code)
        << " (" << error_code << ")"
        << " message := " << reason
        );

    workflowEngine()->failed( jobId.str()
                            , result
                            , error_code
                            , reason
                            );

    try {
      jobManager()->deleteJob(jobId);
    }
    catch(const JobNotDeletedException& ex)
    {
      DMLOG (WARN, "Could not find the job "<<jobId.str()<<" ...");
    }
  }
  else
  {
    DLOG(TRACE, "Send JobFailedEvent to self for the job"<<jobId);
    JobFailedEvent::Ptr pEvtJobFailed
      (new JobFailedEvent ( worker_id
                          , name()
                          , jobId
                          , reason
                          , error_code
                          , reason
                          )
      );

    sendEventToSelf(pEvtJobFailed);
  }
}

void GenericDaemon::activityFinished(const Worker::worker_id_t& worker_id, const job_id_t& jobId, const result_type & result)
{
  if( hasWorkflowEngine() )
  {
    DMLOG (TRACE, "worker job finished: " << jobId);
    workflowEngine()->finished( jobId.str(), result );
    try {
      jobManager()->deleteJob(jobId);
    }
    catch(const JobNotDeletedException& ex)
    {
      DMLOG (WARN, "Could not find the job "<<jobId.str()<<" ...");
    }
  }
  else
  {
    DMLOG (TRACE, "Sent self a jobFinishedEvent for the job"<<jobId);
    JobFinishedEvent::Ptr pEvtJobFinished(new JobFinishedEvent(worker_id, name(), jobId, result));
    sendEventToSelf(pEvtJobFinished);
  }
}

void GenericDaemon::activityCancelled(const Worker::worker_id_t& worker_id, const job_id_t& jobId)
{
  if( hasWorkflowEngine() )
  {
    DMLOG (TRACE, "worker job cancelled: " << jobId);
    workflowEngine()->cancelled( jobId.str() );
    try {
      jobManager()->deleteJob(jobId);
    }
    catch(const JobNotDeletedException& ex)
    {
      DMLOG (WARN, "Could not find the job "<<jobId.str()<<" ...");
    }
  }
  else
  {
    DLOG(TRACE, "Sent CancelJobAckEvent to self for the job"<<jobId);
    CancelJobAckEvent::Ptr pEvtCancelJobAck(new CancelJobAckEvent(worker_id, name(), jobId));
    sendEventToSelf(pEvtCancelJobAck);
  }
}

void GenericDaemon::jobStalled(const job_id_t& id )
{
  // generate an event of type stalled for the job state machine and trigger it
  // on the action_job_stalled ->
  // if it's not a master job invoke trigger the same event for its state machine
  // else, if it's a master job notify the master that the job stalled
}

void GenericDaemon::requestJob(const MasterInfo& masterInfo)
{
  if( masterInfo.is_registered() )
  {
    RequestJobEvent::Ptr pEvtReq( new RequestJobEvent( name(), masterInfo.name() ) );
    sendEventToMaster(pEvtReq);
  }

  updateLastRequestTime();
}

void GenericDaemon::requestRegistration(const MasterInfo& masterInfo)
{
  if( !masterInfo.is_registered() )
  {
    DMLOG (TRACE, "The agent \"" << name()
               << "\" is sending a registration event to master \"" << masterInfo.name()
               << "\", capacity = "<<capacity()
          );

    capabilities_set_t cpbSet;
    getCapabilities(cpbSet);

    //std::cout<<cpbSet;

    WorkerRegistrationEvent::Ptr pEvtWorkerReg(new WorkerRegistrationEvent( name(), masterInfo.name(), capacity(), cpbSet,  rank(), agent_uuid()));
    sendEventToMaster(pEvtWorkerReg);
  }
}

void GenericDaemon::requestRegistration()
{
  // try to re-register
  lock_type lock(mtx_master_);
  BOOST_FOREACH(sdpa::MasterInfo& masterInfo, m_arrMasterInfo)
  {
    requestRegistration(masterInfo);
  }
}

void GenericDaemon::schedule(const sdpa::job_id_t& jobId)
{
  if( scheduler() )
  {
    scheduler()->schedule(jobId);
    return;
  }

  throw std::runtime_error(name() + " does not have scheduler!");
}

void GenericDaemon::reschedule(const sdpa::job_id_t& jobId)
{
  if( scheduler() )
  {
    scheduler()->rescheduleJob(jobId);
    return;
  }

  throw std::runtime_error(name() + " does not have scheduler!");
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
		  sdpa::status_t jobStatus = pJob->getStatus();

		  if(jobStatus.find("Finished") != std::string::npos)
		  {
			  JobFinishedEvent::Ptr pEvtJobFinished
			  	  (new JobFinishedEvent( name()
					  	  	  	  	  , subscriber
					  	  	  	  	  , pJob->id()
					  	  	  	  	  , pJob->result()
							));
			  sendEventToMaster(pEvtJobFinished);
		}
		else if(jobStatus.find("Failed") != std::string::npos)
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
		else if( jobStatus.find("Cancelled") != std::string::npos)
		{
			CancelJobAckEvent::Ptr pEvtCancelJobAck( new CancelJobAckEvent( name(), subscriber, pJob->id() ));
			sendEventToMaster(pEvtCancelJobAck);
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

void GenericDaemon::reScheduleAllMasterJobs()
{
  //jobManager()->reScheduleAllMasterJobs(this);

  sdpa::job_id_list_t listNotCompletedMsterJobs = jobManager()->getListNotCompletedMasterJobs(hasWorkflowEngine());

  BOOST_FOREACH(const job_id_t& jobId, listNotCompletedMsterJobs)
  {
    DMLOG (TRACE, "Re-schedule the job"<<jobId);
    reschedule(jobId);
  }
}

void GenericDaemon::backup( std::ostream& ofs )
{
	try {
		//std::string strArchiveName(name()+".bkp");
		//SDPA_LOG_DEBUG("Backup the agent "<<name()<<" to file "<<strArchiveName);

		boost::archive::text_oarchive oa(ofs);
		oa.register_type(static_cast<JobManager*>(NULL));
		oa.register_type(static_cast<JobImpl*>(NULL));
		oa.register_type(static_cast<JobFSM*>(NULL));
		backupJobManager(oa);

		//oa.register_type(static_cast<SchedulerImpl*>(NULL));
		backupScheduler(oa);

		/*oa.register_type(static_cast<T*>(NULL));
    	oa << ptr_workflow_engine_;*/
		oa << boost::serialization::make_nvp("url_", m_arrMasterInfo);
	}
	catch(exception &e) {
		cout <<"Exception occurred: "<< e.what() << endl;
	}
}

void GenericDaemon::recover( std::istream& ifs )
{
	try {
		boost::archive::text_iarchive ia(ifs);
		ia.register_type(static_cast<JobManager*>(NULL));
		ia.register_type(static_cast<JobImpl*>(NULL));
		ia.register_type(static_cast<JobFSM*>(NULL));
		recoverJobManager(ia);

		//ia.register_type(static_cast<SchedulerImpl*>(NULL));
		recoverScheduler(ia);

		// should ignore the workflow engine,
		// since it is not always possible to recover it

		/*ia.register_type(static_cast<T*>(NULL));
    	ia >> ptr_workflow_engine_;*/
		ia >> boost::serialization::make_nvp("url_", m_arrMasterInfo);
		SDPA_LOG_INFO("The list of recoverd masters is: ");
	}
	catch(exception &e) {
		cout <<"Exception occurred: " << e.what() << endl;
	}
}
