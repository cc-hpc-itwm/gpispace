#include "JobFSM.hpp"

using namespace sdpa::fsm::smc;

//transitions
void JobFSM::process_event(const sdpa::events::SubmitJobEvent& e)
{
	m_fsmContext.SubmitJob(e);
}

void JobFSM::process_event(const sdpa::events::CancelJobEvent& e)
{
	m_fsmContext.CancelJob(e);
}

void JobFSM::process_event(const sdpa::events::CancelJobAckEvent& e)
{
	m_fsmContext.CancelJobAck(e);
}

void JobFSM::process_event(const sdpa::events::DeleteJobEvent& e)
{
	m_fsmContext.DeleteJob(e);
}

void JobFSM::process_event(const sdpa::events::QueryJobStatusEvent& e)
{
	m_fsmContext.QueryJobStatus(e);
}

void JobFSM::process_event(const sdpa::events::JobFinishedEvent& e )
{
	m_fsmContext.JobFinished(e);
}

void JobFSM :: process_event(const sdpa::events::JobFailedEvent& e)
{
	m_fsmContext.JobFailed(e);
}

void  JobFSM ::process_event(const sdpa::events::RetrieveJobResultsEvent& e )
{
	m_fsmContext.RetrieveJobResults(e);
}

