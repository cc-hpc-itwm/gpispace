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
#include <seda/StageRegistry.hpp>
#include <seda/comm/comm.hpp>
#include <seda/comm/ConnectionFactory.hpp>
#include <seda/comm/ConnectionStrategy.hpp>
#include <sdpa/events/CodecStrategy.hpp>

#include <sdpa/daemon/GenericDaemon.hpp>
#include <sdpa/daemon/JobImpl.hpp>
#include <sdpa/daemon/jobFSM/JobFSM.hpp>
#include <sdpa/events/ConfigReplyEvent.hpp>
#include <sdpa/events/StartUpEvent.hpp>
#include <sdpa/events/ConfigOkEvent.hpp>

#include <sdpa/uuid.hpp>
#include <sdpa/uuidgen.hpp>

#include <sdpa/daemon/exceptions.hpp>

#include <sdpa/daemon/jobFSM/JobFSM.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>


using namespace std;
using namespace sdpa::daemon;
using namespace sdpa::events;

//Provide ptr to an implementation of Sdpa2Gwes
GenericDaemon::GenericDaemon(	const std::string &name,
								seda::Stage* ptrToMasterStage,
								seda::Stage* ptrToSlaveStage,
								IWorkflowEngine*  pArgSdpa2Gwes)
	: Strategy(name),
	  SDPA_INIT_LOGGER(name),
	  ptr_job_man_(new JobManager()),
	  ptr_scheduler_(),
	  ptr_workflow_engine_(pArgSdpa2Gwes),
	  ptr_to_master_stage_(ptrToMasterStage),
	  ptr_to_slave_stage_(ptrToSlaveStage),
	  daemon_stage_(NULL),
	  master_(""),
	  m_bRegistered(false),
	  m_nRank(0),
	  m_nExternalJobs(0),
	  delivery_service_(service_thread_.io_service(), 500)
{
}

GenericDaemon::GenericDaemon(	const std::string &name,
								const std::string& toMasterStageName,
								const std::string& toSlaveStageName,
								IWorkflowEngine*  pArgSdpa2Gwes)
	: Strategy(name),
	  SDPA_INIT_LOGGER(name),
	  ptr_job_man_(new JobManager()),
	  ptr_scheduler_(),
	  ptr_workflow_engine_(pArgSdpa2Gwes),
	  daemon_stage_(NULL),
	  master_(""),
	  m_bRegistered(false),
	  m_nRank(0),
	  m_nExternalJobs(0),
	  delivery_service_(service_thread_.io_service(), 500)
{
	if(!toMasterStageName.empty())
	{
		seda::Stage::Ptr pshToMasterStage = seda::StageRegistry::instance().lookup(toMasterStageName);
		ptr_to_master_stage_ = pshToMasterStage.get();
	}
	else
		ptr_to_master_stage_ = NULL;


	if(!toSlaveStageName.empty())
	{
		seda::Stage::Ptr pshToSlaveStage = seda::StageRegistry::instance().lookup(toSlaveStageName);
		ptr_to_slave_stage_ = pshToSlaveStage.get();
	}
	else
		ptr_to_slave_stage_ = NULL;
}

// with network scommunicatio
GenericDaemon::GenericDaemon( const std::string name, IWorkflowEngine*  pArgSdpa2Gwes)
	: Strategy(name),
	  SDPA_INIT_LOGGER(name),
	  ptr_job_man_(new JobManager()),
	  ptr_scheduler_(),
	  ptr_workflow_engine_(pArgSdpa2Gwes),
	  daemon_stage_(NULL),
	  master_(""),
	  m_bRegistered(false),
	  m_nRank(0),
	  m_nExternalJobs(0),
	  delivery_service_(service_thread_.io_service(), 500)
{
}

GenericDaemon::~GenericDaemon()
{
  DLOG(TRACE, "GenericDaemon destructor called ...");

	if(ptr_workflow_engine_)
	{
		DLOG(TRACE, "deleting workflow engine...");
		delete ptr_workflow_engine_;
		ptr_workflow_engine_ = NULL;
	}

	// Allocated outside and passed as a parameter
	daemon_stage_ = NULL;
}

void GenericDaemon::create_daemon_stage(const GenericDaemon::ptr_t& ptr_daemon )
{
	seda::Stage::Ptr daemon_stage( new seda::Stage(ptr_daemon->name() + "", ptr_daemon, 1) );
	ptr_daemon->setStage(daemon_stage.get());
	seda::StageRegistry::instance().insert(daemon_stage);
}

