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

#include <sdpa/daemon/jobFSM/JobFSM.hpp>
#include <seda/StageRegistry.hpp>
#include <sdpa/events/CodecStrategy.hpp>
#include <seda/EventPrioQueue.hpp>

#include <sdpa/daemon/GenericDaemon.hpp>
#include <sdpa/daemon/JobImpl.hpp>
#include <sdpa/events/ConfigReplyEvent.hpp>
#include <sdpa/events/StartUpEvent.hpp>
#include <sdpa/events/ConfigOkEvent.hpp>
#include <sdpa/events/CapabilitiesGainedEvent.hpp>
#include <sdpa/events/CapabilitiesLostEvent.hpp>

#include <sdpa/com/NetworkStrategy.hpp>
#include <sdpa/events/id_generator.hpp>
#include <sdpa/daemon/exceptions.hpp>

#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/tokenizer.hpp>
#include <boost/foreach.hpp>
#include <sstream>
#include <algorithm>

using namespace std;
using namespace sdpa::daemon;
using namespace sdpa::events;

//constructor
GenericDaemon::GenericDaemon( const std::string name,
                              const master_info_list_t arrMasterInfo,
                              unsigned int cap,
                              unsigned int rank )
  : Strategy(name),
    SDPA_INIT_LOGGER(name),
    m_arrMasterInfo(arrMasterInfo),
    m_to_master_stage_name_(name+".net"),
    m_to_slave_stage_name_ (name+".net"),

    ptr_job_man_(new JobManager(name)),
    ptr_scheduler_(),
    ptr_workflow_engine_(NULL),

    m_nRank(rank),
    m_nCap(cap),
    m_strAgentUID(id_generator::instance().next()),
    m_nExternalJobs(0),
    m_ullPollingInterval(100000),
    m_bRequestsAllowed(false),
    m_bStarted(false),
    m_bConfigOk(false),
    m_bStopped(false),
    m_threadBkpService(this),
    m_last_request_time(0)
{
  // ask kvs if there is already an entry for (name.id = m_strAgentUID)
  //     e.g. kvs::get ("sdpa.daemon.<name>")
  //          if exists: throw
  //          else:
  //             (fhg::com::)kvs::put ("sdpa.daemon.<name>.id", m_strAgentUID)
  //             kvs::put ("sdpa.daemon.<name>.pid", getpid())
  //                - remove them in destructor

  //daemon_cfg_ = new sdpa::util::Config;
}

GenericDaemon::~GenericDaemon()
{
  SDPA_LOG_DEBUG("GenericDaemon destructor called ...");
}

/**
 * Start an agent
 * @param[in] bUseReqModel When set on true, the agent uses the request model, otherwise it uses the push model
 * @param[in] bkpFile Backup file for the agent
 * @param[in] cfgFile Configuration file of the agent
 */
void GenericDaemon::start_agent( bool bUseReqModel, const bfs::path& bkpFile, const std::string& cfgFile )
{
  if(!scheduler())
  {
    SDPA_LOG_INFO("Create the scheduler...");
    createScheduler(bUseReqModel);
  }

  bfs::ifstream ifs(bkpFile);
  if( !ifs.fail())
  {
    SDPA_LOG_INFO( "Recover the agent "<<name()<<" from the backup file "<<bkpFile);

    recover(ifs);
    //if( isTop() )
    {
      SDPA_LOG_WARN( "JobManager after recovering:" );
      jobManager()->print();

      SDPA_LOG_INFO("Worker manager after recovering:");
      scheduler()->print();
    }
  }
  else
  {
    SDPA_LOG_WARN( "Can't find the backup file "<<bkpFile);
  }

  scheduler()->setUseRequestModel(bUseReqModel);

  // The stage uses 2 threads
  ptr_daemon_stage_.lock()->start();

  //start-up the the daemon
  SDPA_LOG_INFO("Trigger StartUpEvent...");
  StartUpEvent::Ptr pEvtStartUp(new StartUpEvent(name(), name(), cfgFile));
  sendEventToSelf(pEvtStartUp);

  lock_type lock(mtx_);
  while( !isStarted() )
    cond_can_start_.wait(lock);

  if( isConfigured() )  // can register now
  {
    m_threadBkpService.start(bkpFile);

    SDPA_LOG_INFO("Agent " << name() << " was successfully configured!");
    if( !isTop() )
      requestRegistration();

    SDPA_LOG_INFO("Notify the workers that I'm up again and they should re-register!");

    scheduler()->cancelWorkerJobs();
    ErrorEvent::Ptr pErrEvt(new ErrorEvent( name(), "", ErrorEvent::SDPA_EWORKERNOTREG,  "worker notification") );
    notifyWorkers(pErrEvt);
  }
  else
  {
    SDPA_LOG_INFO("Agent "<<name()<<" could not configure. Giving up now!");
  }

  reScheduleAllMasterJobs();
}

/**
 * Start an agent
 * @param[in] bUseReqModel When set on true, the agent uses the request model, otherwise it uses the push model
 * @param[in] bkpFile Backup string for the agent
 * @param[in] cfgFile Configuration file of the agent
 */
