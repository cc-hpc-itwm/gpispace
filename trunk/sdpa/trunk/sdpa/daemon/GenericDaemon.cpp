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
#include <sdpa/events/CodecStrategy.hpp>

#include <sdpa/daemon/GenericDaemon.hpp>
#include <sdpa/daemon/JobImpl.hpp>
#include <sdpa/daemon/jobFSM/JobFSM.hpp>
#include <sdpa/events/ConfigReplyEvent.hpp>
#include <sdpa/events/StartUpEvent.hpp>
#include <sdpa/events/ConfigOkEvent.hpp>

#include <sdpa/com/NetworkStrategy.hpp>
#include <sdpa/events/id_generator.hpp>
#include <sdpa/daemon/exceptions.hpp>

#include <sdpa/daemon/jobFSM/JobFSM.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/tokenizer.hpp>

using namespace std;
using namespace sdpa::daemon;
using namespace sdpa::events;

// constructor for (obsolete) test cases without network
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
	  //ptr_daemon_stage_(NULL),
	  master_(""),
	  m_bRegistered(false),
	  m_nRank(0),
	  m_strAgentUID(id_generator::instance().next()),
	  m_nExternalJobs(0),
	  m_bRequestsAllowed(false),
	  m_bStopped(false),
	  m_bStarted(false),
	  m_bConfigOk(false)
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
	  //ptr_daemon_stage_(NULL),
	  master_(""),
	  m_bRegistered(false),
	  m_nRank(0),
	  m_strAgentUID(id_generator::instance().next()),
	  m_nExternalJobs(0),
	  m_to_master_stage_name_(toMasterStageName),
	  m_to_slave_stage_name_(toSlaveStageName),
	  m_bRequestsAllowed(false),
	  m_bStopped(false),
	  m_bStarted(false),
	  m_bConfigOk(false)
{
	if(!toMasterStageName.empty())
	{
		seda::Stage::Ptr pshToMasterStage = seda::StageRegistry::instance().lookup(toMasterStageName);
		ptr_to_master_stage_ = pshToMasterStage;
	}

	if(!toSlaveStageName.empty())
	{
		seda::Stage::Ptr pshToSlaveStage = seda::StageRegistry::instance().lookup(toSlaveStageName);
		ptr_to_slave_stage_ = pshToSlaveStage;
	}
}

// current constructor
// with network scommunication
GenericDaemon::GenericDaemon( const std::string name, IWorkflowEngine*  pArgSdpa2Gwes)
	: Strategy(name),
	  SDPA_INIT_LOGGER(name),
	  ptr_job_man_(new JobManager()),
	  ptr_scheduler_(),
	  ptr_workflow_engine_(pArgSdpa2Gwes),
	  //ptr_daemon_stage_(NULL),
	  master_(""),
	  m_bRegistered(false),
	  m_nRank(0),
	  m_strAgentUID(id_generator::instance().next()),
	  m_nExternalJobs(0),
	  m_to_master_stage_name_(name+".net"),
	  m_to_slave_stage_name_ (name+".net"),
	  m_bRequestsAllowed(false),
	  m_bStopped(false),
	  m_bStarted(false),
	  m_bConfigOk(false)
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
	SDPA_LOG_DEBUG("GenericDaemon destructor called ...");
}

void GenericDaemon::start()
{
	// create configuration
	ptr_daemon_cfg_ = sdpa::util::Config::create();

	// The stage uses 2 threads
	ptr_daemon_stage_.lock()->start();

	//start-up the the daemon
	StartUpEvent::Ptr pEvtStartUp(new StartUpEvent(name(), name()));
	sendEventToSelf(pEvtStartUp);

	lock_type lock(mtx_);
	while(!m_bStarted)
		cond_can_start_.wait(lock);

	if(!m_bConfigOk)
	{
		SDPA_LOG_DEBUG("Could not configure "<<name()<<". Giving up now!");
	}
	else // can register now
	{
		if( !is_orchestrator() )
		{
			SDPA_LOG_INFO("Agent (" << name() << ") is sending a registration event to master (" << master() << ") now ...");
			WorkerRegistrationEvent::Ptr pEvtWorkerReg(new WorkerRegistrationEvent(name(), master(), rank(), agent_uuid()));
			sendEventToMaster(pEvtWorkerReg);
		}
	}
}

// TODO: work here
// the configure_network should get a config structure
// the peer needs the following: address to bind to, port to use (0 by default)
// name of the master is not required, actually, the master can be stored in the kvs...

