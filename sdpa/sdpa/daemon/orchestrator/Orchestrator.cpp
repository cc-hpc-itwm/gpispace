// tiberiu.rotaru@itwm.fraunhofer.de

#include <sdpa/daemon/orchestrator/Orchestrator.hpp>

#include <sdpa/daemon/Job.hpp>
#include <sdpa/events/CancelJobEvent.hpp>
#include <sdpa/events/DiscoverJobStatesEvent.hpp>
#include <sdpa/events/DiscoverJobStatesReplyEvent.hpp>
#include <sdpa/job_states.hpp>

namespace sdpa
{
  namespace daemon
  {
    Orchestrator::Orchestrator ( const std::string& name
                               , const std::string& url
                               , std::string kvs_host, std::string kvs_port
                               )
      : GenericDaemon (name, url, kvs_host, kvs_port)
    {}

    std::list<agent_id_t> Orchestrator::subscribers (job_id_t job_id) const
    {
      std::list<agent_id_t> ret;

      BOOST_FOREACH ( const subscriber_map_t::value_type& subscription
                    , m_listSubscribers
                    )
      {
        BOOST_FOREACH (job_id_t id, subscription.second)
        {
          if (id == job_id)
          {
            ret.push_back (subscription.first);
            break;
          }
        }
      }

      return ret;
    }

    void Orchestrator::handleJobFinishedEvent
      (const events::JobFinishedEvent* pEvt)
    {
      LLOG (TRACE, _logger, "The job " << pEvt->job_id() << " has finished!");

      child_proxy (this, pEvt->from()).job_finished_ack (pEvt->job_id());

      Job* pJob (findJob (pEvt->job_id()));
      if (!pJob)
      {
        //! \todo Explain why we can ignore this
        return;
      }

      pJob->JobFinished (pEvt->result());

      BOOST_FOREACH (agent_id_t subscriber, subscribers (pEvt->job_id()))
      {
        sendEventToOther
          ( events::JobFinishedEvent::Ptr
            ( new events::JobFinishedEvent
              (pEvt->from(), subscriber, pEvt->job_id(), pEvt->result())
            )
          );
      }

      try
      {
        scheduler()->worker_manager().findWorker (pEvt->from())->deleteJob (pEvt->job_id());
        request_scheduling();
      }
      catch (WorkerNotFoundException const&)
      {
      }
    }

    void Orchestrator::handleJobFailedEvent (const events::JobFailedEvent* pEvt)
    {
      child_proxy (this, pEvt->from()).job_failed_ack (pEvt->job_id());

      Job* pJob (findJob (pEvt->job_id()));
      if (!pJob)
      {
        //! \todo Explain why we can ignore this
        return;
      }

      pJob->JobFailed (pEvt->error_message());

      BOOST_FOREACH (agent_id_t subscriber, subscribers (pEvt->job_id()))
      {
        sendEventToOther
          ( events::JobFailedEvent::Ptr
            ( new events::JobFailedEvent
              (pEvt->from(), subscriber, pEvt->job_id(), pEvt->error_message())
            )
          );
      }

      try
      {
        scheduler()->worker_manager().findWorker (pEvt->from())->deleteJob (pJob->id());
        request_scheduling();
      }
      catch (const WorkerNotFoundException&)
      {
      }
    }

    void Orchestrator::handleCancelJobEvent (const events::CancelJobEvent* pEvt)
    {
      Job* pJob (findJob (pEvt->job_id()));
      if (!pJob)
      {
        throw std::runtime_error ("CancelJobEvent for unknown job");
      }

      if(pJob->getStatus() == sdpa::status::CANCELING)
      {
        throw std::runtime_error
          ("A cancelation request for this job was already posted!");
      }

      if(sdpa::status::is_terminal (pJob->getStatus()))
      {
        throw std::runtime_error
          ( "Cannot cancel an already terminated job, its current status is: "
          + sdpa::status::show (pJob->getStatus())
          );
      }

      // send immediately an acknowledgment to the component that
      // requested the cancellation
      if (!isSubscriber (pEvt->from()))
      {
        parent_proxy (this, pEvt->from()).cancel_job_ack (pEvt->job_id());
      }

      pJob->CancelJob();

      boost::optional<sdpa::worker_id_t> worker_id =
        scheduler()->worker_manager().findSubmOrAckWorker(pEvt->job_id());
      if (worker_id)
      {
        child_proxy (this, *worker_id).cancel_job (pEvt->job_id());
      }
      else
      {
        // the job was not yet assigned to any worker

        pJob->CancelJobAck();
        ptr_scheduler_->delete_job (pEvt->job_id());

        BOOST_FOREACH (agent_id_t subscriber, subscribers (pEvt->job_id()))
        {
          sendEventToOther
            ( events::CancelJobAckEvent::Ptr
              ( new events::CancelJobAckEvent
                (pEvt->from(), subscriber, pEvt->job_id())
              )
            );
        }
      }
    }

    void Orchestrator::handleCancelJobAckEvent
      (const events::CancelJobAckEvent* pEvt)
    {
      Job* pJob (findJob (pEvt->job_id()));
      if (!pJob)
      {
        //! \todo Explain why we can ignore this
        return;
      }

      pJob->CancelJobAck();

      BOOST_FOREACH (agent_id_t subscriber, subscribers (pEvt->job_id()))
      {
        sendEventToOther
          ( events::CancelJobAckEvent::Ptr
            ( new events::CancelJobAckEvent
              (pEvt->from(), subscriber, pEvt->job_id())
            )
          );
      }
    }

    void Orchestrator::handleDeleteJobEvent (const events::DeleteJobEvent* evt)
    {
      Job* pJob (findJob (evt->job_id()));
      if (!pJob)
      {
        throw std::runtime_error ("DeleteJobEvent for unknown job");
      }

      if(!sdpa::status::is_terminal (pJob->getStatus()))
      {
        throw std::runtime_error
          ( "Cannot delete a job which is in a non-terminal state. "
            "Please, cancel it first!"
          );
      }

      deleteJob (evt->job_id());
      parent_proxy (this, evt->from()).delete_job_ack (evt->job_id());
    }
  }
}
