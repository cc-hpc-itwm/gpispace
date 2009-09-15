#include "JobFSM.hpp"
using namespace sdpa::events;
using namespace sdpa::fsm::smc;

//transitions
void JobFSM::Dispatch()
{
	m_fsmContext.Dispatch();
}

void JobFSM::CancelJob()
{
	m_fsmContext.CancelJob();
}

void JobFSM::CancelJobAck()
{
	m_fsmContext.CancelJobAck();
}

void JobFSM::DeleteJob()
{
	m_fsmContext.DeleteJob();
}

void JobFSM::QueryJobStatus()
{
	m_fsmContext.QueryJobStatus();
}

void JobFSM::JobFinished()
{
	m_fsmContext.JobFinished();
}

void JobFSM :: JobFailed()
{
	m_fsmContext.JobFailed();
}

void  JobFSM ::RetrieveJobResults()
{
	m_fsmContext.RetrieveJobResults();
}
