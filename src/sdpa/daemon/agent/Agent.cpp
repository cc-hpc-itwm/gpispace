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
  }
}