// Remarks bind_addr it's hostname or IP address or IPv6
//void GenericDaemon::configure_network( const std::string& bind_addr, const std::string& bind_port )
void GenericDaemon::configure_network( const std::string& daemonUrl, const std::string& masterName )
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
										   )
			);

		  seda::Stage::Ptr network_stage (new seda::Stage(m_to_master_stage_name_, net));
		  seda::StageRegistry::instance().insert (network_stage);
		  //network_stage->start ();

		  ptr_to_master_stage_ = ptr_to_slave_stage_ = network_stage;

		  if (! masterName.empty())
			setMaster(masterName);
		}
		catch (std::exception const &ex)
		{
		  LOG(ERROR, "could not configure network component: " << ex.what());
		  throw;
		}
    }
}

void GenericDaemon::shutdown_network()
{
    if( !master().empty() && is_registered())
    	sendEventToMaster (ErrorEvent::Ptr(new ErrorEvent(name(), master(), ErrorEvent::SDPA_ENODE_SHUTDOWN, "node shutdown")));
}

void GenericDaemon::shutdown()
{
	// should first notify my master
	// that I will be shutdown

	if(!m_bStopped)
		stop();

	SDPA_LOG_INFO("Remove the stages...");
	// remove the network stage
	seda::StageRegistry::instance().remove(m_to_master_stage_name_);

	// remove the daemon stage
	seda::StageRegistry::instance().remove(name());

	if ( hasWorkflowEngine() )
	{
		SDPA_LOG_DEBUG("Delete the workflow engine ...");
		delete ptr_workflow_engine_;
		ptr_workflow_engine_ = NULL;
	}
}

