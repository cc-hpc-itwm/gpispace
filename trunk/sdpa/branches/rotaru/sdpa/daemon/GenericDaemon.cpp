#include <seda/StageRegistry.hpp>

#include <sdpa/daemon/GenericDaemon.hpp>
#include <sdpa/daemon/JobImpl.hpp>
#include <sdpa/daemon/jobFSM/SMC/JobFSM.hpp>
#include <sdpa/daemon/jobFSM/BSC/JobFSM.hpp>
#include <sdpa/uuid.hpp>
#include <sdpa/uuidgen.hpp>
#include <sstream>
#include <map>


using namespace std;
using namespace sdpa::daemon;
using namespace sdpa::wf;

GenericDaemon::GenericDaemon(const std::string &name, const std::string &outputStage)
	: Strategy(name), output_stage_(outputStage), SDPA_INIT_LOGGER(name),
	ptr_job_man_(new JobManager()), ptr_worker_man_(new WorkerManager())
{
	ptr_scheduler_ = shared_ptr<SchedulerImpl>(new SchedulerImpl(ptr_job_man_, ptr_worker_man_));
}

GenericDaemon::~GenericDaemon(){

}

GenericDaemon::ptr_t GenericDaemon::create(const std::string &name_prefix,  const std::string &outputStage )
{
	// warning: we introduce a cycle here, we have to resolve it during shutdown!
	GenericDaemon::ptr_t daemon( new GenericDaemon(name_prefix, outputStage) );
	seda::Stage::Ptr daemon_stage( new seda::Stage(name_prefix + "", daemon) );
	daemon->setStage(daemon_stage);
	seda::StageRegistry::instance().insert(daemon_stage);
	daemon_stage->start();

	return daemon;
}


void GenericDaemon::perform (const seda::IEvent::Ptr& pEvent){

}

void GenericDaemon::onStageStart(const std::string &stageName){
//initiate GWES and the pointer pSDPA_to_WFE
}

void GenericDaemon::onStageStop(const std::string &stageName){

}

void GenericDaemon::sendEvent(const std::string& stageName, const sdpa::events::SDPAEvent::Ptr& e) {
	ostringstream os;
	os<<"Send event: " <<e->str()<<" ("<<e->from()<<" -> "<<e->to()<<")"<<" to the stage "<<stageName;
	SDPA_LOG_DEBUG(os.str());
}

//actions
void GenericDaemon::action_configure(const sdpa::events::StartUpEvent& e)
{
	ostringstream os;
	os<<"Call 'action_configure'"<< std::endl;
	SDPA_LOG_DEBUG(os.str());
}

void GenericDaemon::action_config_ok(const sdpa::events::ConfigOkEvent& e)
{
	ostringstream os;
	os<<"Call 'action_config_ok'"<< std::endl;
	SDPA_LOG_DEBUG(os.str());
}

void GenericDaemon::action_config_nok(const sdpa::events::ConfigNokEvent& e)
{
	ostringstream os;
	os<<"Call 'action_config_nok'"<< std::endl;
	SDPA_LOG_DEBUG(os.str());
}

void GenericDaemon::action_interrupt(const sdpa::events::InterruptEvent& e)
{
	ostringstream os;
	os<<"Call 'action_interrupt'"<< std::endl;
	SDPA_LOG_DEBUG(os.str());
}

void GenericDaemon::action_lifesign(const sdpa::events::LifeSignEvent& e)
{
	ostringstream os;
	os<<"Call 'action_lifesign'"<< std::endl;
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
		Worker::ptr_t ptr_worker = ptr_worker_man_->findWorker(worker_id);
		ptr_worker->update(e);
		os.str("");
		os<<"Received LS. Updated the time stamp of the worker "<<worker_id<<std::endl;
		SDPA_LOG_DEBUG(os.str());
	} catch(sdpa::daemon::WorkerNotFoundException) {
		os.str("");
		os<<"Worker "<<worker_id<<" not found!"<<std::endl;
		SDPA_LOG_DEBUG(os.str());
	} catch(...) {
		os.str("");
		os<<"Unexpected exception occurred!"<<std::endl;
		SDPA_LOG_DEBUG(os.str());
	}
}

void GenericDaemon::action_delete_job(const sdpa::events::DeleteJobEvent& e )
{
	ostringstream os;
	os<<"Call 'action_delete_job'"<< std::endl;
	SDPA_LOG_DEBUG(os.str());

	os.str("");
	os<<"received DeleteJobEvent from "<<e.from()<<" addressed to "<<e.to()<<std::endl;
	SDPA_LOG_DEBUG(os.str());

	try{
		Job::ptr_t pJob = ptr_job_man_->findJob(e.job_id());
		pJob->DeleteJob(e);

		if( pJob->is_marked_for_deletion() )
		{
			ptr_job_man_->markJobForDeletion(e.job_id(), pJob);
			ptr_job_man_->deleteJob(e.job_id());

			sdpa::events::DeleteJobAckEvent::Ptr pDelAckEvt(new sdpa::events::DeleteJobAckEvent(name(), e.from()));
			sendEvent(output_stage_, pDelAckEvt);
		}
	} catch(sdpa::daemon::JobNotFoundException){
		os.str("");
		os<<"Job "<<e.job_id()<<" not found!"<<std::endl;
		SDPA_LOG_DEBUG(os.str());
	} catch(sdpa::daemon::JobNotMarkedException ){
		os.str("");
		os<<"Job "<<e.job_id()<<" not marked for deletion!"<<std::endl;
		SDPA_LOG_DEBUG(os.str());
	}catch(sdpa::daemon::JobNotDeletedException ){
		os.str("");
		os<<"Job "<<e.job_id()<<" not deleted!"<<std::endl;
		SDPA_LOG_DEBUG(os.str());
	} catch(...) {
		os.str("");
		os<<"Unexpected exception. Most probably the job to be deleted was not in a final state!"<<e.job_id()<<"!"<<std::endl;
		SDPA_LOG_DEBUG(os.str());
	}
}

