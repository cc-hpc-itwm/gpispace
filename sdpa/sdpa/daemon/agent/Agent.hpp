// tiberiu.rotaru@itwm.fraunhofer.de

#ifndef SDPA_AGENT_HPP
#define SDPA_AGENT_HPP 1

#include <sdpa/daemon/GenericDaemon.hpp>

namespace sdpa
{
  namespace daemon
  {
    class Agent : public GenericDaemon
    {
    public:
      Agent ( const std::string& name
            , const std::string& url
            , boost::asio::io_service& peer_io_service
            , boost::asio::io_service& kvs_client_io_service
            , std::string kvs_host
            , std::string kvs_port
            , boost::optional<boost::filesystem::path> const& vmem_socket
            , const sdpa::master_info_list_t arrMasterNames
            , const boost::optional<std::pair<std::string, boost::asio::io_service&>>& gui_info
            );

    protected:
      virtual void handleJobFinishedEvent
        (std::string const& source, const sdpa::events::JobFinishedEvent*) override;
      virtual void handleJobFailedEvent
        (std::string const& source, const sdpa::events::JobFailedEvent*) override;

      virtual void handleCancelJobEvent
        (std::string const& source, const sdpa::events::CancelJobEvent*) override;
      virtual void handleCancelJobAckEvent
        (std::string const& source, const sdpa::events::CancelJobAckEvent*) override;
      virtual void handleDeleteJobEvent
        (std::string const&, const sdpa::events::DeleteJobEvent*) override
      {
        throw std::runtime_error("The agent should not call handleDeleteJobEvent!");
      }
    };
  }
}

#endif
