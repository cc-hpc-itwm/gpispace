#include "JobFSM.hpp"
#include <sdpa/daemon/JobImpl.hpp>

#include "boost/cast.hpp"
#include <iostream>
#include <string>
#include <stdexcept>

using namespace sdpa::events;
using namespace sdpa::daemon;
using namespace sdpa::fsm::bsc;


JobFSM::JobFSM(const Job::job_id_t &id, const Job::job_desc_t &desc, const Job::job_id_t &parent)
				: JobImpl(id, desc, parent), SDPA_INIT_LOGGER("sdpa.fsm.JobFSM")
{
	SDPA_LOG_DEBUG("State machine created");
}

JobFSM::~JobFSM()  throw () { SDPA_LOG_DEBUG("State machine destroyed"); }


//Pending event reactions
sc::result Pending::react(const RunJobEvent& e)
{
	//eventually, start WFE
	return transit<Running>(&JobFSM::action_run_job, e);
}

sc::result Pending::react(const CancelJobEvent& e)
{
	return transit<Cancelled>(&JobFSM::action_cancel_job, e);
}

sc::result Pending::react(const QueryJobStatusEvent& e)
{
	return transit<Pending>(&JobFSM::action_query_job_status, e);
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
	  // state(s). The state machine is termi nated and the
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
	return transit<Cancelled>(&JobFSM::action_cancel_job, e);
}

//Running
sc::result Running::react(const QueryJobStatusEvent& e )
{
   	return transit<Running>(&JobFSM::action_query_job_status, e);
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
    return transit<Terminated>(&JobFSM::action_cancel_job_ack, e);
}

//Terminating
sc::result Terminating::react(const QueryJobStatusEvent& e)
{
  	return transit<Terminating>(&JobFSM::action_query_job_status, e);
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
	return transit<Terminated>(&JobFSM::action_query_job_status, e);
}

sc::result Terminated::react(const sc::exception_thrown & e)
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

//Failed
sc::result Failed::react(const QueryJobStatusEvent& e)
{
	return transit<Failed>(&JobFSM::action_query_job_status, e);
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
	return transit<Finished>(&JobFSM::action_query_job_status, e);
}

//Finished
sc::result Finished::react(const RetriveResultsEvent& e)
{
	return transit<Finished>(&JobFSM::action_retrieve_job_results, e);
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

