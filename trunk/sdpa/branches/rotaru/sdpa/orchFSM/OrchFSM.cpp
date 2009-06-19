#include "OrchFSM.hpp"
#include "boost/cast.hpp"
#include <iostream>
#include <string>
#include <stdexcept>


void OrchFSM :: printStates()
{
	for( state_iterator it = state_begin(); it != state_end(); it++ )
		cout<<"State "<<typeid(*it).name()<<endl;
}

sc::result Down::react(const StartUpEvent& e)
{
	//if( Successfully configured )
		return transit<Up>(&OrchFSM::action_startup_ok, e); // successfully configured and all services started-up
	/*else
		return transit<Down>(&OrchFSM::action_startup_nok, e); // !(successfully configured and all services started-up)*/
}


sc::result Down::react(const sc::exception_thrown & e)
{
	try
	{
	  throw;
	}
	catch ( ... )
	{
	  // ... all other exceptions are forwarded to our outer
	  // state(s). The state machine is terminated and the
	  // exception rethrown if the outer state(s) can't
	  // handle it either...
	  return forward_event();
	}
}


sc::result Up::react(const InterruptEvent& e)
{
	return transit<Down>(&OrchFSM::action_interrupt, e);
}

sc::result Up::react(const LifeSignEvent& e )
{
	return transit<Up>(&OrchFSM::action_lifesign, e);
}

sc::result Up::react(const DeleteJobEvent& e)
{
	return transit<Up>(&OrchFSM::action_delete_job, e);
}

sc::result Up::react(const RequestJobEvent& e)
{
	return transit<Up>(&OrchFSM::action_request_job, e);
}

sc::result Up::react(const SubmitAckEvent& e )
{
   	return transit<Up>(&OrchFSM::action_submit_ack, e);
}

sc::result Up::react(const ConfigRequestEvent& e )
{
   	return transit<Up>(&OrchFSM::action_config_request, e);
}


sc::result Up::react(const sc::exception_thrown & e)
{
	try
	{
	  throw;
	}
	catch ( ... )
	{
	  // ... all other exceptions are forwarded to our outer
	  // state(s). The state machine is terminated and the
	  // exception rethrown if the outer state(s) can't
	  // handle it either...
	  return forward_event();
	}
}
