#include "sdpa/orchFSM/SMC/OrchFSM.hpp"
#include <iostream>

using namespace std;
using namespace sdpa::fsm;
using namespace sdpa::events;

OrchFSM::OrchFSM() : SDPA_INIT_LOGGER("sdpa.fsm.OrchFSM"), m_fsmContext(*this)
{
	SDPA_LOG_DEBUG("State machine created");
}

OrchFSM::~OrchFSM()
{
	SDPA_LOG_DEBUG("State machine destroyed");
}

void OrchFSM :: action_configure(sdpa::events::StartUpEvent&) {
	cout <<"perform 'action_configure'"<< endl;
}

void OrchFSM :: action_config_ok(sdpa::events::ConfigOkEvent&) {
	cout <<"perform 'action_config_ok'"<< endl;
}

void OrchFSM :: action_config_nok(sdpa::events::ConfigNokEvent&) {
	cout <<"perform 'action_config_nok'"<< endl;
}

void OrchFSM :: action_interrupt(sdpa::events::InterruptEvent& ) {
	cout <<"perform 'action_interrupt'"<< endl;
}

void OrchFSM :: action_lifesign(sdpa::events::LifeSignEvent& ) {
	cout <<"perform 'action_lifesign'"<< endl;
}

void OrchFSM :: action_delete_job(sdpa::events::DeleteJobEvent& ) {
	cout <<"perform 'action_delete_job'"<< endl;
}

void OrchFSM :: action_request_job(sdpa::events::RequestJobEvent& ) {
	cout <<"perform 'action_request_job'"<< endl;
}

void OrchFSM :: action_request_job(sdpa::events::SubmitAckEvent& ) {
	cout <<"perform 'action_configure'"<< endl;
}

void OrchFSM :: action_config_request(sdpa::events::ConfigRequestEvent& ) {
	cout <<"perform 'action_configure'"<< endl;
}
