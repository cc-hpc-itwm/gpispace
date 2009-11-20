/*
 * =====================================================================================
 *
 *       Filename:  JobFSM.cpp
 *
 *    Description:  Job state machine (state machine compiler)
 *
 *        Version:  1.0
 *        Created:
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Dr. Tiberiu Rotaru, tiberiu.rotaru@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */
#include "JobFSM.hpp"
using namespace sdpa::events;
using namespace sdpa::fsm::smc;

//transitions
void JobFSM::Dispatch()
{
	lock_type lock(mtx_);
	m_status_ = m_fsmContext.getState().getName();
	m_fsmContext.Dispatch();
	m_status_ = m_fsmContext.getState().getName();
}

void JobFSM::CancelJob(const sdpa::events::CancelJobEvent* pEvt)
{
	lock_type lock(mtx_);
	m_fsmContext.CancelJob(pEvt);
	m_status_ = m_fsmContext.getState().getName();
	m_status_ = m_fsmContext.getState().getName();
}

void JobFSM::CancelJobAck(const sdpa::events::CancelJobAckEvent* pEvt)
{
	lock_type lock(mtx_);
	m_status_ = m_fsmContext.getState().getName();
	m_fsmContext.CancelJobAck(pEvt);
	m_status_ = m_fsmContext.getState().getName();
}

void JobFSM::DeleteJob(const sdpa::events::DeleteJobEvent* pEvt)
{
	lock_type lock(mtx_);
	m_fsmContext.DeleteJob(pEvt);
	m_status_ = m_fsmContext.getState().getName();
}

void JobFSM::QueryJobStatus(const sdpa::events::QueryJobStatusEvent* pEvt)
{
	lock_type lock(mtx_);
	m_status_ = m_fsmContext.getState().getName();
	m_fsmContext.QueryJobStatus(pEvt);
	m_status_ = m_fsmContext.getState().getName();
}

void JobFSM::JobFinished(const sdpa::events::JobFinishedEvent* pEvt)
{
	lock_type lock(mtx_);
	m_status_ = m_fsmContext.getState().getName();
	m_fsmContext.JobFinished(pEvt);
	m_status_ = m_fsmContext.getState().getName();
}

void JobFSM :: JobFailed(const sdpa::events::JobFailedEvent* pEvt)
{
	lock_type lock(mtx_);
	m_status_ = m_fsmContext.getState().getName();
	m_fsmContext.JobFailed(pEvt);
	m_status_ = m_fsmContext.getState().getName();
}

void  JobFSM ::RetrieveJobResults(const sdpa::events::RetrieveJobResultsEvent* pEvt)
{
	lock_type lock(mtx_);
	m_status_ = m_fsmContext.getState().getName();
	m_fsmContext.RetrieveJobResults(pEvt);
	m_status_ = m_fsmContext.getState().getName();
}
