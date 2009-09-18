#include <seda/StageRegistry.hpp>

#include <sdpa/daemon/GenericDaemon.hpp>
#include <sdpa/daemon/JobImpl.hpp>
#include <sdpa/daemon/jobFSM/SMC/JobFSM.hpp>
//#include <sdpa/daemon/jobFSM/BSC/JobFSM.hpp>
#include <sdpa/uuid.hpp>
#include <sdpa/uuidgen.hpp>
#include <sstream>
#include <map>

#include <sdpa/daemon/exceptions.hpp>
#include <sdpa/wf/types.hpp>
//#include <sdpa/wf/WorkflowInterface.hpp>

using namespace std;
using namespace sdpa::daemon;
using namespace sdpa::wf;
using namespace sdpa::events;

//Provide ptr to an implementation of Sdpa2Gwes
GenericDaemon::GenericDaemon(const std::string &name, const std::string &outputStage, Sdpa2Gwes*  pArgSdpa2Gwes)
	: Strategy(name), output_stage_(outputStage), SDPA_INIT_LOGGER(name),
	ptr_job_man_(new JobManager()), ptr_scheduler_(new SchedulerImpl(pArgSdpa2Gwes))
{
	// provide some dummy implementation (for testing) or use the real impl. from the gwes library
	ptr_Sdpa2Gwes_ = pArgSdpa2Gwes;

	if(pArgSdpa2Gwes)
		pArgSdpa2Gwes->registerHandler(this);
}

GenericDaemon::~GenericDaemon()
{
	ostringstream os;
	os<<"GenericDaemon destructor called ...";
	SDPA_LOG_DEBUG(os.str());

	// stop the scheduler

	//daemon_stage_->stop();
	//seda::StageRegistry::instance().lookup(name())->stop();
}

GenericDaemon::ptr_t GenericDaemon::create(const std::string &name_prefix,  const std::string &outputStage, Sdpa2Gwes*  pArgSdpa2Gwes )
{
	// warning: we introduce a cycle here, we have to resolve it during shutdown!
	GenericDaemon::ptr_t daemon( new GenericDaemon(name_prefix, outputStage, pArgSdpa2Gwes) );
	seda::Stage::Ptr daemon_stage( new seda::Stage(name_prefix + "", daemon) );
	daemon->setStage(daemon_stage.get());
	seda::StageRegistry::instance().insert(daemon_stage);
	daemon_stage->start();

	// start here the Scheduler thread as well
	return daemon;
}

void GenericDaemon::start(GenericDaemon::ptr_t ptr_daemon )
{
	seda::Stage::Ptr daemon_stage( new seda::Stage(ptr_daemon->name() + "", ptr_daemon) );
	ptr_daemon->setStage(daemon_stage.get());
	seda::StageRegistry::instance().insert(daemon_stage);
	daemon_stage->start();
}


void GenericDaemon::perform(const seda::IEvent::Ptr& pEvent)
{
	if(dynamic_cast<MgmtEvent*>(pEvent.get()))
		handleDaemonEvent(pEvent);
	else
		if(dynamic_cast<JobEvent*>(pEvent.get()))
		{
			if( dynamic_cast<JobFinishedEvent*>(pEvent.get()) ||
				dynamic_cast<JobFailedEvent*>(pEvent.get()) ||
				dynamic_cast<CancelJobAckEvent*>(pEvent.get()) ||
				dynamic_cast<SubmitJobEvent*>(pEvent.get())) handleDaemonEvent(pEvent);
			else
				handleJobEvent(pEvent);
		}
		else
			cout<<"Throw an exception here!"<<std::endl;
}

void GenericDaemon::handleDaemonEvent(const seda::IEvent::Ptr& pEvent)
{
	ostringstream os;
	os<<"Handle Management Event "<<typeid(*pEvent).name();
	SDPA_LOG_DEBUG(os.str());
}

