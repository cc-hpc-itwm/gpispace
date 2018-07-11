#pragma once

#include <rif/entry_point.hpp>
#include <rif/protocol.hpp>

#include <rpc/remote_tcp_endpoint.hpp>
#include <rpc/remote_function.hpp>

namespace fhg
{
  namespace rif
  {
    class client
    {
    public:
      client ( boost::asio::io_service& io_service
             , fhg::rif::entry_point const& entry_point
             )
        : _endpoint (io_service, entry_point.hostname, entry_point.port)
        , execute_and_get_startup_messages (_endpoint)
        , execute_and_get_startup_messages_and_wait (_endpoint)
        , kill (_endpoint)
        , start_vmem (_endpoint)
        , start_agent (_endpoint)
      {}

    private:
      fhg::rpc::remote_tcp_endpoint _endpoint;

    public:
      rpc::remote_function<protocol::execute_and_get_startup_messages>
        execute_and_get_startup_messages;
      rpc::remote_function<protocol::execute_and_get_startup_messages_and_wait>
        execute_and_get_startup_messages_and_wait;
      rpc::remote_function<protocol::kill> kill;
      rpc::remote_function<protocol::start_vmem> start_vmem;
      rpc::remote_function<protocol::start_agent> start_agent;
    };
  }
}
