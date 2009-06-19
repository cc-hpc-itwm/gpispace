#include "JobFSM.hpp"
#include "boost/cast.hpp"
#include <iostream>
#include <string>
#include <stdexcept>


void JobFSM :: printStates()
{
	for( state_iterator it = state_begin(); it != state_end(); it++ )
		cout<<"State "<<typeid(*it).name()<<endl;
}

//Pending event reactions
sc::result Pending::react(const RunJobEvent& e)
{
	//eventually, start WFE
	return transit<Running>(&JobFSM::action_dispatch, e);
}

sc::result Pending::react(const CancelJobEvent& e)
{
	return transit<Cancelled>(&JobFSM::action_cancel, e);
}

sc::result Pending::react(const QueryJobStatusEvent& e)
{
	return transit<Cancelled>(&JobFSM::action_query_status, e);
}

sc::result Pending::react(const sc::exception_thrown & e)
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

//Running event reactions
sc::result Running::react(const JobFinishedEvent& e )
{
	return transit<Finished>(&JobFSM::action_job_finished, e);
}

//Running
sc::result Running::react(const JobFailedEvent& e)
{
	return transit<Failed>(&JobFSM::action_job_failed, e);
}

//Running
sc::result Running::react(const CancelJobEvent& e)
{
	return transit<Cancelled>(&JobFSM::action_cancel, e);
}

//Running
sc::result Running::react(const QueryJobStatusEvent& e )
{
   	return transit<Running>(&JobFSM::action_query_status, e);
}

sc::result Running::react(const sc::exception_thrown & e)
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

sc::result Cancelled::react(const sc::exception_thrown & e)
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

//Terminating
sc::result Terminating::react(const CancelJobAckEvent& e)
{
    return transit<Terminated>(&Cancelled::action_cancel_ack, e);
}

//Terminating
sc::result Terminating::react(const QueryJobStatusEvent& e)
{
  	return transit<Terminating>(&Cancelled::action_query_status, e);
}

sc::result Terminating::react(const sc::exception_thrown & e)
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

//Terminated
sc::result Terminated::react(const QueryJobStatusEvent& e)
{
	return transit<Terminated>(&Cancelled::action_query_status, e);
}

sc::result Terminated::react(const sc::exception_thrown & e)
{

}

//Failed
sc::result Failed::react(const QueryJobStatusEvent& e)
{
	return transit<Failed>(&JobFSM::action_query_status, e);
}

sc::result Failed::react(const sc::exception_thrown & e)
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

//Finished
sc::result Finished::react(const QueryJobStatusEvent& e)
{
	return transit<Finished>(&JobFSM::action_query_status, e);
}

//Finished
sc::result Finished::react(const RetriveResultsEvent& e)
{
	return transit<Finished>(&JobFSM::action_retrieve_results, e);
}

sc::result Finished::react(const sc::exception_thrown & e)
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

