#include "JobFSM.hpp"

using namespace sdpa::fsm;

JobFSM::JobFSM() : m_fsmContext(*this), m_nNumberSubJobs(0), m_nCancelAckCounter(0){}
JobFSM::~JobFSM(){}

bool JobFSM::IsSubJob(sdpa::Job::job_id_t JobID) { return true; }
int JobFSM::IncGetCancelAckCounter() { return ++m_nCancelAckCounter; }
int JobFSM::GetCancelAckCounter() { return m_nCancelAckCounter;}
int JobFSM::GetNumberSubJobs() { return m_nNumberSubJobs; }

int JobFSM::InformWFEJobFailed( sdpa::Job::job_id_t JobID ){ return 0; }
int JobFSM::GetNextActiveSubJobsListFromWFE( sdpa::Job::job_id_t JobID ){ return 0; }  //assign unique global IDs!
int JobFSM::ScheduleJobs(){ return 0; }
int JobFSM::DoCancelJob( sdpa::Job::job_id_t JobID ){ return 0; }
int JobFSM::PostCancelJobAckEventForMaster( sdpa::events::CancelJobEvent& event ){ return 0; }
int JobFSM::PostJobStatusAnswerEventForMaster( sdpa::events::QueryJobStatusEvent& event ){ return 0; }
int JobFSM::PostJobFinishedEventForMaster( sdpa::events::JobFinishedEvent& event ){ return 0; }
int JobFSM::PostJobFailedEventForMaster( sdpa::events::JobFailedEvent& event){ return 0; }
int JobFSM::PostCancelJobAckEventForMaster( sdpa::events::CancelJobAckEvent& event ) { return 0; }

int JobFSM::HandleJobFailure( sdpa::Job::job_id_t JobID ){ return 0; }
int JobFSM::DoCancelSubJobs( sdpa::Job::job_id_t JobID ){ return 0; }// Attention!: some of SubJobs may already have finished!
