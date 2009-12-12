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

#include <gwes/GWES.h>
#include <sdpa/wf/GwesGlue.hpp>

using namespace std;
using namespace sdpa::daemon;
using namespace sdpa::events;

//Provide ptr to an implementation of Sdpa2Gwes
GenericDaemon::GenericDaemon(	const std::string &name,
								seda::Stage* ptrToMasterStage,
								seda::Stage* ptrToSlaveStage,
								sdpa::Sdpa2Gwes*  pArgSdpa2Gwes)
	: Strategy(name),
	  SDPA_INIT_LOGGER(name),
	  ptr_job_man_(new JobManager()),
	  ptr_scheduler_(new SchedulerImpl(this)),
	  ptr_Sdpa2Gwes_(pArgSdpa2Gwes),
	  ptr_to_master_stage_(ptrToMasterStage),
	  ptr_to_slave_stage_(ptrToSlaveStage),
	  master_(""),
	  m_bRegistered(false)
{
	//master_ = "user"; // should be overriden by the derived classes to the proper value by reading a configuration file

	// initialize last request time
}

GenericDaemon::GenericDaemon(	const std::string &name,
								const std::string& toMasterStageName,
								const std::string& toSlaveStageName,
								sdpa::Sdpa2Gwes*  pArgSdpa2Gwes)
	: Strategy(name),
	  SDPA_INIT_LOGGER(name),
	  ptr_job_man_(new JobManager()),
	  ptr_scheduler_(new SchedulerImpl(this)),
	  ptr_Sdpa2Gwes_(pArgSdpa2Gwes),
	  master_(""),
	  m_bRegistered(false)
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
GenericDaemon::GenericDaemon( const std::string &name, sdpa::Sdpa2Gwes*  pArgSdpa2Gwes)
	: Strategy(name),
	  SDPA_INIT_LOGGER(name),
	  ptr_job_man_(new JobManager()),
	  ptr_scheduler_(new SchedulerImpl(this)),
	  ptr_Sdpa2Gwes_(pArgSdpa2Gwes),
	  master_(""),
	  m_bRegistered(false)
{
}