void GenericDaemon::start_agent( bool bUseReqModel, std::string& strBackup, const std::string& cfgFile )
{
  if(!scheduler())
  {
    SDPA_LOG_INFO("Create the scheduler...");
    createScheduler(bUseReqModel);
  }

  if( !strBackup.empty() )
  {
    SDPA_LOG_INFO( "The backup file is not empty! Attempting to recover the daemon "<<name());
    //LOG(INFO, "The recovery string is: "<<strBackup);
    std::stringstream iostr(strBackup);
    recover(iostr);

    //if( isTop() )
    {
      SDPA_LOG_WARN( "JobManager after recovering:" );
      jobManager()->print();

      SDPA_LOG_INFO("Scheduler after recovering:");
      scheduler()->print();
    }
  }
  else
    SDPA_LOG_INFO( "The backup file is empty! No recovery operation carried out for the daemon "<<name());

  scheduler()->setUseRequestModel(bUseReqModel);

  // The stage uses 2 threads
  ptr_daemon_stage_.lock()->start();

  //start-up the the daemon
  SDPA_LOG_INFO("Trigger StartUpEvent...");
  StartUpEvent::Ptr pEvtStartUp(new StartUpEvent(name(), name(), cfgFile));
  sendEventToSelf(pEvtStartUp);

  lock_type lock(mtx_);
  while( !isStarted() )
    cond_can_start_.wait(lock);

  if( isConfigured() )
  {
    m_threadBkpService.start();

    SDPA_LOG_INFO("Agent " << name() << " was successfully configured!");
    if( !isTop() )
      requestRegistration();

    SDPA_LOG_INFO("Notify the workers that I'm up again and they should re-register!");

    scheduler()->cancelWorkerJobs();
    ErrorEvent::Ptr pErrEvt(new ErrorEvent( name(), "", ErrorEvent::SDPA_EWORKERNOTREG,  "worker notification") );
    notifyWorkers(pErrEvt);
  }
  else
  {
    SDPA_LOG_INFO("Agent "<<name()<<" could not configure. Giving up now!");
  }

  reScheduleAllMasterJobs();

}

/**
 * Start an agent
 * @param[in] bUseReqModel: When set on true, the agent uses the request model, otherwise it uses the push model
 * @param[in] cfgFile: Configuration file of the agent
 */
void GenericDaemon::start_agent(bool bUseReqModel, const std::string& cfgFile )
{
  if(!scheduler())
  {
    SDPA_LOG_INFO("Create the scheduler...");
    createScheduler(bUseReqModel);
  }

  // The stage uses 2 threads
  ptr_daemon_stage_.lock()->start();

  //start-up the the daemon
  SDPA_LOG_INFO("Trigger StartUpEvent...");
  StartUpEvent::Ptr pEvtStartUp(new StartUpEvent(name(), name(), cfgFile));
  sendEventToSelf(pEvtStartUp);

  lock_type lock(mtx_);
  while( !isStarted() )
    cond_can_start_.wait(lock);

  if( isConfigured() )
  {
    // no backup, if a backup file was not specified!
    SDPA_LOG_INFO("Agent " << name() << " was successfully configured!");
    if( !isTop() )
      requestRegistration();

    SDPA_LOG_INFO("Notify the workers that I'm up again and they should re-register!");

    scheduler()->cancelWorkerJobs();
    ErrorEvent::Ptr pErrEvt(new ErrorEvent( name(), "", ErrorEvent::SDPA_EWORKERNOTREG,  "worker notification") );
    notifyWorkers(pErrEvt);
  }
  else
  {
    SDPA_LOG_INFO("Agent "<<name()<<" could not configure. Giving up now!");
  }

  reScheduleAllMasterJobs();
}

/**
 * Shutdown an agent
 * @param[in] bUseReqModel When set on true, the agent uses the request model, otherwise it uses the push model
 * @param[in] bkpFile Backup file for the agent
 * @param[in] cfgFile Configuration file of the agent
 */
void GenericDaemon::shutdown(std::string& strBackup )
{
  if( !isStopped() )
    stop();

  SDPA_LOG_INFO("Get the last backup of the daemon "<<name());
  strBackup = m_threadBkpService.getLastBackup();
}

/**
 * Shutdown an agent
 * @param[in] bUseReqModel When set on true, the agent uses the request model, otherwise it uses the push model
 * @param[in] bkpFile Backup file for the agent
 * @param[in] cfgFile Configuration file of the agent
 */
void GenericDaemon::shutdown( )
{
  if( !isStopped() )
    stop();
}

/**
 * Configure the network
 */
