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
        , start_vmem_initial_setup_and_wait_for_local_comm_port (_endpoint)
        , continue_vmem_set_nodes_and_wait_for_startup (_endpoint)
      {}

    private:
      fhg::rpc::remote_tcp_endpoint _endpoint;

    public:
      rpc::remote_function<protocol::execute_and_get_startup_messages>
        execute_and_get_startup_messages;
      rpc::remote_function<protocol::execute_and_get_startup_messages_and_wait>
        execute_and_get_startup_messages_and_wait;
      rpc::remote_function<protocol::kill> kill;
      rpc::remote_function<protocol::start_vmem_initial_setup_and_wait_for_local_comm_port>
        start_vmem_initial_setup_and_wait_for_local_comm_port;
      rpc::remote_function<protocol::continue_vmem_set_nodes_and_wait_for_startup>
        continue_vmem_set_nodes_and_wait_for_startup;
    };
  }
}
