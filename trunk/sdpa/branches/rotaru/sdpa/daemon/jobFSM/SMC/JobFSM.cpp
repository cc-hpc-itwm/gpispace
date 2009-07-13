#include "JobFSM.hpp"

using namespace sdpa::fsm::smc;

//transitions
void JobFSM::RunJob(const sdpa::events::RunJobEvent& e)
{
	m_fsmContext.RunJob(e);
}

void JobFSM::CancelJob(const sdpa::events::CancelJobEvent& e)
{
	m_fsmContext.CancelJob(e);
}

void JobFSM::CancelJobAck(const sdpa::events::CancelJobAckEvent& e)
{
	m_fsmContext.CancelJobAck(e);
}

void JobFSM::DeleteJob(const sdpa::events::DeleteJobEvent& e)
{
	m_fsmContext.DeleteJob(e);
}

void JobFSM::QueryJobStatus(const sdpa::events::QueryJobStatusEvent& e)
{
	m_fsmContext.QueryJobStatus(e);
}

void JobFSM::JobFinished(const sdpa::events::JobFinishedEvent& e )
{
	m_fsmContext.JobFinished(e);
}

void JobFSM :: JobFailed(const sdpa::events::JobFailedEvent& e)
{
	m_fsmContext.JobFailed(e);
}

void  JobFSM ::RetrieveJobResults(const sdpa::events::RetrieveJobResultsEvent& e )
{
	m_fsmContext.RetrieveJobResults(e);
}