void GenericDaemon::stop()
{
	// here one should only generate a message of type interrupt
	SDPA_LOG_DEBUG("Send to self an InterruptEvent...");
	InterruptEvent::Ptr pEvtInterrupt(new InterruptEvent(name(), name()));
	sendEventToSelf(pEvtInterrupt);

	// wait to be stopped
	{
		lock_type lock(mtx_);
		while(!m_bStopped)
			cond_can_stop_.wait(lock);
	}

	SDPA_LOG_INFO("Shutting down...");

	SDPA_LOG_INFO("Stop the scheduler now!");
	scheduler()->stop();

	SDPA_LOG_INFO("Shutdown the network...");
	shutdown_network();

	// shutdown the daemon stage
	SDPA_LOG_DEBUG("shutdown the daemon stage "<<name());
	seda::StageRegistry::instance().lookup(name())->stop();

	//  shutdown the peer and remove the information from kvs
	SDPA_LOG_DEBUG("shutdown the network stage "<<m_to_master_stage_name_);
	seda::StageRegistry::instance().lookup(m_to_master_stage_name_)->stop();
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

//actions
void GenericDaemon::action_configure(const StartUpEvent&)
{
	// should be overriden by the orchestrator, aggregator and NRE
	SDPA_LOG_INFO("Configuring myself (generic)...");

	// use for now as below, later read from config file
	// TODO: move this to "property" style:
	//    dot separated
	//    hierarchies / categories
	//    retrieve values maybe from kvs?
	//    no spaces

	// Read these from a configuration file !!!!!!!!
	// if this does not exist, use default values
	ptr_daemon_cfg_->put("polling interval",    			1 * 1000 * 1000);
	ptr_daemon_cfg_->put("upper bound polling interval", 	5 * 1000 * 1000 );
	ptr_daemon_cfg_->put("life-sign interval",  			2 * 1000 * 1000);
	ptr_daemon_cfg_->put("node_timeout",        			6 * 1000 * 1000); // 6s
	ptr_daemon_cfg_->put("registration_timeout", 			2 * 1000 * 1000); // 2s

	m_ullPollingInterval = cfg()->get<sdpa::util::time_type>("polling interval");

	try {
		configure_network( url(), masterName() );
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
	SDPA_LOG_INFO("The configuration phase succeeded!");
	m_bRequestsAllowed = true;
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
	m_bRequestsAllowed = false;
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
			sendEventToSlave(pSubmitEvt);
		}
		else // send an error event
		{
			SDPA_LOG_DEBUG("no job available, get_next_job should probably throw an exception?");
		}
	}
	catch(const NoJobScheduledException&)
	{
		//SDPA_LOG_DEBUG("No job was scheduled to be executed on the worker '"<<worker_id);
	}
	catch(const WorkerNotFoundException&)
	{
		SDPA_LOG_INFO("The worker " << worker_id << " is not registered! Sending him a notification ...");

		// the worker should register first, before posting a job request
		ErrorEvent::Ptr pErrorEvt(new ErrorEvent(name(), e.from(), ErrorEvent::SDPA_EWORKERNOTREG, "not registered") );

		sendEventToSlave(pErrorEvt);
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
		// The job already exists -> generate an error message that the job already exists
		DLOG(TRACE, "The job with job-id: " << e.job_id()<<" exist already into the JobManager. Reply to "<< e.from()<<" with an error!");

		if( e.from() != sdpa::daemon::WE ) //e.to())
		{
			ErrorEvent::Ptr pErrorEvt(new ErrorEvent(name(), e.from(), ErrorEvent::SDPA_EUNKNOWN, "The job already exists!") );
			sendEventToMaster(pErrorEvt);
		}

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
		if( e.from() != sdpa::daemon::WE && hasWorkflowEngine() ) //e.to())
		{
			LOG(INFO, "got new job from " << e.from() << " = " << job_id);
			pJob->set_local(true);
		}

		ptr_scheduler_->schedule(job_id);

		if( e.from() != sdpa::daemon::WE )
		{
			//send back to the user a SubmitJobAckEvent
			SubmitJobAckEvent::Ptr pSubmitJobAckEvt(new SubmitJobAckEvent(name(), e.from(), job_id, e.id()));

			// There is a problem with this if uncommented
			sendEventToMaster(pSubmitJobAckEvt);

			if( !master_.empty() )
				incExtJobsCnt();

		}
		//catch also workflow exceptions
	}catch(JobNotAddedException const &ex)
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

        LOG(TRACE, "Trying to register new worker " << worker_id << ", with the rank " << rank);

		addWorker( worker_id, rank, evtRegWorker.agent_uuid() );

		SDPA_LOG_INFO( "Registered the worker " << worker_id << ", with the rank " << rank);

		// send back an acknowledgment
		SDPA_LOG_INFO( "Send back to the worker " << worker_id << " a registration acknowledgment!" );
		WorkerRegistrationAckEvent::Ptr pWorkerRegAckEvt(new WorkerRegistrationAckEvent(name(), evtRegWorker.from()));
		pWorkerRegAckEvt->id() = evtRegWorker.id();
		sendEventToSlave(pWorkerRegAckEvt);
	}
	catch(WorkerAlreadyExistException& ex)
	{
		if( evtRegWorker.agent_uuid() != ex.agent_uuid() )
		{
			LOG(TRACE, "The worker manager already contains an worker with the same id or rank (id="<<ex.worker_id()<<", rank="<<ex.rank()<<"), but with a different agent_uuid!" );
			LOG(TRACE, "Re-schedule the jobs");
			scheduler()->re_schedule( worker_id );
			LOG(TRACE,"Delete worker "<<worker_id);
			scheduler()->delWorker(worker_id);
			LOG(TRACE, "Add worker"<<worker_id );
			addWorker( worker_id, rank, evtRegWorker.agent_uuid() );

			LOG(TRACE, "Registered the worker " << worker_id << ", with the rank " << rank);

			// send back an acknowledgment
			LOG(TRACE,"Send registration ack to the agent " << worker_id << ", with the rank " << rank);
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
			MLOG(WARN, "New instance of the master is up, sending new registration request!");
			WorkerRegistrationEvent::Ptr pWorkerRegEvt(new WorkerRegistrationEvent(name(), error.from(), rank(), m_strAgentUID));
			sendEventToMaster(pWorkerRegEvt);
			break;
		}
		case ErrorEvent::SDPA_ENODE_SHUTDOWN:
		{
			try
			{
				worker_id_t worker_id(error.from());
				findWorker(worker_id);

				MLOG(INFO, "worker " << worker_id << " went down (clean). Tell the WorkerManager to remove it!");
				ptr_scheduler_->delWorker(worker_id); // do a re-scheduling here
			}
			catch (WorkerNotFoundException const& /*ignored*/)
			{
				if( !is_orchestrator() && error.from() == master() )
				{
					SDPA_LOG_WARN("Master " << master() << " is down");
					m_bRegistered = false;
					const unsigned long reg_timeout( cfg()->get<unsigned long>("registration_timeout", 10 *1000*1000) );
					SDPA_LOG_INFO("Wait " << reg_timeout/1000000 << "s before trying to re-register ...");
					usleep(reg_timeout);

					// try to re-register
					SDPA_LOG_INFO("Agent (" << name() << ") is sending a registration event to master (" << master() << ") now ...");
					WorkerRegistrationEvent::Ptr pEvtWorkerReg(new WorkerRegistrationEvent( name(), master(), rank(), agent_uuid()));
					sendEventToMaster(pEvtWorkerReg);
				}
			}
			catch (std::exception const& ex)
			{
				LOG(ERROR, "STRANGE! something went wrong during worker-lookup (" << error.from() << "): " << ex.what ());
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
void GenericDaemon::submit(const id_type& activityId, const encoded_type& desc/*, preference_t pref*/ )
{
	// create new job with the job description = workflow (serialize it first)
	// set the parent_id to ?
	// add this job into the parent's job list (call parent_job->add_subjob( new job(workflow) ) )
	// schedule the new job to some worker

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

void GenericDaemon::handleWorkerRegistrationAckEvent(const sdpa::events::WorkerRegistrationAckEvent* pRegAckEvt)
{
	LOG(TRACE, "Received WorkerRegistrationAckEvent from "<<pRegAckEvt->from());
    acknowledge (pRegAckEvt->id());
	m_bRegistered = true;
}

void GenericDaemon::handleConfigReplyEvent(const sdpa::events::ConfigReplyEvent* pCfgReplyEvt)
{
	SDPA_LOG_DEBUG("Received ConfigReplyEvent from "<<pCfgReplyEvt->from());
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

bool GenericDaemon::acknowledge(const sdpa::events::SDPAEvent::message_id_type &mid)
{
  return true;
}

Worker::ptr_t const & GenericDaemon::findWorker(const Worker::worker_id_t& worker_id ) throw(WorkerNotFoundException)
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

void GenericDaemon::addWorker( const Worker::worker_id_t& workerId, unsigned int rank, const sdpa::worker_id_t& agent_uuid ) throw (WorkerAlreadyExistException)
{
	try {
		ptr_scheduler_->addWorker(workerId, rank, agent_uuid);
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
	if(!m_bRequestsAllowed)
		return false;

	if( extJobsCnt() == 0 && m_ullPollingInterval < ptr_daemon_cfg_->get<unsigned int>("upper bound polling interval") )
		m_ullPollingInterval  = m_ullPollingInterval + 10000;

	return (difftime>m_ullPollingInterval) &&
		   (m_nExternalJobs<cfg()->get<unsigned int>("nmax_ext_job_req"));
}

void GenericDaemon::workerJobFailed(const job_id_t& jobId, const std::string& reason)
{
	if( hasWorkflowEngine() )
	{
		DLOG(TRACE, "informing workflow engine that " << jobId << " has failed: " << reason);
		workflowEngine()->failed( jobId.str(), reason );
		jobManager()->deleteJob(jobId);
	}
	else
	{
		DLOG(TRACE, "Sent JobFailedEvent to self for the job"<<jobId);
		JobFailedEvent::Ptr pEvtJobFailed( new JobFailedEvent(name(), name(), jobId, reason ));
		sendEventToSelf(pEvtJobFailed);
	}
}

void GenericDaemon::workerJobFinished(const job_id_t& jobId, const result_type & result)
{
	if( hasWorkflowEngine() )
	{
		DLOG(TRACE, "informing workflow engine that " << jobId << " has finished");
		workflowEngine()->finished( jobId.str(), result );
		jobManager()->deleteJob(jobId);
	}
	else
	{
		DLOG(TRACE, "Sent JobFinishedEvent to self for the job"<<jobId);
		JobFinishedEvent::Ptr pEvtJobFinished(new JobFinishedEvent(name(), name(), jobId, result));
		sendEventToSelf(pEvtJobFinished);
	}
}

void GenericDaemon::workerJobCancelled(const job_id_t& jobId)
{
	if( hasWorkflowEngine() )
	{
		DLOG(TRACE, "informing workflow engine that " << jobId << " has been cancelled");
		workflowEngine()->cancelled( jobId.str() );
		jobManager()->deleteJob(jobId);
	}
	else
	{
		DLOG(TRACE, "Sent CancelJobAckEvent to self for the job"<<jobId);
		CancelJobAckEvent::Ptr pEvtCancelJobAck(new CancelJobAckEvent(name(), name(), jobId, SDPAEvent::message_id_type()));
		sendEventToSelf(pEvtCancelJobAck);
	}
}