void GenericDaemon::configure_network( std::string daemonUrl, std::string masterName, std::string masterUrl )
{
	SDPA_LOG_DEBUG("configuring network components...");
	const std::string prefix(daemon_stage_->name()+".net");

	try {
		SDPA_LOG_DEBUG("setting up decoding...");
		seda::ForwardStrategy::Ptr fwd_to_daemon_strategy(new seda::ForwardStrategy(daemon_stage_->name()));
		sdpa::events::DecodeStrategy::ptr_t decode_strategy(new sdpa::events::DecodeStrategy(prefix+"-decode", fwd_to_daemon_strategy));
		seda::Stage::Ptr decode_stage(new seda::Stage(prefix+"-decode", decode_strategy));
		seda::StageRegistry::instance().insert(decode_stage);
		decode_stage->start();
	}
	catch(std::exception const& ex)
	{
		SDPA_LOG_ERROR("Exception occurred when trying to start the decoding stage: "<<ex.what());
		throw;
	}

	try {
		SDPA_LOG_DEBUG("setting up the network stage...");
		seda::comm::ConnectionFactory connFactory;
		seda::comm::ConnectionParameters params("udp", daemonUrl, daemon_stage_->name());
		seda::comm::Connection::ptr_t conn = connFactory.createConnection(params);
		// master known service: give the port as a parameter
		if( !masterName.empty() &&  !masterUrl.empty() )
		{
			setMaster(masterName);
			conn->locator()->insert( masterName, masterUrl);
		}

		seda::comm::ConnectionStrategy::ptr_t conn_s(new seda::comm::ConnectionStrategy(prefix+"-decode", conn));
		seda::Stage::Ptr network_stage(new seda::Stage(prefix, conn_s));
		seda::StageRegistry::instance().insert(network_stage);
		network_stage->start();

		SDPA_LOG_DEBUG("setting up the output/encoding stage...");
		seda::ForwardStrategy::Ptr fwd_to_net_strategy(new seda::ForwardStrategy(network_stage->name()));
		sdpa::events::EncodeStrategy::ptr_t encode_strategy(new sdpa::events::EncodeStrategy(prefix+"-encode", fwd_to_net_strategy));
		seda::Stage::Ptr encode_stage(new seda::Stage(prefix+"-encode", encode_strategy));
		seda::StageRegistry::instance().insert(encode_stage);

		ptr_to_master_stage_ = ptr_to_slave_stage_ = encode_stage.get();
		encode_stage->start();
	}
	catch(std::exception const & ex)
	{
		SDPA_LOG_ERROR("Exception occurred when trying to start the network stage: "<<ex.what());
		throw;
	}
}

void GenericDaemon::shutdown_network()
{

    if( !master().empty() && is_registered())
    	sendEventToMaster (ErrorEvent::Ptr(new ErrorEvent(name(), master(), ErrorEvent::SDPA_ENODE_SHUTDOWN, "node shutdown")));

	SDPA_LOG_DEBUG("shutting-down the network components of the daemon "<<daemon_stage_->name());
	const std::string prefix(daemon_stage_->name()+".net");

	SDPA_LOG_DEBUG("shutdown the decoding stage...");
	std::string decode_stage_name = prefix+"-decode";
	seda::Stage::Ptr decode_stage = seda::StageRegistry::instance().lookup(decode_stage_name);
	decode_stage->stop();

	SDPA_LOG_DEBUG("shutdown the network stage...");
	std::string network_stage_name = prefix;
	seda::Stage::Ptr network_stage = seda::StageRegistry::instance().lookup(network_stage_name);
	network_stage->stop();

	SDPA_LOG_DEBUG("shutdown the encoding stage...");
	std::string encode_stage_name = prefix+"-encode";
	seda::Stage::Ptr encode_stage = seda::StageRegistry::instance().lookup(encode_stage_name);

	encode_stage->stop();
	ptr_to_master_stage_ = ptr_to_slave_stage_ = NULL;
}

