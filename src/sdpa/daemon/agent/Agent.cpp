#include <sdpa/daemon/agent/Agent.hpp>

#include <sdpa/daemon/Job.hpp>
#include <sdpa/events/CancelJobEvent.hpp>
#include <sdpa/events/DiscoverJobStatesEvent.hpp>
#include <sdpa/events/DiscoverJobStatesReplyEvent.hpp>

#include <util-generic/join.hpp>

#include <we/type/activity.hpp>
#include <we/type/net.hpp>

namespace sdpa
{
  namespace daemon
  {
    Agent::Agent ( const std::string& name
                 , const std::string& url
                 , std::unique_ptr<boost::asio::io_service> peer_io_service
                 , boost::optional<boost::filesystem::path> const& virtual_memory_socket
                 , std::vector<name_host_port_tuple> const& masters
                 , const boost::optional<std::pair<std::string, boost::asio::io_service&>>& gui_info
                 , fhg::log::Logger& logger
                 )
      : GenericDaemon ( name
                      , url
                      , std::move (peer_io_service)
                      , virtual_memory_socket
                      , masters
                      , logger
                      , gui_info
                      , true
                      )
      , _event_handler_thread (&Agent::handle_events, this)
    {}

    void Agent::handleCancelJobEvent
      (fhg::com::p2p::address_t const&, const events::CancelJobEvent* pEvt)
    {
      Job* const pJob (findJob (pEvt->job_id()));
      if (!pJob)
      {
        throw std::runtime_error ("CancelJobEvent for unknown job");
      }

      if (pJob->getStatus() == sdpa::status::CANCELING)
      {
        throw std::runtime_error
          ("A cancelation request for this job was already posted!");
      }

      if (sdpa::status::is_terminal (pJob->getStatus()))
      {
        throw std::runtime_error
          ( "Cannot cancel an already terminated job, its current status is: "
          + sdpa::status::show (pJob->getStatus())
          );
      }

      if(pJob->getStatus() == sdpa::status::RUNNING)
      {
          pJob->CancelJob();
          workflowEngine()->cancel (pEvt->job_id());
      }
      else
      {
        job_canceled (pJob);

        deleteJob (pEvt->job_id());
      }
    }
  }
}