void GenericDaemon::action_request_job(const sdpa::events::RequestJobEvent& e)
{
	ostringstream os;
	os<<"Call 'action_request_job'"<< std::endl;
	SDPA_LOG_DEBUG(os.str());
	/*
	the slave(aggregator) requests new executable jobs
	this message is sent in regular frequencies depending on the load of the slave(aggregator)
	this message can be seen as the trigger for a submitJob
	it contains the id of the last job that has been received
	the orchestrator answers to this message with a submitJob
	 */

	//take a job from the workers' queue? and serve it
	Worker::worker_id_t worker_id = e.from();
	try {
		Worker::ptr_t ptr_worker = ptr_worker_man_->findWorker(worker_id);
		ptr_worker->update(e);

		Job::ptr_t pJob = ptr_worker->get_next_job(e.last_job_id());
		// implement last_job_id() in RequestJobEvent !!!
		// trigger event

	} catch(sdpa::daemon::WorkerNotFoundException) {
		os.str("");
		os<<"Worker "<<worker_id<<" not found!"<<std::endl;
		SDPA_LOG_DEBUG(os.str());
	} catch(...) {
		os.str("");
		os<<"Unexpected exception occurred!"<<std::endl;
		SDPA_LOG_DEBUG(os.str());
	}

}

void GenericDaemon::action_submit_job(const sdpa::events::SubmitJobEvent& e)
{
	ostringstream os;
	os<<"Call 'action_submit_job'"<< std::endl;
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

	JobId job_id; //already assigns an unique job_id (i.e. the constructor calls the generator)
	Job::ptr_t pJob( new sdpa::fsm::smc::JobFSM( job_id, e.description(), this ));

	try {
		ptr_job_man_->addJob(job_id, pJob);

		//send back a SubmitJobAckEvent
		sdpa::events::SubmitJobAckEvent::Ptr pSubmitJobAckEvt(new sdpa::events::SubmitJobAckEvent(name(), e.from()));
		sendEvent(output_stage_, pSubmitJobAckEvt);

	}catch(sdpa::daemon::JobNotAddedException) {
		os.str("");
		os<<"Job "<<job_id<<" could not be added!"<<std::endl;
		SDPA_LOG_DEBUG(os.str());
		//send back an ErrorEvent
	}catch(...) {
		os.str("");
		os<<"Unexpected exception occured when trying to add the job "<<job_id<<"!"<<std::endl;
		SDPA_LOG_DEBUG(os.str());
		//send back an ErrorEvent
	}
}

void GenericDaemon::action_submit_job_ack(const sdpa::events::SubmitJobAckEvent& e) {
	ostringstream os;
	os<<"Call 'action_submit_job_ack'"<< std::endl;
	SDPA_LOG_DEBUG(os.str());

	//put the job from pending into submitted
	//call worker :: acknowledge(const sdpa::job_id_t& job_id ) = ;
	Worker::worker_id_t worker_id = e.from();
	try {
		Worker::ptr_t ptr_worker = ptr_worker_man_->findWorker(worker_id);
		ptr_worker->acknowledge(e.job_id());

	} catch(sdpa::daemon::WorkerNotFoundException) {
		os.str("");
		os<<"Worker "<<worker_id<<" not found!"<<endl;
		SDPA_LOG_DEBUG(os.str());
	} catch(...) {
		os.str("");
		os<<"Unexpected exception occurred!"<<std::endl;
		SDPA_LOG_DEBUG(os.str());
	}
}

void GenericDaemon::action_config_request(const sdpa::events::ConfigRequestEvent& e) {
	ostringstream os;
	os<<"Call 'action_configure'"<< std::endl;
	SDPA_LOG_DEBUG(os.str());
	/*
	 * on startup the aggregator tries to retrieve a configuration from its orchestrator
	 * post ConfigReplyEvent/message that contains the configuration data for the requesting aggregator
     * TODO: what is contained in the Configuration?
	 */
}

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
	//proceed similarly as in the submitWorkflow case
}

/**
 * Cancel a sub workflow that has previously been submitted to
 * the SDPA. The parent job has to cancel all children.
 */
void GenericDaemon::cancelWorkflow(const workflow_id_t &workflowId) throw (NoSuchWorkflowException)
{
	// cancel the job corresponding to that workflow -> send a CancelJobEvent?
}

/**
 * Cancel an atomic activity that has previously been submitted to
 * the SDPA.
 */
void GenericDaemon::cancelActivity(const activity_id_t &activityId) throw (NoSuchActivityException)
{
	// cancel the job corresponding to that activity -> send downward a CancelJobEvent?
}

/**
 * Notify the SDPA that a workflow finished (state transition
 * from running to finished).
 */
void GenericDaemon::workflowFinished(const workflow_id_t &workflowId) throw (NoSuchWorkflowException)
{
	// generate a JobFinishedEvent for self
}

/**
 * Notify the SDPA that a workflow failed (state transition
 * from running to failed).
 */
void GenericDaemon::workflowFailed(const workflow_id_t &workflowId) throw (NoSuchWorkflowException)
{
	// generate a JobFailedEvent for self
}

/**
 * Notify the SDPA that a workflow has been canceled (state
 * transition from * to terminated.
 */
void GenericDaemon::workflowCanceled(const workflow_id_t &workflowId) throw (NoSuchWorkflowException)
{
	// generate a JobCancelledEvent for self
}
