#include <sdpa/daemon/GenericDaemon.hpp>
#include <sdpa/daemon/JobImpl.hpp>
#include <sdpa/jobFSM/SMC/JobFSM.hpp>

#include <map>

using namespace std;
using namespace sdpa::daemon;

GenericDaemon::GenericDaemon(const std::string &name, const std::string &outputStage) : Strategy(name)  {

}

GenericDaemon::~GenericDaemon(){ }

void GenericDaemon::perform(const seda::IEvent::Ptr& pEvent){

}

void GenericDaemon::onStageStart(const std::string &stageName){

}

void GenericDaemon::onStageStop(const std::string &stageName){

}

void GenericDaemon :: action_configure(const sdpa::events::StartUpEvent& e) {
	std::cout <<"perform 'action_configure'"<< std::endl;
}

void GenericDaemon :: action_config_ok(const sdpa::events::ConfigOkEvent& e) {
	std::cout <<"perform 'action_config_ok'"<< std::endl;
}

void GenericDaemon :: action_config_nok(const sdpa::events::ConfigNokEvent& e) {
	std::cout <<"perform 'action_config_nok'"<< std::endl;
}

void GenericDaemon :: action_interrupt(const sdpa::events::InterruptEvent& e) {
	std::cout <<"perform 'action_interrupt'"<< std::endl;
}

void GenericDaemon :: action_lifesign(const sdpa::events::LifeSignEvent& e) {
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

void GenericDaemon :: action_delete_job(const sdpa::events::DeleteJobEvent& e ) {
	std::cout <<"perform 'GenericDaemon::action_delete_job'"<< std::endl;

	job_map_t::iterator it = job_map_.find( e.job_id() );

	if( it!= job_map_.end() )
	{
		std::cout<<"Job "<<e.job_id()<<" found!"<<std::endl;
		Job::ptr_t pJob = it->second;

		pJob->DeleteJob(e);

		if( pJob->is_marked_for_deletion() )
		{
			job_map_marked_for_del_.insert(pair<sdpa::job_id_t, Job::ptr_t>(e.job_id(), pJob));
			job_map_.erase(it);
			std::cout<<"Erased from job map"<<std::endl;
			//notify/wake-up the garbage collector
			//post a DeleteJobAckEvent to the user
		}
	}
	else
		std::cout<<"Job "<<e.job_id()<<" not found!"<<std::endl;
}

void GenericDaemon :: action_request_job(const sdpa::events::RequestJobEvent& e) {
	std::cout <<"perform 'action_request_job'"<< std::endl;
	/*
	the slave(aggregator) requests new executable jobs
	this message is sent in regular frequencies depending on the load of the slave(aggregator)
	this message can be seen as the trigger for a submitJob
	it contains the id of the last job that has been received
	the orchestrator answers to this message with a submitJob
	 */
}

void GenericDaemon :: action_submit_job(const sdpa::events::SubmitJobEvent& e) {
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
    * send an submitJobAck back
	*/

	sdpa::job_id_t strTestJobID("10");

	Job::ptr_t pJob( new sdpa::fsm::smc::JobFSM( strTestJobID, e.description() ));
	job_map_.insert( make_pair( strTestJobID, pJob) );
	std::cout<<"Job "<<strTestJobID<<" inserted into job map"<<std::endl;
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
