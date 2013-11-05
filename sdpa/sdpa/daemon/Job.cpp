#include "Job.hpp"

#include <sdpa/events/JobResultsReplyEvent.hpp>
#include <sdpa/events/JobStatusReplyEvent.hpp>

#include <sdpa/events/JobResultsReplyEvent.hpp>
#include <sdpa/events/JobStatusReplyEvent.hpp>
#include <sdpa/events/DeleteJobAckEvent.hpp>
#include <sdpa/events/CancelJobEvent.hpp>

#include <seda/Stage.hpp>
#include <seda/StageRegistry.hpp>

#include <sdpa/util/Properties.hpp>
#include <sdpa/daemon/GenericDaemon.hpp>

#include <limits>

using namespace std;
using namespace sdpa::events;

namespace sdpa {
  namespace daemon {
    Job::Job(const sdpa::job_id_t id,
                     const sdpa::job_desc_t desc,
                     const sdpa::daemon::IAgent* pHandler,
                     const sdpa::job_id_t &parent)
        : SDPA_INIT_LOGGER("Job")
        , id_(id)
        , desc_(desc)
        , parent_(parent)
        , b_marked_for_del_(false)
        , type_(Job::WORKER)
        , result_()
        , m_error_code(0)
        , m_error_message()
        , walltime_(2592000) // walltime in seconds: one month by default
    {}

    Job::~Job() {
      DLOG(TRACE, "Destructor of the job "<<id_.str()<<" called!");
    }


    const sdpa::job_id_t & Job::id() const {
        return id_;
    }

    const sdpa::job_id_t & Job::parent() const {
        return parent_;
    }

    const sdpa::job_desc_t & Job::description() const {
        return desc_;
    }


    bool Job::is_marked_for_deletion() {
        return b_marked_for_del_;
    }

    bool Job::mark_for_deletion() {
        return b_marked_for_del_ = true;
    }

    bool Job::isMasterJob() {
        return type_ == Job::MASTER;
    }

    void Job::setType(const job_type& type) {
        type_ = type;
    }

    void Job::action_run_job()
    {
        DLOG(TRACE, "Process 'action_run_job'");
    }

    // transition from Pending to Cancelled
    void Job::action_cancel_job_from_pending(const sdpa::events::CancelJobEvent& evt)
    {
      DLOG(TRACE, "Process 'action_cancel_job_from_pending'");
    }

    // transition from Cancelling to Cancelled
    void Job::action_cancel_job(const sdpa::events::CancelJobEvent& evt)
    {
        DLOG(TRACE, "cancelling job " << id());
    }

    void Job::action_cancel_job_ack(const sdpa::events::CancelJobAckEvent& evt)
    {
        DLOG(TRACE, "acknowledged cancelling job " << id());
    }

    void Job::action_delete_job(const sdpa::events::DeleteJobEvent& e)
    {
        DLOG(TRACE, "delete job " << id());
        b_marked_for_del_ = true;
    }

    void Job::action_job_finished(const sdpa::events::JobFinishedEvent& evt/* evt */)
    {
        DLOG(TRACE, "The job " << id()<<" finished. Set the result now!");
        setResult(evt.result());
    }

    void Job::action_job_failed(const sdpa::events::JobFailedEvent& evt )
    {
        DLOG(TRACE, "job failed " << id());
        setResult(evt.result());
        error_code(evt.error_code());
        error_message(evt.error_message());
    }

    void  Job::action_retrieve_job_results(const sdpa::events::RetrieveJobResultsEvent& e)
    {
        DLOG(TRACE, "retrieving results of job " << id());
    }
}}
