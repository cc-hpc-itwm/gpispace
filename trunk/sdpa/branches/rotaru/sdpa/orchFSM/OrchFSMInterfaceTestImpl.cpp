#include "sdpa/orchFSM/OrchFSMInterface.hpp"

using namespace sdpa::fsm;

void OrchFSMInterface :: action_configure(const sdpa::events::StartUpEvent&) {
	std::cout <<"perform 'action_configure'"<< std::endl;
}

void OrchFSMInterface :: action_config_ok(const sdpa::events::ConfigOkEvent&) {
	std::cout <<"perform 'action_config_ok'"<< std::endl;
}

void OrchFSMInterface :: action_config_nok(const sdpa::events::ConfigNokEvent&) {
	std::cout <<"perform 'action_config_nok'"<< std::endl;
}

void OrchFSMInterface :: action_interrupt(const sdpa::events::InterruptEvent& ) {
	std::cout <<"perform 'action_interrupt'"<< std::endl;
}

void OrchFSMInterface :: action_lifesign(const sdpa::events::LifeSignEvent& ) {
	std::cout <<"perform 'action_lifesign'"<< std::endl;
}

void OrchFSMInterface :: action_delete_job(const sdpa::events::DeleteJobEvent& ) {
	std::cout <<"perform 'action_delete_job'"<< std::endl;
}

void OrchFSMInterface :: action_request_job(const sdpa::events::RequestJobEvent& ) {
	std::cout <<"perform 'action_request_job'"<< std::endl;
}

void OrchFSMInterface :: action_submit_job(const sdpa::events::SubmitJobEvent& ) {
	std::cout <<"perform 'action_submit_job'"<< std::endl;
}

void OrchFSMInterface :: action_submit_job_ack(const sdpa::events::SubmitJobAckEvent& ) {
	std::cout <<"perform 'action_submit_job_ack'"<< std::endl;
}

void OrchFSMInterface :: action_config_request(const sdpa::events::ConfigRequestEvent& ) {
	std::cout <<"perform 'action_configure'"<< std::endl;
}
