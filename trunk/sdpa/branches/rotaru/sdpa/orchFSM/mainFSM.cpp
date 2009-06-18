#include <iostream>
#include "OrchFSM.hpp"
#include <string>
#include <list>

using namespace std;


int main(int argc, char * argv[])
{
	OrchFSM sm;
	sm.initiate();

	list<sc::event_base*> listEvents;

	listEvents.push_back( new StartUpEvent());
	listEvents.push_back( new LifeSignEvent());

	listEvents.push_back( new RequestJobEvent());
	listEvents.push_back( new SubmitAckEvent());
	listEvents.push_back( new DeleteJobEvent());
	listEvents.push_back( new ConfigRequestEvent());
	listEvents.push_back( new InterruptEvent());


	while( !listEvents.empty() )
	{
		sc::event_base* pEvt = listEvents.front();
		sm.process_event(*pEvt);

		listEvents.pop_front();
		delete dynamic_cast<MgmtEvent*>(pEvt);
	}

	return 0;
}