void GenericDaemon::configure_network( const std::string& daemonUrl /*, const std::string& masterName*/ )
{
  SDPA_LOG_DEBUG("configuring network components...");

  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;

  boost::char_separator<char> sep(":");
  tokenizer tok(daemonUrl, sep);

  vector< string > vec;
  vec.assign(tok.begin(),tok.end());

  if( vec.empty() || vec.size() > 2 )
  {
    LOG(ERROR, "Invalid daemon url.  Please specify it in the form <hostname (IP)>:<port>!");
    return;
  }
  else
  {
    std::string bind_addr = vec[0];
    std::string bind_port("0");

    if( vec.size() == 2)
    bind_port = vec[1];

    SDPA_LOG_INFO("Host: "<<bind_addr<<", port: "<<bind_port);

    try
    {
      sdpa::com::NetworkStrategy::ptr_t net
        (new sdpa::com::NetworkStrategy( /*daemon_stage_->*/name()
                                        , name()
                                        , fhg::com::host_t (bind_addr)
                                        , fhg::com::port_t (bind_port)
                                        ) );

      int maxQueueSize = 5000;
      seda::IEventQueue::Ptr ptrEvtPrioQueue( new seda::EventPrioQueue("network.stage."+name()+".queue", maxQueueSize) );
      seda::Stage::Ptr network_stage (new seda::Stage(m_to_master_stage_name_, ptrEvtPrioQueue, net, 1));

      // seda::Stage::Ptr network_stage (new seda::Stage( m_to_master_stage_name_
      //                                                , net
      //                                                , 1
      //                                                )
      //                                );
      seda::StageRegistry::instance().insert (network_stage);
      ptr_to_master_stage_ = ptr_to_slave_stage_ = network_stage;
    }
    catch (std::exception const &ex)
    {
      LOG(ERROR, "could not configure network component: " << ex.what());
      throw;
    }
  }
}

/**
 * Shutdown the network
 */
void GenericDaemon::shutdown_network()
{
  BOOST_FOREACH(sdpa::MasterInfo & masterInfo, m_arrMasterInfo )
  {
    if( !masterInfo.name().empty() && masterInfo.is_registered() )
      sendEventToMaster (ErrorEvent::Ptr(new ErrorEvent(name(), masterInfo.name(), ErrorEvent::SDPA_ENODE_SHUTDOWN, "node shutdown")));
  }
}

/**
 * Stop the agent.
 */
void GenericDaemon::stop()
{
  MLOG (INFO, name () << " is shutting down...");

  scheduler()->stop();

  m_threadBkpService.stop();

  shutdown_network();

  seda::StageRegistry::instance().lookup(name())->stop();
  seda::StageRegistry::instance().lookup(m_to_master_stage_name_)->stop();

  seda::StageRegistry::instance().remove(m_to_master_stage_name_);
  seda::StageRegistry::instance().remove(name());

  if( hasWorkflowEngine() )
  {
    delete ptr_workflow_engine_;
    ptr_workflow_engine_ = NULL;
  }

  MLOG (INFO, name () << " was successfully stopped!");
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
    SDPA_LOG_WARN("Received unexpected event " << pEvent->str()<<". Cannot handle it!");
  }
}

void GenericDaemon::setDefaultConfiguration()
{
  cfg().put("polling interval",             1 * 1000 * 1000);
  cfg().put("upper bound polling interval", 2 * 1000 * 1000 ); // 2s
  cfg().put("registration_timeout",         1 * 1000 * 1000); // 1s
  cfg().put("backup_interval",              5 * 1000 * 1000); // 3s*/
}

//actions
void GenericDaemon::action_configure(const StartUpEvent& evt)
{
  SDPA_LOG_INFO("Configuring myself (generic)...");

  // use for now as below, later read from config file
  // TODO: move this to "property" style:
  //    dot separated
  //    hierarchies / categories
  //    retrieve values maybe from kvs?
  //    no spaces

  // Read these values from a configuration file !!!!!!!!
  // if this does not exist, use default values

  // set default configuration
  // id StartUpEvent contains a configuration file, read the config file and
  // overwrite the default vaules

  setDefaultConfiguration();

  if(!evt.cfgFile().empty())
  {
    SDPA_LOG_ERROR("Read the configuration file daemon_config.txt ... ");

    bfs::path cfgPath(evt.cfgFile());

    if(!bfs::exists(cfgPath))
    {
      SDPA_LOG_ERROR("Could not find the configuration file "<<evt.cfgFile()<<"!");
      m_bConfigOk = false;
      return;
    }

    try {
      cfg().read(evt.cfgFile());
    }
    catch (const sdpa::util::InvalidConfiguration& ex )
    {
      SDPA_LOG_ERROR("Error when parsing the ini file. "<<ex.what());
    }
  }
  else
  {
    LOG (DEBUG, "No configuration file was specified. Using the default configuration.");
  }

  m_ullPollingInterval = cfg().get<sdpa::util::time_type>("polling interval");
  m_threadBkpService.setBackupInterval( cfg().get<sdpa::util::time_type>("backup_interval") );

  try {
    SDPA_LOG_DEBUG("Try to configure the network now ... ");
    configure_network( url() /*, masterName()*/ );
    m_bConfigOk = true;
  }
  catch (std::exception const &ex)
  {
    SDPA_LOG_ERROR("Exception occurred while trying to configure the network " << ex.what());
    m_bConfigOk = false;
  }
}

void GenericDaemon::action_config_ok(const ConfigOkEvent&)
{
  // check if the system should be recovered
  // should be overriden by the orchestrator, aggregator and NRE
  SDPA_LOG_DEBUG("The configuration phase succeeded!");
  setRequestsAllowed(true);
}

void GenericDaemon::action_config_nok(const ConfigNokEvent &pEvtCfgNok)
{
  SDPA_LOG_ERROR("the configuration phase failed!");
}

void GenericDaemon::action_interrupt(const InterruptEvent& pEvtInt)
{
  SDPA_LOG_DEBUG("Call 'action_interrupt'");
  // save the current state of the system .i.e serialize the daemon's state
  // the following code shoud be executed on action action_interrupt!!
  setRequestsAllowed(false);
}

