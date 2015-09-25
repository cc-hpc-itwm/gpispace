#pragma once

#include <rif/entry_point.hpp>
#include <rif/protocol.hpp>

#include <rpc/client.hpp>

namespace fhg
{
  namespace rif
  {
    class client
    {
    public:
      client (fhg::rif::entry_point const& entry_point)
        : _endpoint (entry_point.hostname, entry_point.port)
        , execute_and_get_startup_messages (_endpoint)
        , execute_and_get_startup_messages_and_wait (_endpoint)
        , kill (_endpoint)
        , start_vmem (_endpoint)
      {}

    private:
      fhg::rpc::remote_endpoint _endpoint;

    public:
      rpc::remote_function<protocol::execute_and_get_startup_messages>
        execute_and_get_startup_messages;
      rpc::remote_function<protocol::execute_and_get_startup_messages_and_wait>
        execute_and_get_startup_messages_and_wait;
      rpc::remote_function<protocol::kill> kill;
      rpc::remote_function<protocol::start_vmem> start_vmem;
    };
  }
}