void GenericDaemon::handleJobEvent(const seda::IEvent::Ptr& pEvent)
{
	// check what type of event is and call transitions
	if( QueryJobStatusEvent* ptr = dynamic_cast<QueryJobStatusEvent*>(pEvent.get()) )
	{
		try {
			Job::ptr_t pJob = ptr_job_man_->findJob(ptr->job_id());
			pJob->QueryJobStatus();
		}
		catch(JobNotFoundException)
		{
			ostringstream os;
		    os<<"The job "<<ptr->job_id()<<" was not found by the JobManager";
			SDPA_LOG_DEBUG(os.str());
		}
	}
	else if( CancelJobEvent* ptr = dynamic_cast<CancelJobEvent*>(pEvent.get()) )
	{
		try {
			Job::ptr_t pJob = ptr_job_man_->findJob(ptr->job_id());
			pJob->CancelJob();
		}
		catch(JobNotFoundException)
		{
			ostringstream os;
			os<<"The job "<<ptr->job_id()<<" was not found by the JobManager";
			SDPA_LOG_DEBUG(os.str());
		}
	}
	else if( RetrieveJobResultsEvent* ptr = dynamic_cast<RetrieveJobResultsEvent*>(pEvent.get()) )
	{
		try {
			Job::ptr_t pJob = ptr_job_man_->findJob(ptr->job_id());
			pJob->RetrieveJobResults();
		}
		catch(JobNotFoundException)
		{
			ostringstream os;
			os<<"The job "<<ptr->job_id()<<" was not found by the JobManager";
			SDPA_LOG_DEBUG(os.str());
		}
	}
}

void GenericDaemon::onStageStart(const std::string &stageName)
{
	// start the scheduler thread
	ptr_scheduler_->start();
}

void GenericDaemon::onStageStop(const std::string &stageName)
{
	daemon_stage_ = NULL;
	// stop the scheduler thread
	ptr_scheduler_->stop();
	//ptr_Sdpa2Gwes_ = NULL;

}

void GenericDaemon::sendEvent(const SDPAEvent::Ptr& pEvt)
{
	try {
		if(daemon_stage_)
			daemon_stage_->send(pEvt);
	}
	catch(QueueFull)
	{
		SDPA_LOG_DEBUG("Could not send event. The queue is full!");
	}

	ostringstream os;
	os<<"Sent " <<pEvt->str()<<" to "<<pEvt->to();
	SDPA_LOG_DEBUG(os.str());
}