GenericDaemon::~GenericDaemon()
{
	SDPA_LOG_DEBUG("GenericDaemon destructor called ...");

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

	SDPA_LOG_DEBUG("setting up decoding...");
	seda::ForwardStrategy::Ptr fwd_to_daemon_strategy(new seda::ForwardStrategy(daemon_stage_->name()));
	sdpa::events::DecodeStrategy::ptr_t decode_strategy(new sdpa::events::DecodeStrategy(prefix+"-decode", fwd_to_daemon_strategy));
	seda::Stage::Ptr decode_stage(new seda::Stage(prefix+"-decode", decode_strategy));
	seda::StageRegistry::instance().insert(decode_stage);
	decode_stage->start();

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

void GenericDaemon::shutdown_network()
{
	SDPA_LOG_DEBUG("shutting-down the network components...");
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
	SDPA_LOG_DEBUG("Perform: Handling event " <<typeid(*pEvent.get()).name());

	if(dynamic_cast<MgmtEvent*>(pEvent.get()))
	{
		if( WorkerRegistrationAckEvent* pRegAckEvt = dynamic_cast<WorkerRegistrationAckEvent*>(pEvent.get()) )
			handleWorkerRegistrationAckEvent(pRegAckEvt);
		else if( ConfigReplyEvent* pCfgReplyEvt = dynamic_cast<ConfigReplyEvent*>(pEvent.get()) )
			handleConfigReplyEvent(pCfgReplyEvt);
		else
			handleDaemonEvent(pEvent);
	}
	else if(dynamic_cast<JobEvent*>(pEvent.get()))
		{
			if(	dynamic_cast<SubmitJobEvent*>(pEvent.get()) ) handleDaemonEvent(pEvent);
			else if( dynamic_cast<DeleteJobEvent*>(pEvent.get()) ) handleDaemonEvent(pEvent);
			else
				handleJobEvent(pEvent);
		}
	else if (DeleteWorkflowFromGWES *del = dynamic_cast<DeleteWorkflowFromGWES*>(pEvent.get()))
	{
	  if (gwes())
	  {
		try
		{
		  gwes()->removeWorkflow(del->id);
		}
		catch (const std::exception &ex)
		{
		  MLOG(ERROR, "could not remove workflow=" << del->id << ": " << ex.what());
		}
	  }
	}
	else
	{
	  SDPA_LOG_ERROR("got some unexpected event that i cannot handle: " << pEvent->str());
	}
}

void GenericDaemon::handleWorkerRegistrationAckEvent(const sdpa::events::WorkerRegistrationAckEvent* pRegAckEvt)
{
	SDPA_LOG_DEBUG("Received WorkerRegistrationAckEvent from "<<pRegAckEvt->from());
	m_bRegistered = true;
}

void GenericDaemon::handleConfigReplyEvent(const sdpa::events::ConfigReplyEvent* pCfgReplyEvt)
{
	SDPA_LOG_DEBUG("Received ConfigReplyEvent from "<<pCfgReplyEvt->from());
}

void GenericDaemon::handleDaemonEvent(const seda::IEvent::Ptr& pEvent)
{
	SDPA_LOG_DEBUG("Handle Management Event "<<typeid(*pEvent).name());
}

void GenericDaemon::onStageStart(const std::string & /* stageName */)
{
	DMLOG(DEBUG, "daemon stage is being started");
	MLOG(DEBUG, "registering myself (" << name() << ") with GWES...");
	// start the scheduler thread
	if(ptr_Sdpa2Gwes_)
		ptr_Sdpa2Gwes_->registerHandler(this);

	DMLOG(DEBUG, "starting my scheduler...");
	ptr_scheduler_->start();
}

void GenericDaemon::onStageStop(const std::string & /* stageName */)
{
	DMLOG(DEBUG, "daemon stage is being stopped");
	// stop the scheduler thread

	ptr_scheduler_->stop();
	if (ptr_Sdpa2Gwes_) ptr_Sdpa2Gwes_->unregisterHandler(this);
	ptr_Sdpa2Gwes_ = NULL;
	ptr_to_master_stage_ = NULL;
	ptr_to_slave_stage_ = NULL;
}

void GenericDaemon::sendDeleteEvent(const gwes::workflow_id_t &wid)
{
	try {
		if(daemon_stage_)
		{
			daemon_stage_->send(DeleteWorkflowFromGWES::Ptr(new DeleteWorkflowFromGWES(wid)));
		}
	}
	catch(const QueueFull&)
	{
		SDPA_LOG_DEBUG("Could not send event. The queue is full!");
	}
}


void GenericDaemon::sendEvent(const SDPAEvent::Ptr& pEvt)
{
	try {
		if(daemon_stage_)
		{
			daemon_stage_->send(pEvt);

			SDPA_LOG_DEBUG("Sent " <<pEvt->str()<<" to "<<pEvt->to());
		}
	}
	catch(const QueueFull&)
	{
		SDPA_LOG_DEBUG("Could not send event. The queue is full!");
	}
}

void GenericDaemon::sendEvent(seda::Stage* ptrOutStage, const sdpa::events::SDPAEvent::Ptr& pEvt)
{
	try {
		ptrOutStage->send(pEvt);

		SDPA_LOG_DEBUG("Sent " <<pEvt->str()<<" to "<<pEvt->to());
	}
	catch(QueueFull)
	{
		SDPA_LOG_DEBUG("Could not send event. The queue is full!");
	}
}

Worker::ptr_t GenericDaemon::findWorker(const Worker::worker_id_t& worker_id ) throw(WorkerNotFoundException)
{
	try {
		return  ptr_scheduler_->findWorker(worker_id);
	}
	catch(WorkerNotFoundException) {
		throw WorkerNotFoundException(worker_id);
	}
}

void GenericDaemon::addWorker(const  Worker::ptr_t pWorker)
{
	ptr_scheduler_->addWorker(pWorker);
}

//actions
void GenericDaemon::action_configure(const StartUpEvent&)
{
	// should be overriden by the orchestrator, aggregator and NRE
	SDPA_LOG_DEBUG("Call 'action_configure'");
	// use for now as below, later read from config file
	ptr_daemon_cfg_->put<sdpa::util::time_type>("polling interval",    1 * 1000 * 1000);
	ptr_daemon_cfg_->put<sdpa::util::time_type>("life-sign interval", 30 * 1000 * 1000);
}

void GenericDaemon::action_config_ok(const ConfigOkEvent&)
{
	// should be overriden by the orchestrator, aggregator and NRE
	SDPA_LOG_DEBUG("Call 'action_config_ok'");
	// in fact the master name should be red from the configuration file
	if( name() == sdpa::daemon::AGGREGATOR )
	{
		if(master().empty())
			setMaster(sdpa::daemon::ORCHESTRATOR);

		SDPA_LOG_DEBUG("Send WorkerRegistrationEvent to "<<master());
		WorkerRegistrationEvent::Ptr pEvtWorkerReg(new WorkerRegistrationEvent(name(), master()));
		to_master_stage()->send(pEvtWorkerReg);
	} else if( name() == sdpa::daemon::NRE )
	{
		if(master().empty())
			setMaster(sdpa::daemon::AGGREGATOR );

		SDPA_LOG_DEBUG("Send WorkerRegistrationEvent to "<<master());
		WorkerRegistrationEvent::Ptr pEvtWorkerReg(new WorkerRegistrationEvent(name(), master()));
		to_master_stage()->send(pEvtWorkerReg);
	}
}

void GenericDaemon::action_config_nok(const ConfigNokEvent&)
{
	SDPA_LOG_DEBUG("Call 'action_config_nok'");
}

void GenericDaemon::action_interrupt(const InterruptEvent&)
{
	SDPA_LOG_DEBUG("Call 'action_interrupt'");
}

void GenericDaemon::action_lifesign(const LifeSignEvent& e)
{
	SDPA_LOG_DEBUG("Call 'action_lifesign'");
    /*
    o timestamp, load, other probably useful information
    o last_job_id the id of the last received job identification
    o the aggregator first sends a request for configuration to its orchestrator
    o the orchestrator allocates an internal data structure to keep track of the state of the aggregator
    o this datastructure is being updated everytime a message is received
	o an aggregator is supposed to be unavailable when no messages have been received for a (configurable) period of time
     */
	Worker::worker_id_t worker_id = e.from();
	try {
		Worker::ptr_t ptrWorker = findWorker(worker_id);
		ptrWorker->update(e);
		SDPA_LOG_DEBUG("Received LS from the worker "<<worker_id<<" Updated the time-stamp");
	}
	catch(WorkerNotFoundException&)
	{
		SDPA_LOG_ERROR("Worker "<<worker_id<<" not found!");
		// the worker should register first, before posting a job request
		ErrorEvent::Ptr pErrorEvt(new ErrorEvent(name(), e.from(), ErrorEvent::SDPA_EWORKERNOTREG) );

		sendEvent(ptr_to_slave_stage_, pErrorEvt);
	} catch(...) {
		SDPA_LOG_DEBUG("Unexpected exception occurred!");
	}
}

void GenericDaemon::action_delete_job(const DeleteJobEvent& e )
{
	ostringstream os;
	os<<"Call 'action_delete_job'";
	SDPA_LOG_DEBUG(os.str());

	os.str("");
	os<<"received DeleteJobEvent from "<<e.from()<<" addressed to "<<e.to();
	SDPA_LOG_DEBUG(os.str());

	try{
		Job::ptr_t pJob = ptr_job_man_->findJob(e.job_id());
		pJob->DeleteJob(&e);

		ptr_job_man_->deleteJob(e.job_id());
	} catch(JobNotFoundException&){
		os.str("");
		os<<"Job "<<e.job_id()<<" not found!";
		SDPA_LOG_DEBUG(os.str());
	} catch(JobNotMarkedException& ){
		os.str("");
		os<<"Job "<<e.job_id()<<" not marked for deletion!";
		SDPA_LOG_DEBUG(os.str());
	}catch(JobNotDeletedException& ){
		os.str("");
		os<<"Job "<<e.job_id()<<" not deleted!";
		SDPA_LOG_DEBUG(os.str());
	} catch(...) {
		os.str("");
		os<<"Unexpected exception. Most probably the job to be deleted was not in a final state!"<<e.job_id()<<"!";
		SDPA_LOG_DEBUG(os.str());
	}
}

void GenericDaemon::action_request_job(const RequestJobEvent& e)
{
	SDPA_LOG_DEBUG("got job request from: " << e.from());
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
		Worker::ptr_t ptrWorker = findWorker(worker_id);
		ptrWorker->update(e);

		// you should consume from the  worker's pending list; put the job into the worker's submitted list
		Job::ptr_t ptrJob = ptrWorker->get_next_job(e.last_job_id());
		if( ptrJob.get() )
		{
			// put the job into the Runnig state here
			ptrJob->Dispatch(); // no event need to be sent

			// create a SubmitJobEvent for the job job_id serialize and attach description
			SDPA_LOG_DEBUG("sending SubmitJobEvent (jid=" << ptrJob->id() << ") to: " << e.from());
			SubmitJobEvent::Ptr pSubmitEvt(new SubmitJobEvent(name(), e.from(), ptrJob->id(),  ptrJob->description()));

			//inform GWES
			gwes::activity_id_t actId = ptrJob->id().str();
			gwes::workflow_id_t wfId  = ptrJob->parent().str();

			SDPA_LOG_DEBUG("Call activityDispatched( "<<wfId<<", "<<actId<<" )");
			ptr_Sdpa2Gwes_->activityDispatched( wfId, actId );

			// Post a SubmitJobEvent to the slave who made the request
			sendEvent(ptr_to_slave_stage_, pSubmitEvt);
		}
		else // send an error event
		{
			SDPA_LOG_DEBUG("no job available, get_next_job should probably throw an exception?");
		}
	}
	catch(const NoJobScheduledException&)
	{
		SDPA_LOG_DEBUG("No job was scheduled to be executed on the worker '"<<worker_id);
	}
	catch(const WorkerNotFoundException&)
	{
		SDPA_LOG_DEBUG("worker " << worker_id << " is not registered, asking him to do so first");

		// the worker should register first, before posting a job request
		ErrorEvent::Ptr pErrorEvt(new ErrorEvent(name(), e.from(), ErrorEvent::SDPA_EWORKERNOTREG) );

		sendEvent(ptr_to_slave_stage_, pErrorEvt);
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
	ostringstream os;
	SDPA_LOG_DEBUG("Call 'action_submit_job' FROM "<<e.from());
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

	JobId job_id, job_id_empty(""); //already assigns an unique job_id (i.e. the constructor calls the generator)
	if(e.job_id() != job_id_empty)  //use the job_id already  assigned by the master
		job_id = e.job_id();        //the orchestrator will assign a new job_id for the user jobs, the Agg/NRE will use the job_id assigned by the master

	try {
		// First, parse the workflow in order to be able to create a valid job
		// The job_id is the corresponding activity_id
		// if the event comes from Gwes parent_id is the owner_workflow_id
		Job::ptr_t pJob( new JobFSM( job_id, e.description(), this, e.parent_id() ));

		// the job job_id is in the Pending state now!
		ptr_job_man_->addJob(job_id, pJob);

		// check if the message comes from outside/slave or from WFE
		// if it comes from outside set it as local
		if(e.from() != sdpa::daemon::GWES ) //e.to())
			pJob->set_local(true);

		ptr_scheduler_->schedule(pJob);

		if(pJob->is_local())
		{
			//send back to the user a SubmitJobAckEvent
			SubmitJobAckEvent::Ptr pSubmitJobAckEvt(new SubmitJobAckEvent(name(), e.from(), job_id));

			// There is a problem with this if uncommented
			sendEvent(ptr_to_master_stage_, pSubmitJobAckEvt);
		}
		//catch also workflow exceptions
	}catch(JobNotAddedException&) {
		os.str("");
		os<<"Job "<<job_id<<" could not be added!";
		SDPA_LOG_DEBUG(os.str());
		//send back an ErrorEvent
	}
	catch(QueueFull&)
	{
		SDPA_LOG_DEBUG("Failed to send to the master output stage "<<ptr_to_master_stage_->name()<<" a SubmitJobAckEvt for the job "<<job_id);
	}
	catch(seda::StageNotFound&)
	{
		SDPA_LOG_DEBUG("Stage not found when trying to submit SubmitJobAckEvt for the job "<<job_id);
	}
	catch(...) {
		SDPA_LOG_DEBUG("Unexpected exception occured when calling 'action_submit_job' for the job "<<job_id<<"!");
		//send back an ErrorEvent
	}
}

void GenericDaemon::action_config_request(const ConfigRequestEvent& e)
{
	ostringstream os;
	os<<"Call 'action_configure_request'";
	SDPA_LOG_DEBUG(os.str());
	/*
	 * on startup the aggregator tries to retrieve a configuration from its orchestrator
	 * post ConfigReplyEvent/message that contains the configuration data for the requesting aggregator
     * TODO: what is contained into the Configuration?
	 */

	ConfigReplyEvent::Ptr pCfgReplyEvt( new ConfigReplyEvent( name(), e.from()) );
	sendEvent(ptr_to_slave_stage_, pCfgReplyEvt);
}

void GenericDaemon::action_register_worker(const WorkerRegistrationEvent& evtRegWorker)
{
	try {
		Worker::ptr_t pWorker(new Worker(evtRegWorker.from()));
		addWorker(pWorker);

		SDPA_LOG_INFO("Registered the worker "<<pWorker->name());
		// send back an acknowledgment
		WorkerRegistrationAckEvent::Ptr pWorkerRegAckEvt(new WorkerRegistrationAckEvent(name(), evtRegWorker.from()));
		sendEvent(ptr_to_slave_stage_, pWorkerRegAckEvt);
	}
	catch(const QueueFull&)
	{
		SDPA_LOG_FATAL("could not send WorkerRegistrationAck: queue is full, this should never happen!");
	}
	catch(const seda::StageNotFound& snf)
	{
		SDPA_LOG_FATAL("could not send WorkerRegistrationAck: locate slave-stage failed: " << snf.what());
	}
}

void GenericDaemon::action_error_event(const sdpa::events::ErrorEvent &error)
{
  switch (error.error_code())
  {
	case ErrorEvent::SDPA_ENOERROR:
	{
	  // everything is fine, nothing to do
	  break;
	}
	case ErrorEvent::SDPA_EWORKERNOTREG:
	{
	  MLOG(INFO, "my master forgot me and asked me to register again, sending WorkerRegistrationEvent");
	  WorkerRegistrationEvent::Ptr pWorkerRegEvt(new WorkerRegistrationEvent(name(), error.from()));
	  sendEvent(ptr_to_master_stage_, pWorkerRegEvt);
	  break;
	}
	default:
	  MLOG(INFO, "got an ErrorEvent back (ignoring it): code=" << error.error_code() << " reason=" << error.reason());
  }
}

/* Implements Gwes2Sdpa */
/**
 * Submit an atomic activity to the SDPA.
 * This method is to be called by the GWES in order to delegate
 * the execution of activities.
 * The SDPA will use the callback handler SdpaGwes in order
 * to notify the GWES about activity status transitions.
 */
gwes::activity_id_t GenericDaemon::submitActivity(gwes::activity_t &activity)
{
	// create new job with the job description = workflow (serialize it first)
	// set the parent_id to ?
	// add this job into the parent's job list (call parent_job->add_subjob( new job(workflow) ) )
	// schedule the new job to some worker
	// ATTENTION! Important assumption: the workflow_id should be set identical to the job_id!

	// simple generate a SubmitJobEvent cu from = to = name()
	// send an external job
	ostringstream os;

	try {
		SDPA_LOG_DEBUG("GWES submitted the activity "<<activity.getID());

		job_id_t job_id(activity.getID());

		// transform activity to workflow
		gwdl::Workflow::ptr_t pWf = activity.transform2Workflow();
		//SDPA_LOG_DEBUG("Transformed activity into an workflow: "<<(gwdl::Workflow&)(*pWf));

		// check if the generated workflow has the same id as the activity_id
		// if not, set explicitly
		if(activity.getID() != pWf->getID() )
		{
			SDPA_LOG_DEBUG("The transformed workflow does not have an id already set. Set this to the activity_id ...");
			pWf->setID(activity.getID());
		}

		// serialize workflow
		job_desc_t job_desc =  ptr_Sdpa2Gwes_->serializeWorkflow(*pWf);
		SDPA_LOG_DEBUG("activity_id = "<<activity.getID()<<", workflow_id = "<<pWf->getID());

		gwes::workflow_id_t parent_id = activity.getOwnerWorkflowID();

		SubmitJobEvent::Ptr pEvtSubmitJob(new SubmitJobEvent(sdpa::daemon::GWES, name(), job_id, job_desc, parent_id));
		sendEvent(pEvtSubmitJob);
	}
	catch(QueueFull&)
	{
		os.str("");
		os<<"Failed to send to the daemon stage a SubmitJobEvent";
		SDPA_LOG_DEBUG(os.str());
	}
	catch(seda::StageNotFound&)
	{
		os.str("");
		os<<"Stage not found when trying to submit SubmitJobEvent";
		SDPA_LOG_DEBUG(os.str());
	}
	catch(std::exception&)
	{
		SDPA_LOG_DEBUG("Either transform2Workflow or serializeWorkflow failed! Cancel the activity.");
		// inform immediately GWES that the corresponding activity was cancelled
		gwes::activity_id_t actId = activity.getID();
		gwes::workflow_id_t wfId  = activity.getOwnerWorkflowID();
		try
		{
		  gwes()->activityCanceled( wfId, actId );
		}
		catch (...)
		{
		  LOG(ERROR, "call to GWES failed");
		}
	}

	return activity.getID();
}


/**
 * Cancel an atomic activity that has previously been submitted to
 * the SDPA.
 */
void GenericDaemon::cancelActivity(const gwes::activity_id_t &activityId) throw (gwes::Gwes2Sdpa::NoSuchActivity)
{
	// cancel the job corresponding to that activity -> send downward a CancelJobEvent?
	// look for the job_id corresponding to the received workflowId into job_map_
	// in fact they should be the same!
	// generate const CancelJobEvent& event
	// Job& job = job_map_[job_id];
	// call job.CancelJob(event);

	SDPA_LOG_DEBUG("GWES asked SDPA to cancel the activity "<<activityId<<" ...");
	job_id_t job_id(activityId);
	CancelJobEvent::Ptr pEvtCancelJob(new CancelJobEvent(name(), name(), job_id));
	sendEvent(pEvtCancelJob);
}

/**
 * Notify the SDPA that a workflow finished (state transition
 * from running to finished).
 */
void GenericDaemon::workflowFinished(const gwes::workflow_id_t &workflowId, const gwdl::workflow_result_t& gwes_result) throw (gwes::Gwes2Sdpa::NoSuchWorkflow)
{
	// generate a JobFinishedEvent for self!
	// cancel the job corresponding to that activity -> send downward a CancelJobEvent?
	// look for the job_id corresponding to the received workflowId into job_map_
	// in fact they should be the same!
	// generate const JobFinishedEvent& event
	// Job& job = job_map_[job_id];
	// call job.JobFinished(event);

	SDPA_LOG_DEBUG("GWES notified SDPA that the workflow "<<workflowId<<" finished!");
	job_id_t job_id(workflowId);

	sdpa::job_result_t sdpa_result(sdpa::wf::glue::wrap(gwes_result)); //convert it from gwes_result;
	JobFinishedEvent::Ptr pEvtJobFinished(new JobFinishedEvent(sdpa::daemon::GWES, name(), job_id, sdpa_result));
	sendEvent(pEvtJobFinished);

	// deallocate the results
	gwdl::deallocate_workflow_result(const_cast<gwdl::workflow_result_t&>(gwes_result));

	DMLOG(TRACE, "telling GWES to remove Workflow: " << workflowId);
	sendDeleteEvent(workflowId);
}

/**
 * Notify the SDPA that a workflow failed (state transition
 * from running to failed).
 */
void GenericDaemon::workflowFailed(const gwes::workflow_id_t &workflowId, const gwdl::workflow_result_t& gwes_result) throw (gwes::Gwes2Sdpa::NoSuchWorkflow)
{
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

	SDPA_LOG_DEBUG("GWES notified SDPA that the workflow "<<workflowId<<" failed!");
	job_id_t job_id(workflowId);

	sdpa::job_result_t sdpa_result(sdpa::wf::glue::wrap(gwes_result)); //convert it from gwes_result;
	JobFailedEvent::Ptr pEvtJobFailed( new JobFailedEvent(sdpa::daemon::GWES, name(), job_id, sdpa_result ));
	sendEvent(pEvtJobFailed);

	// deallocate the results
	gwdl::deallocate_workflow_result(const_cast<gwdl::workflow_result_t&>(gwes_result));

	DMLOG(TRACE, "telling GWES to remove Workflow: " << workflowId);
	sendDeleteEvent(workflowId);
}

/**
 * Notify the SDPA that a workflow has been canceled (state
 * transition from * to terminated.
 */
void GenericDaemon::workflowCanceled(const gwes::workflow_id_t &workflowId, const gwdl::workflow_result_t& gwes_result) throw (gwes::Gwes2Sdpa::NoSuchWorkflow)
{
	// generate a JobCancelledEvent for self!
	// identify the job with the job_id == workflow_id_t
	// trigger a CancelJobAck for that job

	SDPA_LOG_DEBUG("GWES notified SDPA that the workflow "<<workflowId<<" was cancelled!");
	job_id_t job_id(workflowId);

	sdpa::job_result_t sdpa_result(sdpa::wf::glue::wrap(gwes_result)); //convert it from gwes_result;
	CancelJobAckEvent::Ptr pEvtCancelJobAck(new CancelJobAckEvent(sdpa::daemon::GWES, name(), job_id));
	sendEvent(pEvtCancelJobAck);

	// deallocate the results
	gwdl::deallocate_workflow_result(const_cast<gwdl::workflow_result_t&>(gwes_result));

	DMLOG(TRACE, "telling GWES to remove Workflow: " << workflowId);
	sendDeleteEvent(workflowId);
}


void GenericDaemon::jobFinished(std::string workerName, const job_id_t& jobID )
{
	sdpa::job_result_t results;
	JobFinishedEvent::Ptr pJobFinEvt( new JobFinishedEvent( workerName, name(), jobID.str(), results ) );
	sendEvent(pJobFinEvt);
}

void GenericDaemon::jobFailed(std::string workerName, const job_id_t& jobID)
{
	sdpa::job_result_t results;
	JobFailedEvent::Ptr pJobFailEvt( new JobFailedEvent( workerName, name(), jobID.str(), results ) );
	sendEvent(pJobFailEvt);
}

void GenericDaemon::jobCancelled(std::string workerName, const job_id_t& jobID)
{
	CancelJobAckEvent::Ptr pCancelAckEvt( new CancelJobAckEvent( workerName, name(), jobID.str() ) );
	sendEvent(pCancelAckEvt);
}

