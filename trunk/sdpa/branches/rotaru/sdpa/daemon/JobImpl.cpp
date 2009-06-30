#include "JobImpl.hpp"

namespace sdpa { namespace daemon {
    JobImpl::JobImpl(const Job::job_id_t &id,
                     const Job::job_desc_t &desc,
                     const Job::job_id_t &parent)
        : id_(id), desc_(desc), parent_(parent)
    {}

    JobImpl::~JobImpl() throw () { }

    const Job::job_id_t & JobImpl::id() const {
        return id_;
    }

    const Job::job_id_t & JobImpl::parent() const {
        return parent_;
    }

    const Job::job_desc_t & JobImpl::description() const {
        return desc_;
    }

    const Job::data_t & JobImpl::input() const {
        return input_;
    }
    const Job::data_t & JobImpl::output() const {
        return output_;
    }

    void JobImpl::add_input(const Job::value_t & v) {
        input_.push_back(v);
    }

    void JobImpl::add_output(const Job::value_t & v) {
        output_.push_back(v);
    }

    void JobImpl::add_subjob(const Job::ptr_t & job) {
        subjobs_.insert(std::make_pair(job->id(), job));
    }

    Job::ptr_t JobImpl::get_subjob(const job_id_t & jid) {
        return subjobs_[jid];
    }


    void JobImpl::action_run_job(const sdpa::events::RunJobEvent& e)
    {
    	std::cout <<"Job "<<e.job_id()<<": process action 'action_dispatch'"<< std::endl;
    }

    void JobImpl::action_cancel_job(const sdpa::events::CancelJobEvent& e)
    {
    	std::cout <<"Job "<<e.job_id()<<": process action 'action_cancel'" << std::endl;
    }

    void JobImpl::action_cancel_job_ack(const sdpa::events::CancelJobAckEvent& e)
    {
    	std::cout <<"Job "<<e.job_id()<<": process action 'action_cancel_ack'" << std::endl;
    }

    void JobImpl::action_query_job_status(const sdpa::events::QueryJobStatusEvent& e)
    {
    	std::cout<<"Job "<<e.job_id()<<": process action 'action_query_status'"<< std::endl;
    	std::cout<<"Posted an event of type StatusReplyEvent"<<std::endl;
    }

    void JobImpl::action_job_finished(const sdpa::events::JobFinishedEvent& e )
    {
    	std::cout <<"Job "<<e.job_id()<<": process action action_job_finished"<< std::endl;
    }

    void JobImpl :: action_job_failed(const sdpa::events::JobFailedEvent& e)
    {
    	std::cout <<"Job "<<e.job_id()<<": process action 'action_job_failed'"<< std::endl;
    }

    void  JobImpl ::action_retrieve_job_results(const sdpa::events::RetriveResultsEvent& e )
    {
    	std::cout <<"Job "<<e.job_id()<<": process action 'action_retrieve_results'"<< std::endl;
    }

    void JobImpl :: WFE_NotifyNewJob(){ std::cout<<"WFE_NotifyNewJob"<<std::endl; }

    void JobImpl :: WFE_GenListNextActiveSubJobs(){ std::cout<<"WFE_GenListNextActiveSubJobs"<<std::endl; } //assign unique global IDs!

    void JobImpl :: WFE_NotifyJobFailed(){ std::cout<<"WFE_NotifyJobFailed"<<std::endl; }

}}