void GenericDaemon::sendEvent(const std::string& stageName, const SDPAEvent::Ptr& pEvt)
{
	try {
		seda::Stage::send(stageName, pEvt);
	}
	catch(QueueFull)
	{
		SDPA_LOG_DEBUG("Could not send event. The queue is full!");
	}


	ostringstream os;
	os<<"Sent " <<pEvt->str()<<" to "<<pEvt->to();
	SDPA_LOG_DEBUG(os.str());
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
void GenericDaemon::action_configure(const StartUpEvent& e)
{
	ostringstream os;
	os<<"Call 'action_configure'";
	SDPA_LOG_DEBUG(os.str());
}

void GenericDaemon::action_config_ok(const ConfigOkEvent& e)
{
	ostringstream os;
	os<<"Call 'action_config_ok'";
	SDPA_LOG_DEBUG(os.str());
}

void GenericDaemon::action_config_nok(const ConfigNokEvent& e)
{
	ostringstream os;
	os<<"Call 'action_config_nok'";
	SDPA_LOG_DEBUG(os.str());
}

void GenericDaemon::action_interrupt(const InterruptEvent& e)
{
	ostringstream os;
	os<<"Call 'action_interrupt'";
	SDPA_LOG_DEBUG(os.str());
}

void GenericDaemon::action_lifesign(const LifeSignEvent& e)
{
	ostringstream os;
	os<<"Call 'action_lifesign'";
	SDPA_LOG_DEBUG(os.str());
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
		os.str("");
		os<<"Received LS from the worker "<<worker_id<<" Updated the time-stamp";
		SDPA_LOG_DEBUG(os.str());
	} catch(sdpa::daemon::WorkerNotFoundException) {
		os.str("");
		os<<"Worker "<<worker_id<<" not found!";
		SDPA_LOG_DEBUG(os.str());
	} catch(...) {
		os.str("");
		os<<"Unexpected exception occurred!";
		SDPA_LOG_DEBUG(os.str());
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
		pJob->process_event(e);

		if( pJob->is_marked_for_deletion() )
		{
			ptr_job_man_->markJobForDeletion(e.job_id(), pJob);
			ptr_job_man_->deleteJob(e.job_id());

			DeleteJobAckEvent::Ptr pDelAckEvt(new DeleteJobAckEvent(name(), e.from()));
			sendEvent(output_stage_, pDelAckEvt);
		}
	} catch(sdpa::daemon::JobNotFoundException){
		os.str("");
		os<<"Job "<<e.job_id()<<" not found!";
		SDPA_LOG_DEBUG(os.str());
	} catch(sdpa::daemon::JobNotMarkedException ){
		os.str("");
		os<<"Job "<<e.job_id()<<" not marked for deletion!";
		SDPA_LOG_DEBUG(os.str());
	}catch(sdpa::daemon::JobNotDeletedException ){
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
	ostringstream os;
	os<<"Call 'action_request_job'";
	SDPA_LOG_DEBUG(os.str());
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

		// you should consume from the  worker's pending list
		Job::ptr_t ptrJob = ptrWorker->get_next_job(e.last_job_id());
		if( ptrJob.use_count() )
		{
			ptrWorker->dispatch(ptrJob);  //the job was sent but not yet acknowledged

			// create a SubmitJobEvent for the job job_id serialize and attach description
			os.str("");
			os<<"Create SubmitJobEvent for the job "<<ptrJob->id()<<" (to be submitted to "<<e.from()<<"! )";
			SDPA_LOG_DEBUG(os.str());
			SubmitJobEvent::Ptr pSubmitEvt(new SubmitJobEvent(name(), e.from(), ptrJob->id(),  ptrJob->description()));

			// Post a SubmitJobEvent to the slave who made the reques
			sendEvent(output_stage_, pSubmitEvt);

			// put the job into the running state
			ptrJob->Dispatch();
		}
	}
	catch(NoJobScheduledException)
	{
		os.str("");
		os<<"No job was scheduled to be executed on the worker '"<<worker_id<<"'";
		SDPA_LOG_DEBUG(os.str());
	}
	catch(sdpa::daemon::WorkerNotFoundException)
	{	os.str("");
		os<<"Worker "<<worker_id<<" not found!";
		SDPA_LOG_DEBUG(os.str());
	}
	catch(QueueFull)
	{
		os.str("");
		os<<"Failed to send to the output stage "<<output_stage_<<" a SubmitJobEvent";
		SDPA_LOG_DEBUG(os.str());
	}
	catch(seda::StageNotFound)
	{
		os.str("");
		os<<"Stage not found when trying to send SubmitJobEvent";
		SDPA_LOG_DEBUG(os.str());
	}
	catch(...) {
	os.str("");
	os<<"Unexpected exception occurred!";
	SDPA_LOG_DEBUG(os.str());
	}
}

void GenericDaemon::action_submit_job(const SubmitJobEvent& e)
{
	ostringstream os;
	os<<"Call 'action_submit_job'";
	SDPA_LOG_DEBUG(os.str());
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
		Job::ptr_t pJob( new sdpa::fsm::smc::JobFSM( job_id, e.description(), this ));

		// the job job_id is in the Pending state now!
		ptr_job_man_->addJob(job_id, pJob);

		// check if the message comes from outside/slave or from WFE
		// if it comes from outside set it as local
		if(e.from() != name() ) //e.to())
			pJob->set_local(true);

		ptr_scheduler_->handleJob(pJob);

		if(pJob->is_local())
		{
			//send back to the user a SubmitJobAckEvent
			SubmitJobAckEvent::Ptr pSubmitJobAckEvt(new SubmitJobAckEvent(name(), e.from(), job_id));

			// There is a problem with this if uncommented
			sendEvent(output_stage_, pSubmitJobAckEvt);
		}
		//catch also workflow exceptions
	}catch(sdpa::daemon::JobNotAddedException) {
		os.str("");
		os<<"Job "<<job_id<<" could not be added!";
		SDPA_LOG_DEBUG(os.str());
		//send back an ErrorEvent
	}
	catch(QueueFull)
	{
		os.str("");
		os<<"Failed to send to the output stage "<<output_stage_<<" a SubmitJobAckEvt for the job "<<job_id;
		SDPA_LOG_DEBUG(os.str());
	}
	catch(seda::StageNotFound)
	{
		os.str("");
		os<<"Stage not found when trying to submit SubmitJobAckEvt for the job "<<job_id;
		SDPA_LOG_DEBUG(os.str());
	}
	catch(...) {
		os.str("");
		os<<"Unexpected exception occured when calling 'action_submit_job' for the job "<<job_id<<"!";
		SDPA_LOG_DEBUG(os.str());
		//send back an ErrorEvent
	}
}

