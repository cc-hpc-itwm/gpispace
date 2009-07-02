#include <sdpa/daemon/GenericDaemon.hpp>
#include <sdpa/daemon/JobImpl.hpp>
#include <sdpa/daemon/jobFSM/SMC/JobFSM.hpp>
#include <sdpa/daemon/jobFSM/BSC/JobFSM.hpp>
#include <sdpa/uuid.hpp>
#include <sdpa/uuidgen.hpp>
#include <map>


using namespace std;
using namespace sdpa::daemon;

GenericDaemon::GenericDaemon(const std::string &name, const std::string &outputStage) : Strategy(name)  {

}

GenericDaemon::~GenericDaemon(){

}

void GenericDaemon::perform(const seda::IEvent::Ptr& pEvent){

}

void GenericDaemon::onStageStart(const std::string &stageName){

}

void GenericDaemon::onStageStop(const std::string &stageName){

}

//helpers
Job::ptr_t GenericDaemon::FindJob(const sdpa::job_id_t& job_id ) throw(JobNotFoundException)
{
	job_map_t::iterator it = job_map_.find( job_id );
	if( it != job_map_.end() )
		return it->second;
	else
		throw JobNotFoundException( job_id );
}

void GenericDaemon :: AddJob(const sdpa::job_id_t& job_id, const Job::ptr_t& pJob) throw(JobNotAddedException)
{
	job_map_t::iterator it;
	bool bsucc = false;

	pair<job_map_t::iterator, bool> ret_pair(it, bsucc);
	pair<sdpa::job_id_t, Job::ptr_t> job_pair(job_id, pJob);

	ret_pair =  job_map_.insert(job_pair);

	if(ret_pair.second)
		std::cout<<"Inserted "<<"Job "<<job_id<<" into the job map"<<std::endl;
	else
		 throw JobNotAddedException(job_id);
}

void GenericDaemon :: MarkJobForDeletion(const sdpa::job_id_t& job_id, const Job::ptr_t& pJob) throw(JobNotMarkedException)
{
	job_map_t::iterator it;
	bool bsucc = false;

	pair<job_map_t::iterator, bool> ret_pair(it, bsucc);
	pair<sdpa::job_id_t, Job::ptr_t> job_pair(job_id, pJob);

	ret_pair =  job_map_marked_for_del_.insert(job_pair);

	if(ret_pair.second)
		std::cout<<"Marked "<<"Job "<<job_id<<" for deletion"<<std::endl;
	else
		 throw JobNotAddedException(job_id);
}

void GenericDaemon :: DeleteJob(const sdpa::job_id_t& job_id) throw(JobNotDeletedException)
{
	job_map_t::size_type ret = job_map_.erase(job_id);
	if( !ret )
		throw JobNotDeletedException(job_id);
	else
		std::cout<<"Erased from job map"<<std::endl;
}

std::vector<sdpa::job_id_t> GenericDaemon :: GetJobIDList()
{
	std::vector<sdpa::job_id_t> v;
	for(job_map_t::iterator it = job_map_.begin(); it!= job_map_.end(); it++)
		v.push_back(it->first);

	return v;
}

//actions
void GenericDaemon :: action_configure(const sdpa::events::StartUpEvent& e)
{
	std::cout <<"perform 'action_configure'"<< std::endl;
}

void GenericDaemon :: action_config_ok(const sdpa::events::ConfigOkEvent& e)
{
	std::cout <<"perform 'action_config_ok'"<< std::endl;
}

void GenericDaemon :: action_config_nok(const sdpa::events::ConfigNokEvent& e)
{
	std::cout <<"perform 'action_config_nok'"<< std::endl;
}

void GenericDaemon :: action_interrupt(const sdpa::events::InterruptEvent& e)
{
	std::cout <<"perform 'action_interrupt'"<< std::endl;
}

void GenericDaemon :: action_lifesign(const sdpa::events::LifeSignEvent& e)
{
	std::cout <<"perform 'action_lifesign'"<< std::endl;
    /*
    o timestamp, load, other probably useful information
    o last_job_id the id of the last received job identification
    o the aggregator first sends a request for configuration to its orchestrator
    o the orchestrator allocates an internal data structure to keep track of the state of the aggregator
    o this datastructure is being updated everytime a message is received
	o an aggregator is supposed to be unavailable when no messages have been received for a (configurable) period of time
     */
}

void GenericDaemon :: action_delete_job(const sdpa::events::DeleteJobEvent& e )
{
	std::cout <<"perform 'GenericDaemon::action_delete_job'"<< std::endl;

	try{
		Job::ptr_t pJob = FindJob(e.job_id());
		pJob->DeleteJob(e);

		if( pJob->is_marked_for_deletion() )
		{
			MarkJobForDeletion(e.job_id(), pJob);
			DeleteJob(e.job_id());

			//notify/wake-up the garbage collector
			//post a DeleteJobAckEvent to the user
		}
	} catch(sdpa::daemon::JobNotFoundException){
		std::cout<<"Job "<<e.job_id()<<" not found!"<<std::endl;
	} catch(sdpa::daemon::JobNotMarkedException ){
		std::cout<<"Job "<<e.job_id()<<" not marked for deletion!"<<std::endl;
	}catch(sdpa::daemon::JobNotDeletedException ){
		std::cout<<"Job "<<e.job_id()<<" not deleted!"<<std::endl;
	}  catch(...) {
		std::cout<<"Unexpected exception. Most probably the job to be deleted was not in a final state!"<<e.job_id()<<"!"<<std::endl;
	}

}

void GenericDaemon :: action_request_job(const sdpa::events::RequestJobEvent& e)
{
	std::cout <<"perform 'action_request_job'"<< std::endl;
	/*
	the slave(aggregator) requests new executable jobs
	this message is sent in regular frequencies depending on the load of the slave(aggregator)
	this message can be seen as the trigger for a submitJob
	it contains the id of the last job that has been received
	the orchestrator answers to this message with a submitJob
	 */
}

void GenericDaemon :: action_submit_job(const sdpa::events::SubmitJobEvent& e)
{
	std::cout <<"perform 'action_submit_job'"<< std::endl;

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

	Job::ptr_t pJob( new sdpa::fsm::smc::JobFSM( job_id, e.description() ));

	try {
		AddJob(job_id, pJob);
	}catch(sdpa::daemon::JobNotAddedException) {
		std::cout<<"Job "<<job_id<<" could not be added!"<<std::endl;
	}catch(...) {
		std::cout<<"Unexpected exception occured when trying to add the job "<<job_id<<"!"<<std::endl;
	}
}

void GenericDaemon :: action_submit_job_ack(const sdpa::events::SubmitJobAckEvent& e) {
	std::cout <<"perform 'action_submit_job_ack'"<< std::endl;
}

void GenericDaemon :: action_config_request(const sdpa::events::ConfigRequestEvent& e) {
	std::cout <<"perform 'action_configure'"<< std::endl;
	/*
	 * on startup the aggregator tries to retrieve a configuration from its orchestrator
	 * post ConfigReplyEvent/message that contains the configuration data for the requesting aggregator
     * TODO: what is contained in the Configuration?
	 */
}
