#include <iostream>
#include "JobFSM.hpp"
#include <string>
#include <list>

using namespace std;


int main(int argc, char * argv[])
{
	JobFSM sm;
	sm.initiate();

	list<sc::event_base*> listEvents;

	listEvents.push_back( new QueryJobStatusEvent(0));
	listEvents.push_back( new RunJobEvent(10));
	listEvents.push_back( new CancelJobEvent(10));
	listEvents.push_back( new CancelJobAckEvent(10));
	listEvents.push_back( new JobFinishedEvent(10));

	listEvents.push_back( new QueryJobStatusEvent(10));

	while( !listEvents.empty() )
	{
		sc::event_base* pEvt = listEvents.front();
		sm.process_event(*pEvt);

		listEvents.pop_front();
		delete dynamic_cast<JobEvent*>(pEvt);
	}

	return 0;
}
