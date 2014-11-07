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
                               , boost::asio::io_service& peer_io_service
                               , boost::asio::io_service& kvs_client_io_service
                               , std::string kvs_host, std::string kvs_port
                               , boost::asio::io_service& rpc_io_service
                               )
      : GenericDaemon ( name
                      , url
                      , peer_io_service
                      , kvs_client_io_service
                      , kvs_host, kvs_port
                      , boost::none
                      , {}
                      )
      , _rpc_connections()
      , _rpc_dispatcher (fhg::rpc::exception::serialization_functions())
      , _rpc_acceptor
          ( boost::asio::ip::tcp::endpoint()
          , rpc_io_service
          , [] (fhg::network::buffer_type b) { return b; }
          , [] (fhg::network::buffer_type b) { return b; }
          , [this] ( fhg::network::connection_type* connection
                   , fhg::network::buffer_type buffer
                   )
          {
              _rpc_dispatcher.dispatch (connection, buffer);
            }
          , [this] (fhg::network::connection_type* connection)
            {
              _rpc_connections.erase
                ( std::find_if
                    ( _rpc_connections.begin()
                    , _rpc_connections.end()
                    , [&connection] (std::unique_ptr<fhg::network::connection_type> const& other)
                      {
                        return other.get() == connection;
                      }
                    )
                );
            }
          , [this] (std::unique_ptr<fhg::network::connection_type> connection)
            {
              _rpc_connections.emplace_back (std::move (connection));
            }
          )
    {}

    boost::asio::ip::tcp::endpoint Orchestrator::rpc_local_endpoint() const
    {
      return _rpc_acceptor.local_endpoint();
    }

    std::list<agent_id_t> Orchestrator::subscribers (job_id_t job_id) const
    {
      std::list<agent_id_t> ret;

      for (const subscriber_map_t::value_type& subscription : _subscriptions)
      {
        for (job_id_t id : subscription.second)
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
      (std::string const& source, const events::JobFinishedEvent* pEvt)
    {
      LLOG (TRACE, _logger, "The job " << pEvt->job_id() << " has finished!");

      child_proxy (this, source).job_finished_ack (pEvt->job_id());

      Job* pJob (findJob (pEvt->job_id()));
      if (!pJob)
      {
        //! \todo Explain why we can ignore this
        return;
      }

      pJob->JobFinished (pEvt->result());

      for (agent_id_t subscriber : subscribers (pEvt->job_id()))
      {
        sendEventToOther
          ( subscriber
          , events::JobFinishedEvent::Ptr
            (new events::JobFinishedEvent (pEvt->job_id(), pEvt->result()))
          );
      }

      try
      {
        scheduler().releaseReservation (pEvt->job_id());
        scheduler().worker_manager().findWorker (source)->deleteJob (pEvt->job_id());
        request_scheduling();
      }
      catch (WorkerNotFoundException const&)
      {
      }
    }

    void Orchestrator::handleJobFailedEvent
      (std::string const& source, const events::JobFailedEvent* pEvt)
    {
      child_proxy (this, source).job_failed_ack (pEvt->job_id());

      Job* pJob (findJob (pEvt->job_id()));
      if (!pJob)
      {
        //! \todo Explain why we can ignore this
        return;
      }

      pJob->JobFailed (pEvt->error_message());

      for (agent_id_t subscriber : subscribers (pEvt->job_id()))
      {
        sendEventToOther
          ( subscriber
          , events::JobFailedEvent::Ptr
            (new events::JobFailedEvent (pEvt->job_id(), pEvt->error_message()))
          );
      }

      try
      {
        scheduler().releaseReservation (pEvt->job_id());
        scheduler().worker_manager().findWorker (source)->deleteJob (pJob->id());
        request_scheduling();
      }
      catch (const WorkerNotFoundException&)
      {
      }
    }

    void Orchestrator::handleCancelJobEvent
      (std::string const& source, const events::CancelJobEvent* pEvt)
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
      if (!isSubscriber (source))
      {
        parent_proxy (this, source).cancel_job_ack (pEvt->job_id());
      }

      pJob->CancelJob();

      boost::optional<sdpa::worker_id_t> worker_id =
        scheduler().worker_manager().findSubmOrAckWorker(pEvt->job_id());
      if (worker_id)
      {
        child_proxy (this, *worker_id).cancel_job (pEvt->job_id());
      }
      else
      {
        // the job was not yet assigned to any worker

        pJob->CancelJobAck();
        _scheduler.delete_job (pEvt->job_id());

        for (agent_id_t subscriber : subscribers (pEvt->job_id()))
        {
          sendEventToOther
            ( subscriber
            , events::CancelJobAckEvent::Ptr
              (new events::CancelJobAckEvent (pEvt->job_id()))
            );
        }
      }
    }

    void Orchestrator::handleCancelJobAckEvent
      (std::string const& source, const events::CancelJobAckEvent* pEvt)
    {
      Job* pJob (findJob (pEvt->job_id()));
      if (!pJob)
      {
        //! \todo Explain why we can ignore this
        return;
      }

      pJob->CancelJobAck();

      for (agent_id_t subscriber : subscribers (pEvt->job_id()))
      {
        sendEventToOther
          ( subscriber
          , events::CancelJobAckEvent::Ptr
            (new events::CancelJobAckEvent (pEvt->job_id()))
          );
      }
    }

    void Orchestrator::handleDeleteJobEvent
      (std::string const& source, const events::DeleteJobEvent* evt)
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
      parent_proxy (this, source).delete_job_ack (evt->job_id());
    }
  }
}
