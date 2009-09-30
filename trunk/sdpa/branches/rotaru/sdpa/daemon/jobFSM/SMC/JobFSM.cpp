#include "JobFSM.hpp"
using namespace sdpa::events;
using namespace sdpa::fsm::smc;

//transitions
void JobFSM::Dispatch(const sdpa::events::SubmitJobAckEvent* pEvt)
{
	m_fsmContext.Dispatch(pEvt);
}

void JobFSM::CancelJob(const sdpa::events::CancelJobEvent* pEvt)
{
	m_fsmContext.CancelJob(pEvt);
}

void JobFSM::CancelJobAck(const sdpa::events::CancelJobAckEvent* pEvt)
{
	m_fsmContext.CancelJobAck(pEvt);
}

void JobFSM::DeleteJob(const sdpa::events::DeleteJobEvent* pEvt)
{
	m_fsmContext.DeleteJob(pEvt);
}

void JobFSM::QueryJobStatus(const sdpa::events::QueryJobStatusEvent* pEvt)
{
	m_status_ = m_fsmContext.getState().getName();
	m_fsmContext.QueryJobStatus(pEvt);
}

void JobFSM::JobFinished(const sdpa::events::JobFinishedEvent* pEvt)
{
	m_fsmContext.JobFinished(pEvt);
}

void JobFSM :: JobFailed(const sdpa::events::JobFailedEvent* pEvt)
{
	m_fsmContext.JobFailed(pEvt);
}

void  JobFSM ::RetrieveJobResults(const sdpa::events::RetrieveJobResultsEvent* pEvt)
{
	m_fsmContext.RetrieveJobResults(pEvt);
}
