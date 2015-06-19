#pragma once

#include <rif/entry_point.hpp>
#include <rif/protocol.hpp>

#include <rpc/simple_client.hpp>

namespace fhg
{
  namespace rif
  {
    class client
    {
    public:
      client (fhg::rif::entry_point const& entry_point)
        : _impl (entry_point.hostname, entry_point.port)
        , execute_and_get_startup_messages (_impl)
        , kill (_impl)
        , start_vmem (_impl)
      {}

    private:
      fhg::rpc::simple_client _impl;

    public:
      rpc::remote_function<protocol::execute_and_get_startup_messages>
        execute_and_get_startup_messages;
      rpc::remote_function<protocol::kill> kill;
      rpc::remote_function<protocol::start_vmem> start_vmem;
    };
  }
}
