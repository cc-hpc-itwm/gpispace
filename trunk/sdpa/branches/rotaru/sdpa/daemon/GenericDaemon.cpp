#include <sdpa/daemon/GenericDaemon.hpp>

using namespace sdpa::daemon;

GenericDaemon::GenericDaemon(const std::string &name, const std::string &outputStage) : Strategy(name)  {

}

GenericDaemon::~GenericDaemon(){ }

void GenericDaemon::perform(const seda::IEvent::Ptr& pEvent){ }

void GenericDaemon::onStageStart(const std::string &stageName){ }

void GenericDaemon::onStageStop(const std::string &stageName){ }

void GenericDaemon :: action_configure(const sdpa::events::StartUpEvent&) {
	std::cout <<"perform 'action_configure'"<< std::endl;
}

void GenericDaemon :: action_config_ok(const sdpa::events::ConfigOkEvent&) {
	std::cout <<"perform 'action_config_ok'"<< std::endl;
}

void GenericDaemon :: action_config_nok(const sdpa::events::ConfigNokEvent&) {
	std::cout <<"perform 'action_config_nok'"<< std::endl;
}

void GenericDaemon :: action_interrupt(const sdpa::events::InterruptEvent& ) {
	std::cout <<"perform 'action_interrupt'"<< std::endl;
}

void GenericDaemon :: action_lifesign(const sdpa::events::LifeSignEvent& ) {
	std::cout <<"perform 'action_lifesign'"<< std::endl;
}

void GenericDaemon :: action_delete_job(const sdpa::events::DeleteJobEvent& ) {
	std::cout <<"perform 'action_delete_job'"<< std::endl;
}

void GenericDaemon :: action_request_job(const sdpa::events::RequestJobEvent& ) {
	std::cout <<"perform 'action_request_job'"<< std::endl;
}

void GenericDaemon :: action_submit_job(const sdpa::events::SubmitJobEvent& ) {
	std::cout <<"perform 'action_submit_job'"<< std::endl;
}

void GenericDaemon :: action_submit_job_ack(const sdpa::events::SubmitJobAckEvent& ) {
	std::cout <<"perform 'action_submit_job_ack'"<< std::endl;
}

void GenericDaemon :: action_config_request(const sdpa::events::ConfigRequestEvent& ) {
	std::cout <<"perform 'action_configure'"<< std::endl;
}