void GenericDaemon::action_delete_job(const DeleteJobEvent& e )
{
  LOG( DEBUG, e.from() << " requesting to delete job " << e.job_id() );

  try{
    Job::ptr_t pJob = jobManager()->findJob(e.job_id());
    pJob->DeleteJob(&e, this);

    jobManager()->deleteJob(e.job_id());
  }
  catch(JobNotFoundException const &)
  {
    SDPA_LOG_ERROR("Job " << e.job_id() << " could not be found!");
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
    SDPA_LOG_WARN("Job " << e.job_id() << " not ready for deletion!");
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
    SDPA_LOG_ERROR("Job " << e.job_id() << " could not be deleted!");
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
    SDPA_LOG_ERROR("unexpected exception during job-deletion: " << ex.what());
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

    SDPA_LOG_ERROR("unexpected exception during job-deletion!");
    throw;
  }
}

void GenericDaemon::serveJob(const Worker::worker_id_t& worker_id, const job_id_t& last_job_id)
{
  //take a job from the workers' queue and serve it

  try {
    // you should consume from the  worker's pending list; put the job into the worker's submitted list
    sdpa::job_id_t jobId = scheduler()->getNextJob(worker_id, last_job_id);
    DMLOG(TRACE, "Assign the job "<<jobId<<" to the worker '"<<worker_id);

    const Job::ptr_t& ptrJob = jobManager()->findJob(jobId);

    DMLOG(TRACE, "Serving a job to the worker "<<worker_id);

    // create a SubmitJobEvent for the job job_id serialize and attach description
    DMLOG(TRACE, "sending SubmitJobEvent (jid=" << ptrJob->id() << ") to: " << worker_id);
    SubmitJobEvent::Ptr pSubmitEvt(new SubmitJobEvent(name(), worker_id, ptrJob->id(),  ptrJob->description(), ""));

    // Post a SubmitJobEvent to the slave who made the request
    sendEventToSlave(pSubmitEvt);
    scheduler()->setLastTimeServed(worker_id, sdpa::util::now());
  }
  catch(const NoJobScheduledException&)
  {
    //SDPA_LOG_DEBUG("No job was scheduled on the worker '"<<worker_id);
  }
  catch(const WorkerNotFoundException&)
  {
    SDPA_LOG_INFO("The worker " << worker_id << " is not registered! Sending him a notification ...");

    // the worker should register first, before posting a job request
    ErrorEvent::Ptr pErrorEvt(new ErrorEvent(name(), worker_id, ErrorEvent::SDPA_EWORKERNOTREG, "not registered") );
    sendEventToSlave(pErrorEvt);
  }
  catch(const QueueFull&)
  {
    SDPA_LOG_ERROR("Could not send event to internal stage: " << ptr_to_slave_stage_->name() << ": queue is full!");
  }
  catch(const seda::StageNotFound&)
  {
    SDPA_LOG_ERROR("Could not lookup stage: " << ptr_to_slave_stage_->name());
  }
  catch(const std::exception &ex)
  {
    SDPA_LOG_ERROR("Error during request-job handling: " << ex.what());
  }
  catch(...)
  {
    SDPA_LOG_ERROR("Unknown error during request-job handling!");
  }
}

void GenericDaemon::action_request_job(const RequestJobEvent& e)
{
  //SDPA_LOG_DEBUG("got job request from: " << e.from());

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
  Worker::worker_id_t worker_id = e.from();
  serveJob( worker_id, e.last_job_id() );
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
    SDPA_LOG_INFO("The agent "<<name()<<" is not yet registered with the master "<<itMaster->name()
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
    SDPA_LOG_WARN("Capacity exceeded! Cannot accept further jobs. Reject the job "<<e.job_id().str());
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

    SDPA_LOG_WARN("The job with job-id: " << e.job_id()<<" does already exist! (possibly recovered)");
    if( e.from() != sdpa::daemon::WE ) //e.to())
    {
        ErrorEvent::Ptr pErrorEvt(new ErrorEvent(name(), e.from(), ErrorEvent::SDPA_EJOBEXISTS, "The job already exists!", e.job_id()) );
        sendEventToMaster(pErrorEvt);
    }

    /*SDPA_LOG_DEBUG("Jobs to be scheduled: ");
    scheduler()->printQ();*/

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
      SDPA_LOG_DEBUG("got new job from " << e.from() << " = " << job_id);
      pJob->setType(Job::MASTER);
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
    SDPA_LOG_ERROR("job " << job_id << " could not be added: " << ex.what());
    // the worker should register first, before posting a job request
    ErrorEvent::Ptr pErrorEvt(new ErrorEvent(name(), e.from(), ErrorEvent::SDPA_EUNKNOWN, ex.what()) );
    sendEventToMaster(pErrorEvt);
  }
  catch(QueueFull const &)
  {
    SDPA_LOG_WARN("Failed to send to the master output stage "<<ptr_to_master_stage_->name()<<" a SubmitJobAckEvt for the job "<<job_id);
  }
  catch(seda::StageNotFound const &)
  {
    SDPA_LOG_FATAL("Stage not found when trying to submit SubmitJobAckEvt for the job "<<job_id);
    throw;
  }
  catch(std::exception const & ex)
  {
    SDPA_LOG_ERROR("Unexpected exception occured when calling 'action_submit_job' for the job "<<job_id<<": " << ex.what());
    throw;
  }
  catch(...)
  {
    SDPA_LOG_ERROR("Unexpected exception occured when calling 'action_submit_job' for the job "<<job_id<<"!");
    throw;
  }
}

