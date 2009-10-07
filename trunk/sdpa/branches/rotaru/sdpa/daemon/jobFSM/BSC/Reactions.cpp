#include "JobFSM.hpp"
#include <sdpa/daemon/JobImpl.hpp>

#include "boost/cast.hpp"
#include <iostream>
#include <string>
#include <stdexcept>

using namespace sdpa::events;
//using namespace sdpa::daemon;
using namespace sdpa::fsm::bsc;


void JobFSM::CancelJob(const sdpa::events::CancelJobEvent* pEvt) { process_event(*pEvt); }
void JobFSM::CancelJobAck(const sdpa::events::CancelJobAckEvent* pEvt) { process_event(*pEvt); }
void JobFSM::DeleteJob(const sdpa::events::DeleteJobEvent* pEvt) { process_event(*pEvt); }
void JobFSM::JobFailed(const sdpa::events::JobFailedEvent* pEvt) { process_event(*pEvt); }
void JobFSM::JobFinished(const sdpa::events::JobFinishedEvent* pEvt) { process_event(*pEvt); }
void JobFSM::QueryJobStatus(const sdpa::events::QueryJobStatusEvent* pEvt) { process_event(*pEvt); }
void JobFSM::RetrieveJobResults(const sdpa::events::RetrieveJobResultsEvent* pEvt) { process_event(*pEvt); }
void JobFSM::Dispatch() { process_event(EvtBSCDispatch()); }

//Pending event reactions
sc::result Pending::react(const EvtBSCDispatch& e)
{
	//eventually, start WFE
	return transit<Running>();//(&JobFSM::action_run_job, e);
}

sc::result Pending::react(const CancelJobEvent& e)
{
	return transit<Cancelled>(&JobFSM::action_cancel_job_from_pending, e);
}

/*sc::result Pending::react(const QueryJobStatusEvent& e)
{
	return transit<Pending>(&JobFSM::action_query_job_status, e);
}
*/

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
/*sc::result Running::react(const QueryJobStatusEvent& e )
{
   	return transit<Running>(&JobFSM::action_query_job_status, e);
}*/

sc::result Running::react(const sc::exception_thrown & e)
{
	try
	{
	  throw;
	}
	catch ( ... )
	{
	  // ... all other exceptions are forwarded to our outer
	  // state(s). The state machine is Cancelled and the
	  // exception rethrown if the outer state(s) can't
	  // handle it either...
	  return forward_event();
	}
}

sc::result Cancel::react(const sc::exception_thrown & e)
{
	try
	{
	  throw;
	}
	catch ( ... )
	{
	  // ... all other exceptions are forwarded to our outer
	  // state(s). The state machine is Cancelled and the
	  // exception rethrown if the outer state(s) can't
	  // handle it either...
	  return forward_event();
	}
}

//Cancelling
sc::result Cancelling::react(const CancelJobAckEvent& e)
{
    return transit<Cancelled>(&JobFSM::action_cancel_job_ack, e);
}

//Cancelling
/*sc::result Cancelling::react(const QueryJobStatusEvent& e)
{
  	return transit<Cancelling>(&JobFSM::action_query_job_status, e);
}*/

//Running event reactions
sc::result Cancelling::react(const JobFinishedEvent& e )
{
	return transit<Cancelled>(&JobFSM::action_job_finished, e);
}

//Running
sc::result Cancelling::react(const JobFailedEvent& e)
{
	return transit<Cancelled>(&JobFSM::action_job_failed, e);
}

sc::result Cancelling::react(const sc::exception_thrown & e)
{
	try
	{
	  throw;
	}
	catch ( ... )
	{
	  // ... all other exceptions are forwarded to our outer
	  // state(s). The state machine is Cancelled and the
	  // exception rethrown if the outer state(s) can't
	  // handle it either...
	  return forward_event();
	}
}

//Cancelled
/*sc::result Cancelled::react(const QueryJobStatusEvent& e)
{
	return transit<Cancelled>(&JobFSM::action_query_job_status, e);
}*/

//Cancelled
sc::result Cancelled::react(const DeleteJobEvent& e)
{
	return transit<Cancelled>(&JobFSM::action_delete_job, e);
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
	  // state(s). The state machine is Cancelled and the
	  // exception re-thrown if the outer state(s) can't
	  // handle it either...
	  return forward_event();
	}
}

//Failed
/*sc::result Failed::react(const QueryJobStatusEvent& e)
{
	return transit<Failed>(&JobFSM::action_query_job_status, e);
}*/

//Cancelled
sc::result Failed::react(const DeleteJobEvent& e)
{
	return transit<Failed>(&JobFSM::action_delete_job, e);
}

//Finished
sc::result Failed::react(const RetrieveJobResultsEvent& e)
{
	return transit<Finished>(&JobFSM::action_retrieve_job_results, e);
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
	  // state(s). The state machine is Cancelled and the
	  // exception rethrown if the outer state(s) can't
	  // handle it either...
	  return forward_event();
	}
}

//Finished
/*sc::result Finished::react(const QueryJobStatusEvent& e)
{
	return transit<Finished>(&JobFSM::action_query_job_status, e);
}*/

//Cancelled
sc::result Finished::react(const DeleteJobEvent& e)
{
	return transit<Finished>(&JobFSM::action_delete_job, e);
}


//Finished
sc::result Finished::react(const RetrieveJobResultsEvent& e)
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
	  // state(s). The state machine is Cancelled and the
	  // exception rethrown if the outer state(s) can't
	  // handle it either...
	  return forward_event();
	}
}