void GenericDaemon::action_submit_job_ack(const SubmitJobAckEvent& e) {
	ostringstream os;
	os<<"Call 'action_request_job_ack'"<< std::endl;
	SDPA_LOG_DEBUG(os.str());

	// a slave posted an acknowledgement for a job request
	// put the job from pending into submitted
	// call worker :: acknowledge(const sdpa::job_id_t& job_id ) = ;
	Worker::worker_id_t worker_id = e.from();
	try {
		Worker::ptr_t ptrWorker = findWorker(worker_id);
		//put the job into the Running state: do this in acknowledge!
		ptrWorker->acknowledge(e.job_id());
	} catch(sdpa::daemon::WorkerNotFoundException) {
		os.str("");
		os<<"Worker "<<worker_id<<" not found!";
		SDPA_LOG_DEBUG(os.str());
	} catch(...) {
		os.str("");
		os<<"Unexpected exception occurred!";
		SDPA_LOG_DEBUG(os.str());
	}
}

void GenericDaemon::action_config_request(const ConfigRequestEvent& e) {
	ostringstream os;
	os<<"Call 'action_configure'";
	SDPA_LOG_DEBUG(os.str());
	/*
	 * on startup the aggregator tries to retrieve a configuration from its orchestrator
	 * post ConfigReplyEvent/message that contains the configuration data for the requesting aggregator
     * TODO: what is contained in the Configuration?
	 */
}

void GenericDaemon::action_job_finished(const JobFinishedEvent& )
{
	// check if the message comes from outside/slave or from WFE
	// if it comes from a slave, one should inform WFE -> subjob
	// if it comes from WFE -> concerns the master job
}

void GenericDaemon::action_job_failed(const JobFailedEvent& )
{
	// check if the message comes from outside/slave or from WFE
	// if it comes from a slave, one should inform WFE -> subjob
	// if it comes from WFE -> concerns the master job
}

void GenericDaemon::action_job_canceled(const CancelJobAckEvent& )
{
	// check if the message comes from outside/slave or from WFE
	// if it comes from a slave, one should inform WFE -> subjob
	// if it comes from WFE -> concerns the master job
}

void GenericDaemon::action_register_worker(const WorkerRegistrationEvent& evtRegWorker)
{
	ostringstream os;
	try {
		Worker::ptr_t pWorker(new Worker(evtRegWorker.from()));
		addWorker(pWorker);

		ostringstream os;
		os<<"Registered the worker "<<pWorker->name()<<" ...";
		SDPA_LOG_DEBUG(os.str());

		os.str("");
		os<<"Send back registration acknowledgement to the worker "<<pWorker->name()<<" ...";
		SDPA_LOG_DEBUG(os.str());
		// send back an acknowledgement
		WorkerRegistrationAckEvent::Ptr pWorkerRegAckEvt(new WorkerRegistrationAckEvent(name(), evtRegWorker.from()));
		sendEvent(output_stage_, pWorkerRegAckEvt);
	}
	catch(QueueFull)
	{
		os.str("");
		os<<"Failed to send to the output stage "<<output_stage_<<" a WorkerRegistrationEvent";
		SDPA_LOG_DEBUG(os.str());
	}
	catch(seda::StageNotFound)
	{
		os.str("");
		os<<"Stage not found when trying to submit WorkerRegistrationEvent";
		SDPA_LOG_DEBUG(os.str());
	}
}

/* Implements Gwes2Sdpa */

/**
 * Submit a sub workflow to the SDPA.
 * This method is to be called by the GWES in order to delegate
 * the execution of sub workflows.
 * The SDPA will use the callback handler SdpaGwes in order
 * to notify the GWES about status transitions.
*/
workflow_id_t GenericDaemon::submitWorkflow(const workflow_t &workflow)
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
		SDPA_LOG_DEBUG("New workflow submitted by GWES ...");

		job_id_t job_id(workflow.getId());
		job_desc_t job_desc = workflow.serialize();

		SubmitJobEvent::Ptr pEvtSubmitJob(new SubmitJobEvent(name(), name(), job_id, job_desc));
		sendEvent(pEvtSubmitJob);

		return workflow.getId();
	}
	catch(QueueFull)
		{
			os.str("");
			os<<"Failed to send to the output stage "<<output_stage_<<" a SubmitJobEvent";
			SDPA_LOG_DEBUG(os.str());
		}
		catch(seda::StageNotFound)
		{
			os.str("");
			os<<"Stage not found when trying to submit SubmitJobEvent";
			SDPA_LOG_DEBUG(os.str());
		}
}