void GenericDaemon::action_config_request(const ConfigRequestEvent& e)
{
  //SDPA_LOG_DEBUG("got config request from " << e.from());
  /*
  * on startup the aggregator tries to retrieve a configuration from its orchestrator
  * post ConfigReplyEvent/message that contains the configuration data for the requesting aggregator
  * TODO: what is contained into the Configuration?
  */

  ConfigReplyEvent::Ptr pCfgReplyEvt( new ConfigReplyEvent( name(), e.from()) );
  sendEventToSlave(pCfgReplyEvt);
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

      SDPA_LOG_DEBUG( "The worker manager already contains an worker with the same id (="<<ex.worker_id()<<") but with a different agent_uuid!" );

      try {
    	  const Worker::ptr_t& pWorker = findWorker(worker_id);

    	  // mark the worker as disconnected
    	  pWorker->set_disconnected();
    	  SDPA_LOG_DEBUG( "Reschedule the jobs of the worker "<<worker_id<<" and, afterwards, delete it!" );
    	  scheduler()->delWorker(worker_id);
      }
      catch (const WorkerNotFoundException& ex)
      {
        SDPA_LOG_ERROR("New worker find the worker "<<worker_id);
      }

      SDPA_LOG_DEBUG( "Add worker"<<worker_id );
      registerWorker(evtRegWorker);
    }
    else
    {
      SDPA_LOG_INFO("A worker with the same id (" << worker_id << ") and uuid ("<<evtRegWorker.agent_uuid()<<" is already registered!");

      // just answer back with an acknowledgment
      SDPA_LOG_INFO("Send registration ack to the agent " << worker_id );
      WorkerRegistrationAckEvent::Ptr pWorkerRegAckEvt(new WorkerRegistrationAckEvent(name(), evtRegWorker.from()));

      pWorkerRegAckEvt->id() = evtRegWorker.id();
      sendEventToSlave(pWorkerRegAckEvt);
    }
  }
  catch(const QueueFull& ex)
  {
    SDPA_LOG_FATAL("could not send WorkerRegistrationAck: queue is full, this should never happen!"<<ex.what());
    throw;
  }
  catch(const seda::StageNotFound& snf)
  {
    SDPA_LOG_FATAL("could not send WorkerRegistrationAck: locate slave-stage failed: " << snf.what());
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
      SDPA_LOG_WARN("The worker "<<worker_id<<" rejected the job "<<error.job_id().str()<<". Reschedule it now!");

      scheduler()->reschedule(worker_id, jobId);
      break;
    }
    case ErrorEvent::SDPA_EWORKERNOTREG:
    {
      SDPA_LOG_WARN("New instance of the master is up, sending new registration request!");
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
          SDPA_LOG_INFO("worker " << worker_id << " went down (clean). Tell the WorkerManager to remove it!");

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
          scheduler()->reschedule(worker_id);
          scheduler()->delWorker(worker_id); // do a re-scheduling here
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
              SDPA_LOG_WARN("The connection with the master " << masterInfo.name() << " is broken!");
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
      SDPA_LOG_INFO("The worker managed to recover the job "<<error.job_id()<<", it already has it!");
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
        SDPA_LOG_ERROR("The job " << error.job_id() << " was not found on"<<name()<<"!");
      }
      catch(WorkerNotFoundException const &)
      {
        SDPA_LOG_ERROR("job re-submission could not be acknowledged: worker " << worker_id << " not found!!");
      }
      catch(std::exception const &ex)
      {
        SDPA_LOG_ERROR("Unexpected exception occurred upon receiving ErrorEvent::SDPA_EJOBEXISTS: " << ex.what());
      }

      break;
    }
    case ErrorEvent::SDPA_EPERM:
    {
      SDPA_LOG_WARN("Got error from "<<error.from()<<". Reason: "<<error.reason());
      if( error.job_id() != sdpa::job_id_t::invalid_job_id() )
      {
        // check if there were any jobs submitted and not acknowledged to that worker
        // if this is the case, move the submitted jobs back into the pending queue
        // don't forget to update the state machine
        sdpa::job_id_t jobId(error.job_id());
        sdpa::worker_id_t worker_id(error.from());
        SDPA_LOG_WARN("The worker "<<worker_id<<" rejected the job "<<error.job_id().str()<<". Re-assign it now!");

        scheduler()->reassign(worker_id, jobId);
      }
       break;
    }
    default:
    {
      MLOG(WARN, "got an ErrorEvent back (ignoring it): code=" << error.error_code() << " reason=" << error.reason());
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
void GenericDaemon::submit(const id_type& activityId, const encoded_type& desc, const requirement_list_t& job_req_list )
{
  // create new job with the job description = workflow (serialize it first)
  // set the parent_id to ?
  // add this job into the parent's job list (call parent_job->add_subjob( new job(workflow) ) )
  // schedule the new job to some worker

  try {
    DMLOG(TRACE, "workflow engine submitted "<<activityId);

    job_id_t job_id(activityId);
    job_id_t parent_id("WE"); // is this really needed?

    jobManager()->addJobRequirements(job_id, job_req_list);

    // WORK HERE: limit number of maximum parallel jobs
    jobManager()->waitForFreeSlot ();

    // don't forget to set here the job's preferences
    SubmitJobEvent::Ptr pEvtSubmitJob( new SubmitJobEvent( sdpa::daemon::WE, name(), job_id, desc, parent_id) );
    sendEventToSelf(pEvtSubmitJob);
  }
  catch(QueueFull const &)
  {
    SDPA_LOG_ERROR( "could not send event to my stage:"
                  << " queue is full!"
                  );
    workflowEngine()->failed( activityId
                            , desc
                            , fhg::error::UNEXPECTED_ERROR
                            , "internal queue had an overflow"
                            );
  }
  catch(seda::StageNotFound const &)
  {
    SDPA_LOG_ERROR( "Stage not found when trying to deliver"
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
    SDPA_LOG_ERROR( "unexpected exception during submitJob: "
                  << ex.what()
                  );
    workflowEngine()->failed( activityId
                            , desc
                            , fhg::error::UNEXPECTED_ERROR
                            , ex.what()
                            );
    throw;
  }
  catch(...)
  {
    SDPA_LOG_ERROR("unexpected exception during submitJob!");
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
  SDPA_LOG_WARN ("The workflow engine requests the cancellation of the activity " << activityId << "( reason: " << reason<<")!");

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
  SDPA_LOG_INFO ("activity finished: " << workflowId);
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
    ( new JobFailedEvent( sdpa::daemon::WE, name(), job_id, result ));
  pEvtJobFailed->error_code() = error_code;
  pEvtJobFailed->error_message() = reason;

  sendEventToSelf(pEvtJobFailed);

  return true;
}

/**
 * Notify the SDPA that a workflow has been canceled (state
 * transition from * to terminated.
 */
bool GenericDaemon::cancelled(const id_type& workflowId)
{
  SDPA_LOG_INFO ("activity cancelled: " << workflowId);
  // generate a JobCancelledEvent for self!

  job_id_t job_id(workflowId);

  CancelJobAckEvent::Ptr pEvtCancelJobAck( new CancelJobAckEvent(sdpa::daemon::WE, name(), job_id ));
  sendEventToSelf(pEvtCancelJobAck);

  return true;
}

Job::ptr_t& GenericDaemon::findJob(const sdpa::job_id_t& job_id ) const
{
  try {
    return jobManager()->findJob(job_id);
  }
  catch(const JobNotFoundException& ex)
  {
    throw ex;
  }
}

void GenericDaemon::deleteJob(const sdpa::job_id_t& jobId)
{
  try {
    jobManager()->deleteJob(jobId);
  }
  catch(const JobNotDeletedException& ex)
  {
    throw ex;
  }
}

const requirement_list_t GenericDaemon::getJobRequirements(const sdpa::job_id_t& jobId) const
{
  try {
    return jobManager()->getJobRequirements(jobId);
  }
  catch (const NoJobRequirements& ex)
  {
    throw ex;
  }
}

void GenericDaemon::submitWorkflow(const id_type& wf_id, const encoded_type& desc )
{
  if(!ptr_workflow_engine_)
    throw NoWorkflowEngine();

  ptr_workflow_engine_->submit(wf_id, desc);
}

void GenericDaemon::cancelWorkflow(const id_type& workflowId, const std::string& reason)
{
  if (hasWorkflowEngine())
  {
    ptr_workflow_engine_->cancel(workflowId, reason);
  }
  else
  {
    LOG(WARN, "would cancel " << workflowId << " on myself");
  }
}

void GenericDaemon::handleWorkerRegistrationAckEvent(const sdpa::events::WorkerRegistrationAckEvent* pRegAckEvt)
{
  std::string masterName = pRegAckEvt->from();
  SDPA_LOG_INFO("Received registration acknowledgment from "<<masterName);

  bool bFound = false;
  lock_type lock(mtx_master_);
  for( sdpa::master_info_list_t::iterator it = m_arrMasterInfo.begin(); it != m_arrMasterInfo.end() && !bFound; it++)
    if( it->name() == masterName )
    {
      SDPA_LOG_INFO("Mark the agent "<<name()<<" as registered within the corresponding MasterInfo object ... ");
      it->set_registered(true);
      bFound=true;
    }

  // for all jobs that are in a terminal state and not yet acknowledged by the  master
  // re-submit  them to the master, after registration

  if(!isTop())
    jobManager()->resubmitResults(this);
}

void GenericDaemon::handleConfigReplyEvent(const sdpa::events::ConfigReplyEvent* pCfgReplyEvt)
{
  SDPA_LOG_DEBUG("Received ConfigReplyEvent from "<<pCfgReplyEvt->from());
}

void GenericDaemon::registerWorker(const WorkerRegistrationEvent& evtRegWorker)
{
  worker_id_t worker_id (evtRegWorker.from());

  SDPA_LOG_INFO( "****************Got new registration request from: " << worker_id << ", capacity = "<<evtRegWorker.capacity()<<", capabilities:" );
  std::cout<<evtRegWorker.capabilities()<<std::endl;

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

  //SDPA_LOG_INFO("Register the worker \"" << worker_id << "\"" <<", added the following capabilities: "<< workerCpbSet);

  // send back an acknowledgment
  SDPA_LOG_INFO( "Send back to the worker " << worker_id << " a registration acknowledgment!" );
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
  DMLOG(TRACE, "Received CapabilitiesGainedEvent!");
  // tell the scheduler to add the capabilities of the worker pCpbGainEvt->from
  sdpa::worker_id_t worker_id = pCpbGainEvt->from();

  if( pCpbGainEvt->capabilities().empty() )
  {
    //SDPA_LOG_ERROR("Received empty set of capabilities from agent "<<worker_id);
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
    SDPA_LOG_ERROR("Could not add new capabilities. The worker "<<worker_id<<" was not found!");
  }
  catch( const AlreadyHasCpbException& ex )
  {
    //SDPA_LOG_ERROR("The agent "<<name()<<" already has the capability "<<ex.capability());
  }
  catch( const std::exception& ex)
  {
    SDPA_LOG_ERROR("Unexpected exception ("<<ex.what()<<") occurred when trying to add new capabilities to the worker "<<worker_id);
  }
}

void GenericDaemon::handleCapabilitiesLostEvent(const sdpa::events::CapabilitiesLostEvent* pCpbLostEvt)
{
  DMLOG(TRACE, "Received CapabilitiesLostEvent!");
  // tell the scheduler to remove the capabilities of the worker pCpbLostEvt->from

  sdpa::worker_id_t worker_id = pCpbLostEvt->from();
  try {
    scheduler()->removeCapabilities(worker_id, pCpbLostEvt->capabilities());

    SDPA_LOG_INFO("lost capabilities from: " << worker_id << ": "<<pCpbLostEvt->capabilities());

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
    SDPA_LOG_ERROR("Could not remove the specified capabilities. The worker "<<worker_id<<" was not found!");
  }
  catch( const std::exception& ex)
  {
    SDPA_LOG_ERROR("Unexpected exception ("<<ex.what()<<") occurred when trying to remove some capabilities of the worker "<<worker_id);
  }
}

void GenericDaemon::handleSubscribeEvent( const sdpa::events::SubscribeEvent* pEvt )
{
  DMLOG(TRACE, "Received subscribe event!");
  try {
    subscribe(pEvt->subscriber(), pEvt->listJobIds());
  }
  catch(...)
  {
    SDPA_LOG_WARN("An exception occurred when "<<pEvt->subscriber()<<" was attempting to subscribe!");
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
      SDPA_LOG_ERROR("Daemon stage not defined! ");
    }
  }
  catch(const seda::QueueFull&)
  {
    SDPA_LOG_WARN("Could not send event. The queue is full!");
  }
  catch(const seda::StageNotFound& ex)
  {
    SDPA_LOG_ERROR("Stage not found! "<<ex.what());
  }
  catch(const std::exception& ex)
  {
    SDPA_LOG_WARN("Could not send event. Exception occurred: "<<ex.what());
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
      SDPA_LOG_ERROR("The master stage does not exist!");
    }
  }
  catch(const QueueFull&)
  {
    SDPA_LOG_WARN("Could not send event. The queue is full!");
  }
  catch(const seda::StageNotFound& )
  {
    SDPA_LOG_ERROR("Stage "<<to_master_stage()->name()<<" not found!");
  }
  catch(const std::exception& ex)
  {
    SDPA_LOG_WARN("Could not send event. Exception occurred: "<<ex.what());
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
      SDPA_LOG_ERROR("The slave stage does not exist!");
    }
  }
  catch(const QueueFull&)
  {
    SDPA_LOG_WARN("Could not send event. The queue is full!");
  }
  catch(const seda::StageNotFound& )
  {
    SDPA_LOG_ERROR("Stage "<<to_slave_stage()->name()<<" not found!");
  }
  catch(const std::exception& ex)
  {
    SDPA_LOG_WARN("Could not send event. Exception occurred: "<<ex.what());
  }
}

