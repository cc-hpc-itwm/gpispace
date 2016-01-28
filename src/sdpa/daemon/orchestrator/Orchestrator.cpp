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
                               , fhg::log::Logger& logger
                               )
      : GenericDaemon ( name
                      , url
                      , std::move (peer_io_service)
                      , boost::none
                      , {}
                      , logger
                      )
      , _event_handler_thread (&Orchestrator::handle_events, this)
    {}
  }
}
