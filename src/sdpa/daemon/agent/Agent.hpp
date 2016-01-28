#pragma once

#include <sdpa/daemon/GenericDaemon.hpp>

namespace sdpa
{
  namespace daemon
  {
    class Agent final : public GenericDaemon
    {
    public:
      Agent ( const std::string& name
            , const std::string& url
            , std::unique_ptr<boost::asio::io_service> peer_io_service
            , boost::optional<boost::filesystem::path> const& vmem_socket
            , std::vector<name_host_port_tuple> const&
            , const boost::optional<std::pair<std::string, boost::asio::io_service&>>& gui_info
            , fhg::log::Logger&
            );

    protected:
      virtual void handleCancelJobEvent
        (fhg::com::p2p::address_t const& source, const sdpa::events::CancelJobEvent*) override;

    private:
      boost::strict_scoped_thread<boost::interrupt_and_join_if_joinable>
        _event_handler_thread;
    };
  }
}
