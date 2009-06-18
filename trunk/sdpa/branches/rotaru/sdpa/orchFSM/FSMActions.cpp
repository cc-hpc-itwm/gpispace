#include "OrchFSM.hpp"

void  OrchFSM :: action_startup_ok(const StartUpEvent& e){
	cout <<"process action 'action_startup'"<< endl;
}

void  OrchFSM :: action_startup_nok(const StartUpEvent& e){
	cout <<"process action 'action_startup'"<< endl;
}

void  OrchFSM :: action_interrupt(const InterruptEvent& e){
	cout <<"process action 'action_interrupt'"<< endl;
}

void  OrchFSM :: action_lifesign(const LifeSignEvent& e){
	cout <<"process action 'action_lifesign'"<< endl;
}

void  OrchFSM :: action_delete_job(const DeleteJobEvent& e){
	cout <<"process action 'action_delete_job'"<< endl;
}

void  OrchFSM :: action_request_job(const RequestJobEvent& e){
	cout <<"process action 'action_request_job'"<< endl;
}

void  OrchFSM :: action_submit_ack(const SubmitAckEvent& e){
	cout <<"process action 'action_submit_ack'"<< endl;
}

void  OrchFSM :: action_config_request(const ConfigRequestEvent& e){
	cout <<"process action 'action_config_request'"<< endl;
}
