#include "JobFSM.hpp"
#include <iostream>

using namespace std;
using namespace sdpa::fsm;

JobFSM::JobFSM() : SDPA_INIT_LOGGER("sdpa.fsm.JobFSM"), m_fsmContext(*this), m_nNumberSubJobs(0), m_nCancelAckCounter(0)
{
	SDPA_LOG_DEBUG("State machine created");
}

JobFSM::~JobFSM()
{
	SDPA_LOG_DEBUG("State machine destroyed");
}

bool JobFSM::IsSubJob(sdpa::Job::job_id_t JobID)
{
	return true;
}

int JobFSM::IncGetCancelAckCounter()
{
	SDPA_LOG_DEBUG("Increment CancelAckCounter");
	return ++m_nCancelAckCounter;
}

int JobFSM::GetCancelAckCounter()
{
	return m_nCancelAckCounter;
}

int JobFSM::GetNumberSubJobs()
{
	return m_nNumberSubJobs;
}

int JobFSM::InformWFEJobFailed( sdpa::Job::job_id_t sJobID )
{
	SDPA_LOG_DEBUG("Inform WFE that the job "<<sJobID<<" failed");
	return 0;
}

int JobFSM::GetNextActiveSubJobsListFromWFE( sdpa::Job::job_id_t JobID )
{
	SDPA_LOG_DEBUG("Query WFE for the list of next active jobs");
	SDPA_LOG_DEBUG("Don't forget to assign unique JobIDs");
	return 0;
}  //assign unique global IDs!

int JobFSM::ScheduleJobs()
{
	SDPA_LOG_DEBUG("Schedule job");
	return 0;
}

int JobFSM::DoCancelJob( sdpa::Job::job_id_t JobID )
{
	SDPA_LOG_DEBUG("DoCancelJob");
	return 0;
}

int JobFSM::PostCancelJobAckEventForMaster( sdpa::events::CancelJobEvent& event )
{
	SDPA_LOG_DEBUG("PostCancelJobAckEventForMaster");
	return 0;
}

int JobFSM::PostJobStatusAnswerEventForMaster( sdpa::events::QueryJobStatusEvent& event )
{
	SDPA_LOG_DEBUG("PostJobStatusAnswerEventForMaster");
	return 0;
}

int JobFSM::PostJobFinishedEventForMaster( sdpa::events::JobFinishedEvent& event )
{
	SDPA_LOG_DEBUG("PostJobFinishedEventForMaster");
	return 0;
}

int JobFSM::PostJobFailedEventForMaster( sdpa::events::JobFailedEvent& event)
{
	SDPA_LOG_DEBUG("PostJobFailedEventForMaster");
	return 0;
}

int JobFSM::PostCancelJobAckEventForMaster( sdpa::events::CancelJobAckEvent& event )
{
	SDPA_LOG_DEBUG("PostCancelJobAckEventForMaster");
	return 0;
}

int JobFSM::HandleJobFailure( sdpa::Job::job_id_t JobID )
{
	SDPA_LOG_DEBUG("HandleJobFailure");
	return 0;
}

int JobFSM::DoCancelSubJobs( sdpa::Job::job_id_t JobID )
{
	SDPA_LOG_DEBUG("DoCancelSubJobs");
	return 0;
}// Attention!: some of SubJobs may already have finished!
