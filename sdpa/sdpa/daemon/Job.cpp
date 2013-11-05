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

    void Job::action_delete_job(const sdpa::events::DeleteJobEvent& e)
    {
        b_marked_for_del_ = true;
    }

    void Job::action_job_finished(const sdpa::events::JobFinishedEvent& evt/* evt */)
    {
        setResult(evt.result());
    }

    void Job::action_job_failed(const sdpa::events::JobFailedEvent& evt )
    {
        setResult(evt.result());
        error_code(evt.error_code());
        error_message(evt.error_message());
    }

    void JobFSM::DeleteJob(const sdpa::events::DeleteJobEvent* pEvt, sdpa::daemon::IAgent*  ptr_comm)
    {
      assert (ptr_comm);
      lock_type lock(mtx_);
      process_event(*pEvt);

      sdpa::events::DeleteJobAckEvent::Ptr pDelJobReply(new sdpa::events::DeleteJobAckEvent(pEvt->to(), pEvt->from(), id(), pEvt->id()) );
      //send ack to master
      ptr_comm->sendEventToMaster(pDelJobReply);
    }

    void JobFSM::QueryJobStatus(const sdpa::events::QueryJobStatusEvent* pEvt, sdpa::daemon::IAgent* pDaemon )
    {
      assert (pDaemon);
      // attention, no action called!
      lock_type const _ (mtx_);
      process_event (*pEvt);

      sdpa::events::JobStatusReplyEvent::Ptr const pStatReply
        (new sdpa::events::JobStatusReplyEvent ( pEvt->to()
                                               , pEvt->from()
                                               , id()
                                               , getStatus()
                                               , error_code()
                                               , error_message()
                                               )
        );

      pDaemon->sendEventToMaster (pStatReply);
    }

    void JobFSM::RetrieveJobResults(const sdpa::events::RetrieveJobResultsEvent* pEvt, sdpa::daemon::IAgent* ptr_comm)
    {
      assert (ptr_comm);
      lock_type lock(mtx_);
      process_event(*pEvt);
      const sdpa::events::JobResultsReplyEvent::Ptr pResReply( new sdpa::events::JobResultsReplyEvent( pEvt->to(), pEvt->from(), id(), result() ));

      // reply the results to master
      ptr_comm->sendEventToMaster(pResReply);
    }

    void JobFSM::Reschedule(sdpa::daemon::IAgent*  pAgent)
    {
      MSMRescheduleEvent ReschedEvt;
      lock_type lock(mtx_);
      process_event(ReschedEvt);
      pAgent->schedule(id());
    }
}}
