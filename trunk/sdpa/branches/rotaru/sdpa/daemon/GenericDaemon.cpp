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

GenericDaemon::GenericDaemon(const std::string &name, const std::string &outputStage)
	: Strategy(name), output_stage_(outputStage), SDPA_INIT_LOGGER(name)  {

}

GenericDaemon::~GenericDaemon(){

}

void GenericDaemon::perform (const seda::IEvent::Ptr& pEvent){

}

void GenericDaemon::onStageStart(const std::string &stageName){

}

void GenericDaemon::onStageStop(const std::string &stageName){

}

void GenericDaemon::sendEvent(const std::string& stageName, const sdpa::events::SDPAEvent::Ptr& e) {
	ostringstream os;
	os<<"Send event: " <<e->str()<<" ("<<e->from()<<" -> "<<e->to()<<")"<<" to the stage "<<stageName;
	SDPA_LOG_DEBUG(os.str());
}

//helpers
Job::ptr_t GenericDaemon::findJob(const sdpa::job_id_t& job_id ) throw(JobNotFoundException)
{
	job_map_t::iterator it = job_map_.find( job_id );
	if( it != job_map_.end() )
		return it->second;
	else
		throw JobNotFoundException( job_id );
}

Worker::ptr_t GenericDaemon::findWorker(const Worker::worker_id_t& worker_id )  throw(WorkerNotFoundException){
	worker_map_t::iterator it = worker_map_.find(worker_id);
	if( it != worker_map_.end() )
		return it->second;
	else
		throw WorkerNotFoundException(worker_id);
}

void GenericDaemon::addJob(const sdpa::job_id_t& job_id, const Job::ptr_t& pJob) throw(JobNotAddedException)
{
	ostringstream os;
	job_map_t::iterator it;
	bool bsucc = false;

	pair<job_map_t::iterator, bool> ret_pair(it, bsucc);
	pair<sdpa::job_id_t, Job::ptr_t> job_pair(job_id, pJob);

	ret_pair =  job_map_.insert(job_pair);

	if(ret_pair.second)
	{
		os<<"Inserted job "<<job_id<<" into the job map"<<std::endl;
		SDPA_LOG_DEBUG(os.str());
	}
	else
		throw JobNotAddedException(job_id);
}

void GenericDaemon::markJobForDeletion(const sdpa::job_id_t& job_id, const Job::ptr_t& pJob) throw(JobNotMarkedException)
{
	ostringstream os;
	job_map_t::iterator it;
	bool bsucc = false;

	pair<job_map_t::iterator, bool> ret_pair(it, bsucc);
	pair<sdpa::job_id_t, Job::ptr_t> job_pair(job_id, pJob);

	ret_pair =  job_map_marked_for_del_.insert(job_pair);

	if(ret_pair.second)
	{
		os<<"Marked job "<<job_id<<" for deletion"<<std::endl;
		SDPA_LOG_DEBUG(os.str());
	}
	else
		throw JobNotAddedException(job_id);
}

void GenericDaemon::deleteJob(const sdpa::job_id_t& job_id) throw(JobNotDeletedException)
{
	ostringstream os;
	job_map_t::size_type ret = job_map_.erase(job_id);
	if( !ret )
		throw JobNotDeletedException(job_id);
	else
	{
		os<<"Erased job "<<job_id<<" from job map"<<std::endl;
		SDPA_LOG_DEBUG(os.str());
	}
}

std::vector<sdpa::job_id_t> GenericDaemon :: getJobIDList()
{
	std::vector<sdpa::job_id_t> v;
	for(job_map_t::iterator it = job_map_.begin(); it!= job_map_.end(); it++)
		v.push_back(it->first);

	return v;
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
		Worker::ptr_t pWorker = findWorker(worker_id);
		pWorker->update(e);
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
		Job::ptr_t pJob = findJob(e.job_id());
		pJob->DeleteJob(e);

		if( pJob->is_marked_for_deletion() )
		{
			markJobForDeletion(e.job_id(), pJob);
			deleteJob(e.job_id());

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
		Worker::ptr_t pWorker = findWorker(worker_id);
		pWorker->update(e);

		Job::ptr_t pJob = pWorker->get_next_job(e.last_job_id());
		//implement last_job_id() in RequestJobEvent !!!

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
	sdpa::uuid id;
	sdpa::uuidgen gen;
	gen(id);
	sdpa::job_id_t job_id = id.str();

	Job::ptr_t pJob( new sdpa::fsm::smc::JobFSM( job_id, e.description(), this ));

	try {
		addJob(job_id, pJob);
	}catch(sdpa::daemon::JobNotAddedException) {
		os.str("");
		os<<"Job "<<job_id<<" could not be added!"<<std::endl;
		SDPA_LOG_DEBUG(os.str());
	}catch(...) {
		os.str("");
		os<<"Unexpected exception occured when trying to add the job "<<job_id<<"!"<<std::endl;
		SDPA_LOG_DEBUG(os.str());
	}
}

void GenericDaemon::action_submit_job_ack(const sdpa::events::SubmitJobAckEvent& e) {
	ostringstream os;
	os<<"Call 'action_submit_job_ack'"<< std::endl;
	SDPA_LOG_DEBUG(os.str());

	//put the job from pending into submitted
	//call worker :: acknowledge(const sdpa::job_id_t& job_id ) = 0;
	Worker::worker_id_t worker_id = e.from();
	try {
		Worker::ptr_t pWorker = findWorker(worker_id);
		pWorker->acknowledge(e.job_id());

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