Worker::ptr_t const & GenericDaemon::findWorker(const Worker::worker_id_t& worker_id ) const
{
  try {
    return  scheduler()->findWorker(worker_id);
  }
  catch(const WorkerNotFoundException& ex) {
    throw ex;
  }
}

const Worker::worker_id_t& GenericDaemon::findWorker(const sdpa::job_id_t& job_id) const
{
  try {
    return  scheduler()->findWorker(job_id);
  }
  catch(const NoWorkerFoundException& ex) {
    throw ex;
  }
}

void GenericDaemon::addWorker(  const Worker::worker_id_t& workerId,
                                unsigned int cap,
                                const capabilities_set_t& cpbset,
                                const unsigned int& agent_rank,
                                const sdpa::worker_id_t& agent_uuid )
{
  try {
    scheduler()->addWorker(workerId, cap, cpbset, agent_rank, agent_uuid);
  }
  catch( const WorkerAlreadyExistException& ex )
  {
    throw ex;
  }
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
  //SDPA_LOG_INFO("The actual polling interval is: "<<m_ullPollingInterval);

  if(!m_bRequestsAllowed)
  {
      SDPA_LOG_WARN("The flag \"m_bRequestsAllowed\" is set on false!!!!!!!!!!!!!!!!!!!!!!!!! ");
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
      SDPA_LOG_WARN("Could not find the job "<<jobId.str()<<" ...");
    }
  }
  else
  {
    DLOG(TRACE, "Send JobFailedEvent to self for the job"<<jobId);
    JobFailedEvent::Ptr pEvtJobFailed
      (new JobFailedEvent(worker_id, name(), jobId, reason));
    pEvtJobFailed->error_code() = error_code;
    pEvtJobFailed->error_message() = reason;
    sendEventToSelf(pEvtJobFailed);
  }
}

