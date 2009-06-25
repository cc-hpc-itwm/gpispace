#include "JobFSM.hpp"
#include <iostream>

using namespace std;
using namespace sdpa::fsm;
using namespace sdpa::events;

JobFSM::JobFSM() : SDPA_INIT_LOGGER("sdpa.fsm.JobFSM"), m_fsmContext(*this)
{
	SDPA_LOG_DEBUG("State machine created");
}

JobFSM::~JobFSM()
{
	SDPA_LOG_DEBUG("State machine destroyed");
}

void JobFSM::action_dispatch(const RunJobEvent& e)
{
	cout <<"Job "<<e.job_id()<<": process action 'action_dispatch'"<< endl;
}

void JobFSM::action_cancel(const CancelJobEvent& e)
{
	cout <<"Job "<<e.job_id()<<": process action 'action_cancel'" << endl;
}

void JobFSM::action_cancel_ack(const CancelJobAckEvent& e)
{
	cout <<"Job "<<e.job_id()<<": process action 'action_cancel_ack'" << endl;
}

void JobFSM::action_query_status(const QueryJobStatusEvent& e)
{
	cout<<"Job "<<e.job_id()<<": process action 'action_query_status'"<< endl;
	cout<<"Posted an event of type StatusReplyEvent"<<endl;
}

void JobFSM::action_job_finished(const JobFinishedEvent& e )
{
	cout <<"Job "<<e.job_id()<<": process action action_job_finished"<< endl;
}

void JobFSM :: action_job_failed(const JobFailedEvent& e)
{
	cout <<"Job "<<e.job_id()<<": process action 'action_job_failed'"<< endl;
}

void  JobFSM ::action_retrieve_results(const RetriveResultsEvent& e )
{
	cout <<"Job "<<e.job_id()<<": process action 'action_retrieve_results'"<< endl;
}

void JobFSM :: WFE_NotifyNewJob(){ cout<<"WFE_NotifyNewJob"<<endl; }

void JobFSM :: WFE_GenListNextActiveSubJobs(){ cout<<"WFE_GenListNextActiveSubJobs"<<endl; } //assign unique global IDs!

void JobFSM :: WFE_NotifyJobFailed(){ cout<<"WFE_NotifyJobFailed"<<endl; }

