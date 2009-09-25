#include "JobImpl.hpp"
#include <sstream>

#include <sdpa/events/JobResultsReplyEvent.hpp>
#include <sdpa/events/JobStatusReplyEvent.hpp>

#include <sdpa/events/JobResultsReplyEvent.hpp>
#include <sdpa/events/JobStatusReplyEvent.hpp>
#include <sdpa/events/DeleteJobAckEvent.hpp>

#include <seda/Stage.hpp>
#include <seda/StageRegistry.hpp>

using namespace std;
using namespace boost::statechart;
using namespace sdpa::events;

namespace sdpa { namespace daemon {
    JobImpl::JobImpl(const sdpa::job_id_t &id,
                     const sdpa::job_desc_t &desc,
                     const sdpa::daemon::ISendEvent* pHandler,
                     const sdpa::job_id_t &parent)
        : id_(id), desc_(desc), parent_(parent), pSendEvent(const_cast<ISendEvent*>(pHandler)), b_marked_for_del_(false), b_local_(false),
        SDPA_INIT_LOGGER( string("Job ")+ id.str() )
    {}

    JobImpl::~JobImpl() throw () { }

    const sdpa::job_id_t & JobImpl::id() const {
        return id_;
    }

    const sdpa::job_id_t & JobImpl::parent() const {
        return parent_;
    }

    const sdpa::wf::workflow_id_t& JobImpl::workflow_id() const {
    	return workflow_id_;
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

    bool JobImpl::is_local() {
      	return b_local_;
    }

    void JobImpl::set_local(bool b_val) {
    	b_local_ = b_val;
    }

    void JobImpl::action_run_job(const sdpa::events::SubmitJobEvent& event)
    {
    	ostringstream os;
    	os<<"Process 'action_run_job'";
    	SDPA_LOG_DEBUG(os.str());
    }

    void JobImpl::action_cancel_job(const sdpa::events::CancelJobEvent& event)
    {
    	ostringstream os;
    	os<<"Process 'action_cancel_job'" ;
    	// cancel the job
    	SDPA_LOG_DEBUG(os.str());
    }

    void JobImpl::action_cancel_job_ack(const sdpa::events::CancelJobAckEvent& event)
    {
    	ostringstream os;
    	os<<"Process 'action_cancel_job_ack'" ;
    	// Notify WFE that the job e.job_id() was canceled (send a CancelJobAckEvent event to the stage WFE)
    	SDPA_LOG_DEBUG(os.str());
    }

    void JobImpl::action_delete_job(const sdpa::events::DeleteJobEvent& e)
    {
    	ostringstream os;
    	os<<"Process 'action_delete_job'" ;
    	b_marked_for_del_ = true;

    	DeleteJobAckEvent::Ptr pDelJobReply(new DeleteJobAckEvent(e.to(), e.from(), id()) );
    	//send the event
    	pSendEvent->sendEvent(pSendEvent->output_stage(), pDelJobReply);

    	SDPA_LOG_DEBUG(os.str());
    }

    void JobImpl::action_query_job_status(const sdpa::events::QueryJobStatusEvent& e)
    {
    	ostringstream os;
    	os<<"Process 'action_query_job_status'";
    	SDPA_LOG_DEBUG(os.str());

    	JobStatusReplyEvent::status_t status(""); //typeid(GetContext().getState()).name());

    	// Post a JobStatusReplyEvent to e.from()
		JobStatusReplyEvent::Ptr pStatReply(new JobStatusReplyEvent(e.to(), e.from(), id(), status));

		// send the event
		pSendEvent->sendEvent(pSendEvent->output_stage(), pStatReply);

    	os.str("");
    	os<<"Posted an event of type StatusReplyEvent";
    	SDPA_LOG_DEBUG(os.str());
    }

    void JobImpl::action_job_finished(const sdpa::events::JobFinishedEvent& event)
    {
    	ostringstream os;
    	os <<"Process 'action_job_finished'";
    	// inform WFE (send a JobFinishedEvent event to the stage WFE)
    	// obsolete: post a JobFinishedAckEvent to e.from()
    	SDPA_LOG_DEBUG(os.str());
    }

    void JobImpl::action_job_failed(const sdpa::events::JobFailedEvent& event)
    {
    	ostringstream os;
    	os <<"Process 'action_job_failed'";
    	// inform WFE (send a JobFailedEvent event to the stage WFE)
    	// obsolete: post a JobFailedAckEvent to e.from()
    	SDPA_LOG_DEBUG(os.str());
    }

    void  JobImpl::action_retrieve_job_results(const sdpa::events::RetrieveJobResultsEvent& e)
    {
    	ostringstream os;
    	os <<"Process 'action_retrieve_results'";

    	// fill it here with real results
    	JobResultsReplyEvent::result_t results("");

    	const JobResultsReplyEvent::Ptr pResReply(new JobResultsReplyEvent(e.to(), e.from(), id(), results));

    	// attach to this event the results!

    	// send the event
    	const std::string outstage = pSendEvent->output_stage();
    	pSendEvent->sendEvent(outstage, pResReply);

    	// Post a JobResultsReplyEvent to e.from()
    	SDPA_LOG_DEBUG(os.str());
    }
}}