void GenericDaemon::start(const GenericDaemon::ptr_t& ptr_daemon )
{
	ptr_daemon->ptr_daemon_cfg_ = sdpa::util::Config::create(); // initialize it with default options

	// The stage uses 2 threads
	ptr_daemon->daemon_stage()->start();

	//start-up the the daemon
	StartUpEvent::Ptr pEvtStartUp(new StartUpEvent());
	ptr_daemon->daemon_stage()->send(pEvtStartUp);

	sleep(1);

	// configuration done
	ConfigOkEvent::Ptr pEvtConfigOk( new ConfigOkEvent());
	ptr_daemon->daemon_stage()->send(pEvtConfigOk);
}

void GenericDaemon::start( const GenericDaemon::ptr_t& ptr_daemon,  sdpa::util::Config::ptr_t ptrConfig )
{
	ptr_daemon->ptr_daemon_cfg_ = ptrConfig; // initialize it with default options

	// The stage uses 2 threads
	ptr_daemon->daemon_stage()->start();

	//start-up the the daemon
	StartUpEvent::Ptr pEvtStartUp(new StartUpEvent());
	ptr_daemon->daemon_stage()->send(pEvtStartUp);

	sleep(1);

	// configuration done
	ConfigOkEvent::Ptr pEvtConfigOk( new ConfigOkEvent());
	ptr_daemon->daemon_stage()->send(pEvtConfigOk);
}

void GenericDaemon::stop()
{
	seda::StageRegistry::instance().lookup(name())->stop();
}

void GenericDaemon::perform(const seda::IEvent::Ptr& pEvent)
{
	DLOG(TRACE, "perform (" << typeid(*pEvent.get()).name() << ")");
	if( SDPAEvent* pSdpaEvt = dynamic_cast<SDPAEvent*>(pEvent.get()) )
	{
		pSdpaEvt->handleBy(this);
	}
	else
	{
		SDPA_LOG_WARN("Received unexpected event " << pEvent->str()<<". Cannot handle it!");
	}
}

void GenericDaemon::handleWorkerRegistrationAckEvent(const sdpa::events::WorkerRegistrationAckEvent* pRegAckEvt)
{
	SDPA_LOG_DEBUG("Received WorkerRegistrationAckEvent from "<<pRegAckEvt->from());
    acknowledge (pRegAckEvt->id());
	m_bRegistered = true;
}

void GenericDaemon::handleConfigReplyEvent(const sdpa::events::ConfigReplyEvent* pCfgReplyEvt)
{
	SDPA_LOG_DEBUG("Received ConfigReplyEvent from "<<pCfgReplyEvt->from());
}

void GenericDaemon::onStageStart(const std::string & /* stageName */)
{
	DMLOG(TRACE, "daemon stage is being started");
    //	MLOG(INFO, "registering myself (" << name() << ")...");

	DMLOG(TRACE, "starting delivery service...");
	delivery_service_.register_callback_handler(boost::bind(&GenericDaemon::messageDeliveryFailed, this, _1));
	delivery_service_.start();
	service_thread_.start();
	DMLOG(TRACE, "starting my scheduler...");

	try
	{
		ptr_scheduler_ = Scheduler::ptr_t(this->create_scheduler());
		ptr_scheduler_->start();
	} catch (...)
	{
		ptr_scheduler_->stop();
		ptr_scheduler_.reset();
		throw;
	}
}

void GenericDaemon::onStageStop(const std::string & /* stageName */)
{
	DMLOG(TRACE, "daemon stage is being stopped");
	// stop the scheduler thread
	ptr_scheduler_->stop();

	service_thread_.stop();
	delivery_service_.stop();

	ptr_to_master_stage_ = NULL;
	ptr_to_slave_stage_ = NULL;
}