void GenericDaemon::activityFinished(const Worker::worker_id_t& worker_id, const job_id_t& jobId, const result_type & result)
{
  if( hasWorkflowEngine() )
  {
    SDPA_LOG_INFO("worker job finished: " << jobId);
    workflowEngine()->finished( jobId.str(), result );
    try {
      jobManager()->deleteJob(jobId);
    }
    catch(const JobNotDeletedException& ex)
    {
      SDPA_LOG_WARN("Could not find the job "<<jobId.str()<<" ...");
    }
  }
  else
  {
    SDPA_LOG_INFO("Sent self a jobFinishedEvent for the job"<<jobId);
    JobFinishedEvent::Ptr pEvtJobFinished(new JobFinishedEvent(worker_id, name(), jobId, result));
    sendEventToSelf(pEvtJobFinished);
  }
}

void GenericDaemon::activityCancelled(const Worker::worker_id_t& worker_id, const job_id_t& jobId)
{
  if( hasWorkflowEngine() )
  {
    SDPA_LOG_INFO("worker job cancelled: " << jobId);
    workflowEngine()->cancelled( jobId.str() );
    try {
      jobManager()->deleteJob(jobId);
    }
    catch(const JobNotDeletedException& ex)
    {
      SDPA_LOG_WARN("Could not find the job "<<jobId.str()<<" ...");
    }
  }
  else
  {
    DLOG(TRACE, "Sent CancelJobAckEvent to self for the job"<<jobId);
    CancelJobAckEvent::Ptr pEvtCancelJobAck(new CancelJobAckEvent(worker_id, name(), jobId));
    sendEventToSelf(pEvtCancelJobAck);
  }
}

