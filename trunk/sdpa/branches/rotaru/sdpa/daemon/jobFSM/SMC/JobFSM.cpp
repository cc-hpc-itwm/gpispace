#include "JobFSM.hpp"
using namespace sdpa::events;
using namespace sdpa::fsm::smc;

//transitions
void JobFSM::Dispatch()
{
	m_status_ = m_fsmContext.getState().getName();
	m_fsmContext.Dispatch();
	m_status_ = m_fsmContext.getState().getName();
}

void JobFSM::CancelJob(const sdpa::events::CancelJobEvent* pEvt)
{
	m_fsmContext.CancelJob(pEvt);
	m_status_ = m_fsmContext.getState().getName();
	m_status_ = m_fsmContext.getState().getName();
}

void JobFSM::CancelJobAck(const sdpa::events::CancelJobAckEvent* pEvt)
{
	m_status_ = m_fsmContext.getState().getName();
	m_fsmContext.CancelJobAck(pEvt);
	m_status_ = m_fsmContext.getState().getName();
}

void JobFSM::DeleteJob(const sdpa::events::DeleteJobEvent* pEvt)
{
	m_fsmContext.DeleteJob(pEvt);
	m_status_ = m_fsmContext.getState().getName();
}

void JobFSM::QueryJobStatus(const sdpa::events::QueryJobStatusEvent* pEvt)
{
	m_status_ = m_fsmContext.getState().getName();
	m_fsmContext.QueryJobStatus(pEvt);
	m_status_ = m_fsmContext.getState().getName();
}

void JobFSM::JobFinished(const sdpa::events::JobFinishedEvent* pEvt)
{
	m_status_ = m_fsmContext.getState().getName();
	m_fsmContext.JobFinished(pEvt);
	m_status_ = m_fsmContext.getState().getName();
}

void JobFSM :: JobFailed(const sdpa::events::JobFailedEvent* pEvt)
{
	m_status_ = m_fsmContext.getState().getName();
	m_fsmContext.JobFailed(pEvt);
	m_status_ = m_fsmContext.getState().getName();
}

void  JobFSM ::RetrieveJobResults(const sdpa::events::RetrieveJobResultsEvent* pEvt)
{
	m_status_ = m_fsmContext.getState().getName();
	m_fsmContext.RetrieveJobResults(pEvt);
	m_status_ = m_fsmContext.getState().getName();
}
