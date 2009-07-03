#include "JobImpl.hpp"

using namespace boost::statechart;

namespace sdpa { namespace daemon {
    JobImpl::JobImpl(const sdpa::job_id_t &id,
                     const sdpa::job_desc_t &desc,
                     const sdpa::daemon::ISendEventHandler* pHandler,
                     const sdpa::job_id_t &parent)
        : id_(id), desc_(desc), parent_(parent), pSendEventHandler(pHandler), b_marked_for_del_(false)
    {}

    JobImpl::~JobImpl() throw () { }

    const sdpa::job_id_t & JobImpl::id() const {
        return id_;
    }

    const sdpa::job_id_t & JobImpl::parent() const {
        return parent_;
    }

    const sdpa::job_desc_t & JobImpl::description() const {
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

    bool JobImpl::is_marked_for_deletion() {
    	return b_marked_for_del_;
    }

    void JobImpl::action_run_job(const sdpa::events::RunJobEvent& e)
    {
    	std::cout <<"Job "<<e.job_id()<<": process 'action_dispatch'"<< std::endl;
    }

    void JobImpl::action_cancel_job(const sdpa::events::CancelJobEvent& e)
    {
    	std::cout <<"Job "<<e.job_id()<<": process 'action_cancel'" << std::endl;
    }

    void JobImpl::action_cancel_job_ack(const sdpa::events::CancelJobAckEvent& e)
    {
    	std::cout <<"Job "<<e.job_id()<<": process 'action_cancel_ack'" << std::endl;
    }

    void JobImpl::action_delete_job(const sdpa::events::DeleteJobEvent& e)
    {
    	std::cout <<"Job "<<e.job_id()<<": process 'action_delete_job'" << std::endl;
    	b_marked_for_del_ = true;

    }

    void JobImpl::action_query_job_status(const sdpa::events::QueryJobStatusEvent& e)
    {
    	std::cout<<"Job "<<e.job_id()<<": process 'action_query_status'"<< std::endl;
    	std::cout<<"Posted an event of type StatusReplyEvent"<<std::endl;
    }

    void JobImpl::action_job_finished(const sdpa::events::JobFinishedEvent& e )
    {
    	std::cout <<"Job "<<e.job_id()<<": process 'action_job_finished'"<< std::endl;
    }

    void JobImpl::action_job_failed(const sdpa::events::JobFailedEvent& e)
    {
    	std::cout <<"Job "<<e.job_id()<<": process 'action_job_failed'"<< std::endl;
    }

    void  JobImpl::action_retrieve_job_results(const sdpa::events::RetrieveResultsEvent& e )
    {
    	std::cout <<"Job "<<e.job_id()<<": process 'action_retrieve_results'"<< std::endl;
    }

	void JobImpl::process_event( const boost::statechart::event_base & e) {
		std::cout <<"Called  'JobImpl ::process_event'"<< std::endl;
	};

	void JobImpl::CancelJob(const sdpa::events::CancelJobEvent& e) {
		std::cout <<"Job "<<e.job_id()<<": call transition  'CancelJob'"<< std::endl;
	}

	void JobImpl::CancelJobAck(const sdpa::events::CancelJobAckEvent& e){
		std::cout <<"Job "<<e.job_id()<<": call transition  'CancelJobAck'"<< std::endl;
	}

	void JobImpl:: DeleteJob(const sdpa::events::DeleteJobEvent& e){
		std::cout <<"Job "<<e.job_id()<<": call transition  'DeleteJob'"<< std::endl;
	}

	void JobImpl::JobFailed(const sdpa::events::JobFailedEvent& e){
		std::cout <<"Job "<<e.job_id()<<": call transition  'JobFailed'"<< std::endl;
	}

	void JobImpl::JobFinished(const sdpa::events::JobFinishedEvent& e){
		std::cout <<"Job "<<e.job_id()<<": call transition  'JobFinished'"<< std::endl;
	}

	void JobImpl::QueryJobStatus(const sdpa::events::QueryJobStatusEvent& e){
		std::cout <<"Job "<<e.job_id()<<": call transition  'QueryJobStatus'"<< std::endl;
	}

	void JobImpl::RetrieveResults(const sdpa::events::RetrieveResultsEvent& e){
		std::cout <<"Job "<<e.job_id()<<": call transition  'RetrieveResultsEvent'"<< std::endl;
	}

	void JobImpl::RunJob(const sdpa::events::RunJobEvent& e){
		std::cout <<"Job "<<e.job_id()<<": call transition  'RunJob'"<< std::endl;
	}

}}