void GenericDaemon::requestJob(const MasterInfo& masterInfo)
{
  if( masterInfo.is_registered() )
  {
    //SDPA_LOG_INFO( "Post a new job request to the master "<<master );
    RequestJobEvent::Ptr pEvtReq( new RequestJobEvent( name(), masterInfo.name() ) );
    sendEventToMaster(pEvtReq);
  }

  updateLastRequestTime();
}

void GenericDaemon::requestRegistration(const MasterInfo& masterInfo)
{
  if( !masterInfo.is_registered() )
  {
    SDPA_LOG_INFO("The agent \"" << name()
                                         << "\" is sending a registration event to master \"" << masterInfo.name()
                                         << "\", capacity = "<<capacity() );

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

  SDPA_LOG_ERROR("The agent "<<name()<<" has no scheduler!");
  throw std::runtime_error(name() + " does not have scheduler!");
}

void GenericDaemon::reschedule(const sdpa::job_id_t& jobId)
{
  if( scheduler() )
  {
    scheduler()->reschedule(jobId);
    return;
  }

  SDPA_LOG_ERROR("The agent "<<name()<<" has no scheduler!");
  throw std::runtime_error(name() + " does not have scheduler!");
}

void GenericDaemon::start_fsm()
{
  // to be overriden by DaemonFSM
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
  if( info.name() == agId )
    return true;
  else
    return false;
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
  for(sdpa::capabilities_set_t::iterator it = m_capabilities.begin(); it!= m_capabilities.end(); it++ )
    cpbset.insert(*it);

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
  for(sdpa::job_id_list_t::const_iterator it = m_listSubscribers[agId].begin(); it != m_listSubscribers[agId].end(); it++ )
    if( *it == jobId )
      return true;

  return false;
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
  SDPA_LOG_INFO("reply immediately with a SubscribeAckEvent");
  sdpa::events::SubscribeAckEvent::Ptr ptrSubscAckEvt(new sdpa::events::SubscribeAckEvent(name(), subscriber, listJobIds));
  sendEventToMaster(ptrSubscAckEvt);

  // check if the subscribed jobs are already in a terminal state
  BOOST_FOREACH(const sdpa::JobId& jobId, listJobIds)
  {
    //try
    {
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
                             )
          );
        pEvtJobFailed->error_code() =
          fhg::error::UNASSIGNED_ERROR;
        pEvtJobFailed->error_message() =
          "TODO: take the error message from the job"
          " pointer somehow";
        sendEventToMaster(pEvtJobFailed);
      }
      else if( jobStatus.find("Cancelled") != std::string::npos)
      {
        CancelJobAckEvent::Ptr pEvtCancelJobAck( new CancelJobAckEvent( name(), subscriber, pJob->id() ));
        sendEventToMaster(pEvtCancelJobAck);
      }
    }
  }
}

bool GenericDaemon::isSubscriber(const sdpa::agent_id_t& agentId)
{
  lock_type lock(mtx_subscriber_);
  sdpa::subscriber_map_t::iterator it = m_listSubscribers.find(agentId);

  return (it != m_listSubscribers.end());
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
    SDPA_LOG_INFO("Re-schedule the job"<<jobId);
    reschedule(jobId);
  }
}
