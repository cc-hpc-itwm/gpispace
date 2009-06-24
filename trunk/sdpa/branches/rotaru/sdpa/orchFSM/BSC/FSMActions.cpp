#include "OrchFSM.hpp"

using namespace std;


void  OrchFSM :: action_configure(const StartUpEvent& e){
	cout <<"process action 'action_configure'"<< endl;
}

void  OrchFSM :: action_config_ok(const ConfigOkEvent& e){
	cout <<"process action 'action_configure_ok'"<< endl;
}

void  OrchFSM :: action_config_nok(const ConfigNokEvent& e){
	cout <<"process action 'action_configure_nok'"<< endl;
}

void  OrchFSM :: action_interrupt(const InterruptEvent& e){
	cout <<"process action 'action_interrupt'"<< endl;
}

void  OrchFSM :: action_lifesign(const LifeSignEvent& e){

	//lifesign sghould be re-directed to scheduler
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