/**
 * Submit an atomic activity to the SDPA.
 * This method is to be called by the GWES in order to delegate
 * the execution of activities.
 * The SDPA will use the callback handler SdpaGwes in order
 * to notify the GWES about activity status transitions.
 */
activity_id_t GenericDaemon::submitActivity(const activity_t &activity)
{
	// proceed similarly as in the submitWorkflow case

	job_id_t job_id(activity.getId());
	job_desc_t job_desc = activity.serialize();

	SubmitJobEvent::Ptr pEvtSubmitJob(new SubmitJobEvent(name(), name(), job_id, job_desc));
	sendEvent(pEvtSubmitJob);

	return activity.getId();
}

/**
 * Cancel a sub workflow that has previously been submitted to
 * the SDPA. The parent job has to cancel all children.
 */
void GenericDaemon::cancelWorkflow(const workflow_id_t &workflowId) throw (NoSuchWorkflowException)
{
	// cancel the job corresponding to that workflow -> send a CancelJobEvent?
	// look for the job_id corresponding to the received workflowId into job_map_
	// generate const CancelJobEvent& event
	// Job& job = job_map_[job_id];
	// and call the job.CancelJob(const CancelJobEvent& event);

	job_id_t job_id(workflowId);
	CancelJobEvent::Ptr pEvtCancelJob(new CancelJobEvent(name(), name(), workflowId));
	sendEvent(pEvtCancelJob);
}

/**
 * Cancel an atomic activity that has previously been submitted to
 * the SDPA.
 */
void GenericDaemon::cancelActivity(const activity_id_t &activityId) throw (NoSuchActivityException)
{
	// cancel the job corresponding to that activity -> send downward a CancelJobEvent?
	// look for the job_id corresponding to the received workflowId into job_map_
	// in fact they should be the same!
	// generate const CancelJobEvent& event
	// Job& job = job_map_[job_id];
	// call job.CancelJob(event);

	job_id_t job_id(activityId);
	CancelJobEvent::Ptr pEvtCancelJob(new CancelJobEvent(name(), name(), job_id));
	sendEvent(pEvtCancelJob);
}

/**
 * Notify the SDPA that a workflow finished (state transition
 * from running to finished).
 */
void GenericDaemon::workflowFinished(const workflow_id_t &workflowId) throw (NoSuchWorkflowException)
{
	// generate a JobFinishedEvent for self!
	// cancel the job corresponding to that activity -> send downward a CancelJobEvent?
	// look for the job_id corresponding to the received workflowId into job_map_
	// in fact they should be the same!
	// generate const JobFinishedEvent& event
	// Job& job = job_map_[job_id];
	// call job.JobFinished(event);

	job_id_t job_id(workflowId);
	JobFinishedEvent::Ptr pEvtJobFinished(new JobFinishedEvent(name(), name(), job_id));
	sendEvent(pEvtJobFinished);
}

/**
 * Notify the SDPA that a workflow failed (state transition
 * from running to failed).
 */
void GenericDaemon::workflowFailed(const workflow_id_t &workflowId) throw (NoSuchWorkflowException)
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

	job_id_t job_id(workflowId);
	JobFailedEvent::Ptr pEvtJobFailed( new JobFailedEvent(name(), name(), job_id ));
	sendEvent(pEvtJobFailed);
}

/**
 * Notify the SDPA that a workflow has been canceled (state
 * transition from * to terminated.
 */
void GenericDaemon::workflowCanceled(const workflow_id_t &workflowId) throw (NoSuchWorkflowException)
{
	// generate a JobCancelledEvent for self!
	// identify the job with the job_id == workflow_id_t
	// trigger a CancelJobAck for that job

	job_id_t job_id(workflowId);
	CancelJobAckEvent::Ptr pEvtCancelJobAck(new CancelJobAckEvent(name(), name(), job_id));
	sendEvent(pEvtCancelJobAck);
}


