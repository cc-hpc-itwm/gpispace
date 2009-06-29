#include <sdpa/jobFSM/JobFSMActions.hpp>

using namespace sdpa::fsm;

void JobFSMActions::action_run_job(const sdpa::events::RunJobEvent& e)
{
	std::cout <<"Job "<<e.job_id()<<": process action 'action_dispatch'"<< std::endl;
}

void JobFSMActions::action_cancel_job(const sdpa::events::CancelJobEvent& e)
{
	std::cout <<"Job "<<e.job_id()<<": process action 'action_cancel'" << std::endl;
}

void JobFSMActions::action_cancel_job_ack(const sdpa::events::CancelJobAckEvent& e)
{
	std::cout <<"Job "<<e.job_id()<<": process action 'action_cancel_ack'" << std::endl;
}

void JobFSMActions::action_query_job_status(const sdpa::events::QueryJobStatusEvent& e)
{
	std::cout<<"Job "<<e.job_id()<<": process action 'action_query_status'"<< std::endl;
	std::cout<<"Posted an event of type StatusReplyEvent"<<std::endl;
}

void JobFSMActions::action_job_finished(const sdpa::events::JobFinishedEvent& e )
{
	std::cout <<"Job "<<e.job_id()<<": process action action_job_finished"<< std::endl;
}

void JobFSMActions :: action_job_failed(const sdpa::events::JobFailedEvent& e)
{
	std::cout <<"Job "<<e.job_id()<<": process action 'action_job_failed'"<< std::endl;
}

void  JobFSMActions ::action_retrieve_job_results(const sdpa::events::RetriveResultsEvent& e )
{
	std::cout <<"Job "<<e.job_id()<<": process action 'action_retrieve_results'"<< std::endl;
}

void JobFSMActions :: WFE_NotifyNewJob(){ std::cout<<"WFE_NotifyNewJob"<<std::endl; }

void JobFSMActions :: WFE_GenListNextActiveSubJobs(){ std::cout<<"WFE_GenListNextActiveSubJobs"<<std::endl; } //assign unique global IDs!

void JobFSMActions :: WFE_NotifyJobFailed(){ std::cout<<"WFE_NotifyJobFailed"<<std::endl; }
