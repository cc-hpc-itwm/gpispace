#include "JobImpl.hpp"
#include <sstream>

using namespace std;
using namespace boost::statechart;

namespace sdpa { namespace daemon {
    JobImpl::JobImpl(const sdpa::job_id_t &id,
                     const sdpa::job_desc_t &desc,
                     const sdpa::daemon::ISendEvent* pHandler,
                     const sdpa::job_id_t &parent)
        : id_(id), desc_(desc), parent_(parent), pSendEvent(pHandler), b_marked_for_del_(false),
        SDPA_INIT_LOGGER( string("Job ")+ id.str() )
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
    	ostringstream os;
    	os<<"Process 'action_run_job'";
    	SDPA_LOG_DEBUG(os.str());
    }

    void JobImpl::action_cancel_job(const sdpa::events::CancelJobEvent& e)
    {
    	ostringstream os;
    	os<<"Process 'action_cancel_job'" ;
    	//
    	SDPA_LOG_DEBUG(os.str());
    }

    void JobImpl::action_cancel_job_ack(const sdpa::events::CancelJobAckEvent& e)
    {
    	ostringstream os;
    	os<<"Process 'action_cancel_job_ack'" ;
    	SDPA_LOG_DEBUG(os.str());
    }

    void JobImpl::action_delete_job(const sdpa::events::DeleteJobEvent& e)
    {
    	ostringstream os;
    	os<<"Process 'action_delete_job'" ;
    	b_marked_for_del_ = true;
    	SDPA_LOG_DEBUG(os.str());
    }

    void JobImpl::action_query_job_status(const sdpa::events::QueryJobStatusEvent& e)
    {
    	ostringstream os;
    	os<<"Process 'action_query_job_status'";
    	SDPA_LOG_DEBUG(os.str());

    	os.str("");
    	os<<"Posted an event of type StatusReplyEvent";
    	SDPA_LOG_DEBUG(os.str());
    }

    void JobImpl::action_job_finished(const sdpa::events::JobFinishedEvent& e )
    {
    	ostringstream os;
    	os <<"Process 'action_job_finished'";
    	SDPA_LOG_DEBUG(os.str());
    }

    void JobImpl::action_job_failed(const sdpa::events::JobFailedEvent& e)
    {
    	ostringstream os;
    	os <<"Process 'action_job_failed'";
    	SDPA_LOG_DEBUG(os.str());
    }

    void  JobImpl::action_retrieve_job_results(const sdpa::events::RetrieveResultsEvent& e )
    {
    	ostringstream os;
    	os <<"Process 'action_retrieve_results'";
    	SDPA_LOG_DEBUG(os.str());
    }

	void JobImpl::process_event( const boost::statechart::event_base & e)
	{
		ostringstream os;
		os <<"Called  'JobImpl ::process_event'";
		SDPA_LOG_DEBUG(os.str());
	};

	void JobImpl::CancelJob(const sdpa::events::CancelJobEvent& e)
	{
		ostringstream os;
		os <<"Call transition  'CancelJob'";
		SDPA_LOG_DEBUG(os.str());
	}

	void JobImpl::CancelJobAck(const sdpa::events::CancelJobAckEvent& e)
	{
		ostringstream os;
		os <<"Call transition  'CancelJobAck'";
		SDPA_LOG_DEBUG(os.str());
	}

	void JobImpl:: DeleteJob(const sdpa::events::DeleteJobEvent& e)
	{
		ostringstream os;
		os <<"Call transition  'DeleteJob'";
		SDPA_LOG_DEBUG(os.str());
	}

	void JobImpl::JobFailed(const sdpa::events::JobFailedEvent& e)
	{
		ostringstream os;
		os <<"Call transition  'JobFailed'";
		SDPA_LOG_DEBUG(os.str());
	}

	void JobImpl::JobFinished(const sdpa::events::JobFinishedEvent& e)
	{
		ostringstream os;
		os <<"Call transition  'JobFinished'";
		SDPA_LOG_DEBUG(os.str());
	}

	void JobImpl::QueryJobStatus(const sdpa::events::QueryJobStatusEvent& e)
	{
		ostringstream os;
		os <<"Call transition  'QueryJobStatus'";
		SDPA_LOG_DEBUG(os.str());
	}

	void JobImpl::RetrieveResults(const sdpa::events::RetrieveResultsEvent& e)
	{
		ostringstream os;
		os <<"Call transition  'RetrieveResultsEvent'";
		SDPA_LOG_DEBUG(os.str());
	}

	void JobImpl::RunJob(const sdpa::events::RunJobEvent& e)
	{
		ostringstream os;
		os <<"Call transition  'RunJob'";
		SDPA_LOG_DEBUG(os.str());
	}

}}
