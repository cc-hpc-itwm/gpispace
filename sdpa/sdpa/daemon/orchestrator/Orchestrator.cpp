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
                               , std::unique_ptr<boost::asio::io_service> peer_io_service
                               , boost::asio::io_service& rpc_io_service
                               , fhg::log::Logger& logger
                               )
      : GenericDaemon ( name
                      , url
                      , std::move (peer_io_service)
                      , boost::none
                      , {}
                      , logger
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

    void Orchestrator::handleJobFinishedEvent
      (fhg::com::p2p::address_t const& source, const events::JobFinishedEvent* pEvt)
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

      notify_subscribers<events::JobFinishedEvent>
        (pEvt->job_id(), pEvt->job_id(), pEvt->result());

      scheduler().releaseReservation (pEvt->job_id());
      request_scheduling();
    }

    void Orchestrator::handleJobFailedEvent
      (fhg::com::p2p::address_t const& source, const events::JobFailedEvent* pEvt)
    {
      child_proxy (this, source).job_failed_ack (pEvt->job_id());

      Job* pJob (findJob (pEvt->job_id()));
      if (!pJob)
      {
        //! \todo Explain why we can ignore this
        return;
      }

      pJob->JobFailed (pEvt->error_message());

      notify_subscribers<events::JobFailedEvent>
        (pEvt->job_id(), pEvt->job_id(), pEvt->error_message());

      scheduler().releaseReservation (pEvt->job_id());
      request_scheduling();
    }

    void Orchestrator::handleCancelJobEvent
      (fhg::com::p2p::address_t const& source, const events::CancelJobEvent* pEvt)
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
        child_proxy (this, scheduler().worker_manager().address_by_worker (*worker_id).get()->second)
          .cancel_job (pEvt->job_id());
      }
      else
      {
        // the job was not yet assigned to any worker

        pJob->CancelJobAck();
        _scheduler.delete_job (pEvt->job_id());

        notify_subscribers<events::CancelJobAckEvent>
          (pEvt->job_id(), pEvt->job_id());
      }
    }

    void Orchestrator::handleCancelJobAckEvent
      (fhg::com::p2p::address_t const&, const events::CancelJobAckEvent* pEvt)
    {
      Job* pJob (findJob (pEvt->job_id()));
      if (!pJob)
      {
        //! \todo Explain why we can ignore this
        return;
      }

      pJob->CancelJobAck();

      notify_subscribers<events::CancelJobAckEvent>
        (pEvt->job_id(), pEvt->job_id());
    }

    void Orchestrator::handleDeleteJobEvent
      (fhg::com::p2p::address_t const& source, const events::DeleteJobEvent* evt)
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
