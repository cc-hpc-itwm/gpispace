/*
 * =====================================================================================
 *
 *       Filename:  JobImpl.cpp
 *
 *    Description:  Job implementation
 *
 *        Version:  1.0
 *        Created:
 *       Revision:  none
 *       Compiler:  gcc
 *
 *         Author:  Dr. Tiberiu Rotaru, tiberiu.rotaru@itwm.fraunhofer.de
 *        Company:  Fraunhofer ITWM
 *
 * =====================================================================================
 */
#include "JobImpl.hpp"

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
    JobImpl::JobImpl(const sdpa::job_id_t id,
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

    JobImpl::~JobImpl() {
      DLOG(TRACE, "Destructor of the job "<<id_.str()<<" called!");
    }


    const sdpa::job_id_t & JobImpl::id() const {
        return id_;
    }

    const sdpa::job_id_t & JobImpl::parent() const {
        return parent_;
    }

    const sdpa::job_desc_t & JobImpl::description() const {
        return desc_;
    }


    bool JobImpl::is_marked_for_deletion() {
        return b_marked_for_del_;
    }

    bool JobImpl::mark_for_deletion() {
        return b_marked_for_del_ = true;
    }

    bool JobImpl::isMasterJob() {
        return type_ == Job::MASTER;
    }

    void JobImpl::setType(const job_type& type) {
        type_ = type;
    }

    void JobImpl::action_run_job()
    {
        DLOG(TRACE, "Process 'action_run_job'");
    }

    // transition from Pending to Cancelled
    void JobImpl::action_cancel_job_from_pending(const sdpa::events::CancelJobEvent& evt)
    {
      DLOG(TRACE, "Process 'action_cancel_job_from_pending'");
    }

    // transition from Cancelling to Cancelled
    void JobImpl::action_cancel_job(const sdpa::events::CancelJobEvent& evt)
    {
        DLOG(TRACE, "cancelling job " << id());
    }

    void JobImpl::action_cancel_job_ack(const sdpa::events::CancelJobAckEvent& evt)
    {
        DLOG(TRACE, "acknowledged cancelling job " << id());
    }

    void JobImpl::action_delete_job(const sdpa::events::DeleteJobEvent& e)
    {
        DLOG(TRACE, "delete job " << id());
        b_marked_for_del_ = true;
    }

    void JobImpl::action_job_finished(const sdpa::events::JobFinishedEvent& evt/* evt */)
    {
        DLOG(TRACE, "The job " << id()<<" finished. Set the result now!");
        setResult(evt.result());
    }

    void JobImpl::action_job_failed(const sdpa::events::JobFailedEvent& evt )
    {
        DLOG(TRACE, "job failed " << id());
        setResult(evt.result());
        error_code(evt.error_code());
        error_message(evt.error_message());
    }

    void  JobImpl::action_retrieve_job_results(const sdpa::events::RetrieveJobResultsEvent& e)
    {
        DLOG(TRACE, "retrieving results of job " << id());
    }
}}
