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
        SDPA_INIT_LOGGER( "Job "+id )
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
    	os<<" process 'action_dispatch'";
    	SDPA_LOG_DEBUG(os.str());
    }

    void JobImpl::action_cancel_job(const sdpa::events::CancelJobEvent& e)
    {
    	ostringstream os;
    	os<<" process 'action_cancel'" << std::endl;
    	SDPA_LOG_DEBUG(os.str());
    }

    void JobImpl::action_cancel_job_ack(const sdpa::events::CancelJobAckEvent& e)
    {
    	ostringstream os;
    	os<<" process 'action_cancel_ack'" << std::endl;
    	SDPA_LOG_DEBUG(os.str());
    }

    void JobImpl::action_delete_job(const sdpa::events::DeleteJobEvent& e)
    {
    	ostringstream os;
    	os<<" process 'action_delete_job'" << std::endl;
    	b_marked_for_del_ = true;
    	SDPA_LOG_DEBUG(os.str());
    }

    void JobImpl::action_query_job_status(const sdpa::events::QueryJobStatusEvent& e)
    {
    	ostringstream os;
    	os<<" process 'action_query_status'"<< std::endl;
    	SDPA_LOG_DEBUG(os.str());

    	os.str("");
    	os<<"Posted an event of type StatusReplyEvent";
    	SDPA_LOG_DEBUG(os.str());
    }

    void JobImpl::action_job_finished(const sdpa::events::JobFinishedEvent& e )
    {
    	ostringstream os;
    	os <<" process 'action_job_finished'"<< std::endl;
    	SDPA_LOG_DEBUG(os.str());
    }

    void JobImpl::action_job_failed(const sdpa::events::JobFailedEvent& e)
    {
    	ostringstream os;
    	os <<" process 'action_job_failed'"<< std::endl;
    	SDPA_LOG_DEBUG(os.str());
    }

    void  JobImpl::action_retrieve_job_results(const sdpa::events::RetrieveResultsEvent& e )
    {
    	ostringstream os;
    	os <<" process 'action_retrieve_results'"<< std::endl;
    	SDPA_LOG_DEBUG(os.str());
    }

	void JobImpl::process_event( const boost::statechart::event_base & e)
	{
		ostringstream os;
		os <<"Called  'JobImpl ::process_event'"<< std::endl;
		SDPA_LOG_DEBUG(os.str());
	};

	void JobImpl::CancelJob(const sdpa::events::CancelJobEvent& e)
	{
		ostringstream os;
		os <<" call transition  'CancelJob'"<< std::endl;
		SDPA_LOG_DEBUG(os.str());
	}

	void JobImpl::CancelJobAck(const sdpa::events::CancelJobAckEvent& e)
	{
		ostringstream os;
		os <<" call transition  'CancelJobAck'"<< std::endl;
		SDPA_LOG_DEBUG(os.str());
	}

	void JobImpl:: DeleteJob(const sdpa::events::DeleteJobEvent& e)
	{
		ostringstream os;
		os <<" call transition  'DeleteJob'"<< std::endl;
		SDPA_LOG_DEBUG(os.str());
	}

	void JobImpl::JobFailed(const sdpa::events::JobFailedEvent& e)
	{
		ostringstream os;
		os <<" call transition  'JobFailed'"<< std::endl;
		SDPA_LOG_DEBUG(os.str());
	}

	void JobImpl::JobFinished(const sdpa::events::JobFinishedEvent& e)
	{
		ostringstream os;
		os <<" call transition  'JobFinished'"<< std::endl;
		SDPA_LOG_DEBUG(os.str());
	}

	void JobImpl::QueryJobStatus(const sdpa::events::QueryJobStatusEvent& e)
	{
		ostringstream os;
		os <<" call transition  'QueryJobStatus'"<< std::endl;
		SDPA_LOG_DEBUG(os.str());
	}

	void JobImpl::RetrieveResults(const sdpa::events::RetrieveResultsEvent& e)
	{
		ostringstream os;
		os <<" call transition  'RetrieveResultsEvent'"<< std::endl;
		SDPA_LOG_DEBUG(os.str());
	}

	void JobImpl::RunJob(const sdpa::events::RunJobEvent& e)
	{
		ostringstream os;
		os <<" call transition  'RunJob'"<< std::endl;
		SDPA_LOG_DEBUG(os.str());
	}

}}