void GenericDaemon::sendEventToSelf(const SDPAEvent::Ptr& pEvt)
{
	try {
		if(daemon_stage_)
		{
			daemon_stage_->send(pEvt);
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

void GenericDaemon::sendEventToMaster(const sdpa::events::SDPAEvent::Ptr& pEvt, std::size_t retries, unsigned long timeout)
{
	try {
		delivery_service_.send(to_master_stage(), pEvt, pEvt->id(), timeout, retries);
		DLOG(TRACE, "Sent " << pEvt->str() << " to " << pEvt->to() << ": message-id: " << pEvt->id());
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

void GenericDaemon::sendEventToSlave(const sdpa::events::SDPAEvent::Ptr& pEvt, std::size_t retries, unsigned long timeout)
{
	try {
		delivery_service_.send(to_slave_stage(), pEvt, pEvt->id(), timeout, retries);
		DLOG(TRACE, "Sent " <<pEvt->str()<<" to "<<pEvt->to());
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

bool GenericDaemon::acknowledge(const sdpa::events::SDPAEvent::message_id_type &mid)
{
  return delivery_service_.acknowledge(mid);
}

void GenericDaemon::messageDeliveryFailed(sdpa::events::SDPAEvent::Ptr e)
{
  MLOG(ERROR, "delivery of message[" << e->id() << "] failed: " << e->str());
}

Worker::ptr_t GenericDaemon::findWorker(const Worker::worker_id_t& worker_id ) throw(WorkerNotFoundException)
{
	try {
		return  ptr_scheduler_->findWorker(worker_id);
	}
	catch(const WorkerNotFoundException& ex) {
          throw ex;
	}
}

const Worker::worker_id_t& GenericDaemon::findWorker(const sdpa::job_id_t& job_id) throw (NoWorkerFoundException)
{
	try {
		return  ptr_scheduler_->findWorker(job_id);
	}
	catch(const NoWorkerFoundException& ex) {
		  throw ex;
	}
}

void GenericDaemon::addWorker( const Worker::worker_id_t& workerId, unsigned int rank ) throw (WorkerAlreadyExistException)
{
	try {
		ptr_scheduler_->addWorker(workerId, rank);
	}catch( const WorkerAlreadyExistException& ex )
	{
		throw ex;
	}
}

bool GenericDaemon::requestsAllowed( const sdpa::util::time_type& difftime )
{
	// if m_nExternalJobs is null then slow it down, i.e. increase m_ullPollingInterval
	// reset it to the value specified by config first time when m_nExternalJobs becomes positive
	// don't forget to decrement m_nExternalJobs when the job is finished !

	if( extJobsCnt() == 0 && m_ullPollingInterval < ptr_daemon_cfg_->get<unsigned int>("upper bound polling interval") )
		m_ullPollingInterval  = m_ullPollingInterval + 10000;

	return (difftime>m_ullPollingInterval) &&
		   (m_nExternalJobs<cfg()->get<unsigned int>("nmax_ext_job_req"));
}

//actions
void GenericDaemon::action_configure(const StartUpEvent&)
{
	// should be overriden by the orchestrator, aggregator and NRE
	SDPA_LOG_INFO("Configuring myself (generic)...");

	// use for now as below, later read from config file
	ptr_daemon_cfg_->put("polling interval",    1 * 1000 * 1000);
	ptr_daemon_cfg_->put("upper bound polling interval", 5 * 1000*1000 );
	ptr_daemon_cfg_->put("life-sign interval",  2 * 1000 * 1000);
	ptr_daemon_cfg_->put("node_timeout",        6 * 1000 * 1000);

	m_ullPollingInterval = cfg()->get<sdpa::util::time_type>("polling interval");
}

void GenericDaemon::action_config_ok(const ConfigOkEvent&)
{
	// check if the system should be recovered
	// should be overriden by the orchestrator, aggregator and NRE
	SDPA_LOG_INFO("configuration was ok");
}

void GenericDaemon::action_config_nok(const ConfigNokEvent &)
{
	SDPA_LOG_FATAL("configuration was not ok!");
}

void GenericDaemon::action_interrupt(const InterruptEvent&)
{
	SDPA_LOG_DEBUG("Call 'action_interrupt'");
	// save the current state of the system .i.e serialize the daemon's state
}

void GenericDaemon::action_lifesign(const LifeSignEvent& e)
{
  DLOG(TRACE, "Received LS from the worker " << e.from());

    /*
    o timestamp, load, other probably useful information
    o last_job_id the id of the last received job identification
    o the aggregator first sends a request for configuration to its orchestrator
    o the orchestrator allocates an internal data structure to keep track of the state of the aggregator
    o this datastructure is being updated everytime a message is received
	o an aggregator is supposed to be unavailable when no messages have been received for a (configurable) period of time
     */
	try {
          Worker::ptr_t ptrWorker = findWorker(e.from());
          ptrWorker->update();
	}
	catch(WorkerNotFoundException const &)
	{
		SDPA_LOG_ERROR("got LS from unknown worker: " << e.from());
		// the worker should register first, before posting a job request
		ErrorEvent::Ptr pErrorEvt(new ErrorEvent(name(), e.from(), ErrorEvent::SDPA_EWORKERNOTREG, "not registered") );
		sendEventToSlave(pErrorEvt);
	} catch(...) {
		SDPA_LOG_ERROR("Unexpected exception occurred!");
	}
}

void GenericDaemon::action_delete_job(const DeleteJobEvent& e )
{
	LOG( INFO, e.from() << " requesting to delete job " << e.job_id() );

	try{
		Job::ptr_t pJob = ptr_job_man_->findJob(e.job_id());
		pJob->DeleteJob(&e);

		ptr_job_man_->deleteJob(e.job_id());
	} catch(JobNotFoundException const &){
          SDPA_LOG_ERROR("Job " << e.job_id() << " could not be found!");
          sendEventToMaster( ErrorEvent::Ptr( new ErrorEvent( name()
                                                            , e.from()
                                                            , ErrorEvent::SDPA_EJOBNOTFOUND
                                                            , "no such job"
                                                            )
                                            )
                           );
	} catch(JobNotMarkedException const &){
          SDPA_LOG_WARN("Job " << e.job_id() << " not ready for deletion!");
          sendEventToMaster( ErrorEvent::Ptr( new ErrorEvent( name()
                                                            , e.from()
                                                            , ErrorEvent::SDPA_EAGAIN
                                                            , "not ready for deletion, try again later"
                                                            )
                                            )
                           );
	}catch(JobNotDeletedException const & ex){
          SDPA_LOG_ERROR("Job " << e.job_id() << " could not be deleted!");
          sendEventToMaster( ErrorEvent::Ptr( new ErrorEvent( name()
                                                            , e.from()
                                                            , ErrorEvent::SDPA_EUNKNOWN
                                                            , ex.what()
                                                            )
                                            )
                           );
        } catch(std::exception const & ex) {
          SDPA_LOG_ERROR("unexpected exception during job-deletion: " << ex.what());
          sendEventToMaster( ErrorEvent::Ptr( new ErrorEvent( name()
                                                            , e.from()
                                                            , ErrorEvent::SDPA_EUNKNOWN
                                                            , ex.what()
                                                            )
                                            )
                           );
          throw;
	} catch(...) {
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

void GenericDaemon::action_request_job(const RequestJobEvent& e)
{
	DLOG(DEBUG, "got job request from: " << e.from());

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
	try {

		// you should consume from the  worker's pending list; put the job into the worker's submitted list
		sdpa::job_id_t jobId = ptr_scheduler_->getNextJob(worker_id, e.last_job_id());

		const Job::ptr_t& ptrJob = jobManager()->findJob(jobId);

		if( ptrJob.get() )
		{
			// put the job into the Running state here
			ptrJob->Dispatch(); // no event need to be sent

			// create a SubmitJobEvent for the job job_id serialize and attach description
			SDPA_LOG_DEBUG("sending SubmitJobEvent (jid=" << ptrJob->id() << ") to: " << e.from());
			SubmitJobEvent::Ptr pSubmitEvt(new SubmitJobEvent(name(), e.from(), ptrJob->id(),  ptrJob->description(), ""));

			// Post a SubmitJobEvent to the slave who made the request
			sendEventToSlave(pSubmitEvt, 0);
		}
		else // send an error event
		{
			SDPA_LOG_DEBUG("no job available, get_next_job should probably throw an exception?");
		}
	}
	catch(const NoJobScheduledException&)
	{
          DLOG (DEBUG, "No job was scheduled to be executed on the worker '"<<worker_id);
	}
	catch(const WorkerNotFoundException&)
	{
		SDPA_LOG_INFO("worker " << worker_id << " is not registered, asking him to do so first");

		// the worker should register first, before posting a job request
		ErrorEvent::Ptr pErrorEvt(new ErrorEvent(name(), e.from(), ErrorEvent::SDPA_EWORKERNOTREG, "not registered") );

		sendEventToSlave(pErrorEvt, 0);
	}
	catch(const QueueFull&)
	{
		SDPA_LOG_FATAL("Could not send event to internal stage: " << ptr_to_slave_stage_->name() << ": queue is full!");
	}
	catch(const seda::StageNotFound&)
	{
		SDPA_LOG_FATAL("Could not lookup stage: " << ptr_to_slave_stage_->name());
	}
	catch(const std::exception &ex)
	{
		SDPA_LOG_FATAL("Error during request-job handling: " << ex.what());
	}
	catch(...)
	{
		SDPA_LOG_FATAL("Unknown error during request-job handling!");
	}
}


void GenericDaemon::action_submit_job(const SubmitJobEvent& e)
{
	DLOG(TRACE, "got job submission from " << e.from() << ": job-id := " << e.job_id());
	/*
	* job-id (ignored by the orchestrator, see below)
    * contains workflow description and initial tokens
    * contains a flag for simulation
    * parse the workflow (syntax checking) using the builder pattern
          o creates an object-representation of the workflow
    * if something is wrong send an error message
    * create a new Job object and assign a unique job-id
    * put the job into the job-map
    * send a submitJobAck back
    */
	static const JobId job_id_empty ("");

	// First, check if the job 'job_id' wasn't already submitted!
	try {
		ptr_job_man_->findJob(e.job_id());
		return;
	} catch(const JobNotFoundException&){
          DLOG(TRACE, "Receive new job from "<<e.from() << " with job-id: " << e.job_id());
	}

	JobId job_id; //already assigns an unique job_id (i.e. the constructor calls the generator)
	if(e.job_id() != job_id_empty)  //use the job_id already  assigned by the master
		job_id = e.job_id();        //the orchestrator will assign a new job_id for the user jobs, the Agg/NRE will use the job_id assigned by the master

	try {
		// One should parse the workflow in order to be able to create a valid job
		// if the event comes from Gwes parent_id is the owner_workflow_id
		Job::ptr_t pJob( new JobFSM( job_id, e.description(), this, e.parent_id() ));

		// the job job_id is in the Pending state now!
		ptr_job_man_->addJob(job_id, pJob);

		// check if the message comes from outside/slave or from WFE
		// if it comes from outside set it as local
		if(e.from() != sdpa::daemon::WE ) //e.to())
		{
			LOG(DEBUG, "got new job from " << e.from() << " = " << job_id);
			pJob->set_local(true);
		}

		ptr_scheduler_->schedule(job_id);

		if(pJob->is_local())
		{
			//send back to the user a SubmitJobAckEvent
			SubmitJobAckEvent::Ptr pSubmitJobAckEvt(new SubmitJobAckEvent(name(), e.from(), job_id, e.id()));

			// There is a problem with this if uncommented
			sendEventToMaster(pSubmitJobAckEvt, 0);

			if( !master_.empty() )
				incExtJobsCnt();

		}
		//catch also workflow exceptions
	}catch(JobNotAddedException const &ex) {
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
	catch(std::exception const & ex) {
          SDPA_LOG_ERROR("Unexpected exception occured when calling 'action_submit_job' for the job "<<job_id<<": " << ex.what());
          throw;
	}
	catch(...) {
          SDPA_LOG_ERROR("Unexpected exception occured when calling 'action_submit_job' for the job "<<job_id<<"!");
          throw;
	}
}

void GenericDaemon::action_config_request(const ConfigRequestEvent& e)
{
  DLOG(TRACE, "got config request from " << e.from());
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
  unsigned int rank (evtRegWorker.rank());

	// check if the worker evtRegWorker.from() has already registered!
	try {
		const unsigned long long node_timeout (cfg()->get<unsigned long long>("node_timeout", 30 * 1000 * 1000));
        ptr_scheduler_->deleteNonResponsiveWorkers (node_timeout);

        LOG(TRACE, "Trying to registering new worker: " << worker_id << " with rank " << rank);

		addWorker( worker_id, rank );

		SDPA_LOG_INFO( "Registered the worker " << worker_id << " with the rank " << rank);

		// send back an acknowledgment
		WorkerRegistrationAckEvent::Ptr pWorkerRegAckEvt(new WorkerRegistrationAckEvent(name(), evtRegWorker.from()));
		pWorkerRegAckEvt->id() = evtRegWorker.id();
		sendEventToSlave(pWorkerRegAckEvt, 0);
	}
	catch(WorkerAlreadyExistException const & ex) {
          SDPA_LOG_ERROR( "An worker with either the same id or the same rank already exist into the worker map! "
                        "id="<<ex.worker_id()<<", rank="<<ex.rank());
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
  LOG(TRACE, "got error event from " << error.from() << " code: " << error.error_code() << " reason: " << error.reason());
  switch (error.error_code())
  {
	case ErrorEvent::SDPA_ENOERROR:
	{
	  // everything is fine, nothing to do
	  break;
	}
	case ErrorEvent::SDPA_EWORKERNOTREG:
	{
		MLOG(WARN, "my master forgot me and asked me to register again, sending WorkerRegistrationEvent");
		WorkerRegistrationEvent::Ptr pWorkerRegEvt(new WorkerRegistrationEvent(name(), error.from(), rank() ));
		sendEventToMaster(pWorkerRegEvt);
		break;
	}
	case ErrorEvent::SDPA_ENODE_SHUTDOWN:
	{
          MLOG(INFO, "worker " << error.from() << " went down (clean)");
          ptr_scheduler_->delWorker(error.from());
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
void GenericDaemon::submit(const id_type& activityId, const encoded_type& desc/*, preference_t pref*/ )
{
	// create new job with the job description = workflow (serialize it first)
	// set the parent_id to ?
	// add this job into the parent's job list (call parent_job->add_subjob( new job(workflow) ) )
	// schedule the new job to some worker
	// ATTENTION! Important assumption: the workflow_id should be set identical to the job_id!

	// simple generate a SubmitJobEvent cu from = to = name()
	// send an external job
	try {
          DLOG(TRACE, "workflow engine submitted "<<activityId);

		job_id_t job_id(activityId);
		job_id_t parent_id(""); // is this really needed?

		// TO DO: modify the prototype of the submit function
		//ptr_job_man_->addPreferences(job_id, pref);

        // WORK HERE: limit number of maximum parallel jobs
		ptr_job_man_->waitForFreeSlot ();

		// don't forget to set here the job's preferences
		SubmitJobEvent::Ptr pEvtSubmitJob(new SubmitJobEvent(sdpa::daemon::WE, name(), job_id, desc, parent_id));
		sendEventToSelf(pEvtSubmitJob);
	}
	catch(QueueFull const &)
	{
          SDPA_LOG_ERROR("could not send event to my stage, queue is full!");
          workflowEngine()->failed( activityId, "" ); // why?
	}
	catch(seda::StageNotFound const &)
	{
          SDPA_LOG_ERROR("Stage not found when trying to submit SubmitJobEvent!");
          workflowEngine()->failed( activityId, "" ); // why?
          throw;
	}
	catch(std::exception const & ex)
	{
          SDPA_LOG_ERROR("unexpected exception during submitJob: " << ex.what());
          workflowEngine()->failed( activityId, "" ); // why?
          throw;
	}
	catch(...)
	{
          SDPA_LOG_ERROR("unexpected exception during submitJob!");
          workflowEngine()->failed( activityId, "" ); // why?
          throw;
	}
}

/**
 * Cancel an atomic activity that has previously been submitted to
 * the SDPA.
 */
bool GenericDaemon::cancel(const id_type& activityId, const reason_type & reason)
{
  SDPA_LOG_INFO ("cancelling activity " << activityId << " reason: " << reason);

	// cancel the job corresponding to that activity -> send downward a CancelJobEvent?
	// look for the job_id corresponding to the received workflowId into job_map_
	// in fact they should be the same!
	// generate const CancelJobEvent& event
	// Job& job = job_map_[job_id];
	// call job.CancelJob(event);

	job_id_t job_id(activityId);
	CancelJobEvent::Ptr pEvtCancelJob(new CancelJobEvent(name(), name(), job_id));
	sendEventToSelf(pEvtCancelJob);
	return true;
}

/**
 * Notify the SDPA that a workflow finished (state transition
 * from running to finished).
 */
bool GenericDaemon::finished(const id_type& workflowId, const result_type& result)
{
  DLOG(TRACE, "activity finished: " << workflowId);
	// generate a JobFinishedEvent for self!
	// cancel the job corresponding to that activity -> send downward a CancelJobEvent?
	// look for the job_id corresponding to the received workflowId into job_map_
	// in fact they should be the same!
	// generate const JobFinishedEvent& event
	// Job& job = job_map_[job_id];
	// call job.JobFinished(event);

	job_id_t job_id(workflowId);
	JobFinishedEvent::Ptr pEvtJobFinished(new JobFinishedEvent(sdpa::daemon::WE, name(), job_id, result));
	sendEventToSelf(pEvtJobFinished);
	decExtJobsCnt();

	return true;
}

/**
 * Notify the SDPA that a workflow failed (state transition
 * from running to failed).
 */
bool GenericDaemon::failed(const id_type& workflowId, const result_type & result)
{
  SDPA_LOG_WARN ("activity failed: " << workflowId);
	// generate a JobFinishedEvent for self!
	// cancel the job corresponding to that activity -> send downward a CancelJobEvent?
	// look for the job_id corresponding to the received workflowId into job_map_
	// in fact they should be the same!
	// generate const JobFailedEvent& event
	// Job& job = job_map_[job_id];
	// call job.JobFailed(event);
	// if( has siblings)
	// kill the siblings and the master job, else kill the master job
	// The master process (User, Orch, Agg) should be informed that the job failed or  was canceled

	job_id_t job_id(workflowId);

	JobFailedEvent::Ptr pEvtJobFailed( new JobFailedEvent(sdpa::daemon::WE, name(), job_id, result ));
	sendEventToSelf(pEvtJobFailed);
	decExtJobsCnt();

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
	// identify the job with the job_id == workflow_id_t
	// trigger a CancelJobAck for that job

	job_id_t job_id(workflowId);

	CancelJobAckEvent::Ptr pEvtCancelJobAck(new CancelJobAckEvent(sdpa::daemon::WE, name(), job_id, SDPAEvent::message_id_type()));
	sendEventToSelf(pEvtCancelJobAck);
	decExtJobsCnt();

	return true;
}


Job::ptr_t& GenericDaemon::findJob(const sdpa::job_id_t& job_id ) throw(JobNotFoundException)
{
	try {
		return ptr_job_man_->findJob(job_id);
	}
	catch(const JobNotFoundException& ex)
	{
		throw ex;
	}
}

void GenericDaemon::jobFailed(const job_id_t& jobId, const std::string& reason)
{
  DLOG(TRACE, "informing workflow engine that " << jobId << " has failed: " << reason);
	workflowEngine()->failed( jobId.str(), reason );
	jobManager()->deleteJob(jobId);
}

const we::preference_t& GenericDaemon::getJobPreferences(const sdpa::job_id_t& jobId) const throw (NoJobPreferences)
{
	try {
		return ptr_job_man_->getJobPreferences(jobId);
	}
	catch (const NoJobPreferences& ex)
	{
		throw ex;
	}
}

void GenericDaemon::workerJobFailed(const job_id_t& jobId, const std::string& reason)
{
	DLOG(TRACE, "informing workflow engine that " << jobId << " has failed: " << reason);
	workflowEngine()->failed( jobId.str(), reason );
	jobManager()->deleteJob(jobId);
}

void GenericDaemon::workerJobFinished(const job_id_t& jobId, const result_type & result)
{
	DLOG(TRACE, "informing workflow engine that " << jobId << " has finished");
	workflowEngine()->finished( jobId.str(), result );
	jobManager()->deleteJob(jobId);
}

void GenericDaemon::workerJobCancelled(const job_id_t& jobId)
{
	DLOG(TRACE, "informing workflow engine that " << jobId << " has been cancelled");
	workflowEngine()->cancelled( jobId.str() );
	jobManager()->deleteJob(jobId);
}

void GenericDaemon::submitWorkflow(const id_type& wf_id, const encoded_type& desc )
{
	if(!ptr_workflow_engine_)
		throw NoWorkflowEngine();

	ptr_workflow_engine_->submit(wf_id, desc);
}

void GenericDaemon::cancelWorkflow(const id_type& workflowId, const std::string& reason)
{
	ptr_workflow_engine_->cancel(workflowId, reason);
}

void GenericDaemon::activityCancelled(const id_type& actId, const std::string& )
{
	ptr_workflow_engine_->cancelled( actId );
}

void GenericDaemon::incExtJobsCnt()
{
	lock_type lock(ext_job_cnt_mtx_);
	m_nExternalJobs++;

	// reset the polling interval
	if( m_nExternalJobs == 1 )
		m_ullPollingInterval = cfg()->get<sdpa::util::time_type>("polling interval");
}

void GenericDaemon::decExtJobsCnt()
{
	lock_type lock(ext_job_cnt_mtx_);
	m_nExternalJobs--;
}

unsigned int GenericDaemon::extJobsCnt()
{
	return m_nExternalJobs;
}
