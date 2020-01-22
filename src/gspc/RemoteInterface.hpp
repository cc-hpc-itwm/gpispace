#pragma once

#include <gspc/ErrorOr.hpp>
#include <gspc/Forest.hpp>
#include <gspc/Resource.hpp>
#include <gspc/Worker.hpp>
#include <gspc/comm/runtime_system/remote_interface/protocol.hpp>
#include <gspc/remote_interface/ID.hpp>
#include <gspc/resource/ID.hpp>
#include <gspc/rpc/TODO.hpp>

#include <util-generic/scoped_boost_asio_io_service_with_threads.hpp>

#include <tuple>
#include <unordered_map>

namespace gspc
{
  class RemoteInterface
  {
  public:
    RemoteInterface (remote_interface::ID);

    UniqueForest<std::tuple<Resource, ErrorOr<resource::ID>>>
      add (UniqueForest<Resource> const&);

    Forest<resource::ID, ErrorOr<>> remove (Forest<resource::ID> const&);

    rpc::endpoint const& local_endpoint() const;
    rpc::endpoint worker_endpoint_for_scheduler (resource::ID) const;

  private:
    resource::ID _next_resource_id;

    //! BEGIN Syntax goal:
    //! comm::runtime_system::remote_interface::Server _comm_server;
    rpc::service_dispatcher _service_dispatcher;
    fhg::util::scoped_boost_asio_io_service_with_threads _io_service;

    rpc::service_handler<comm::runtime_system::remote_interface::add> const _add;
    rpc::service_handler<comm::runtime_system::remote_interface::remove> const _remove;
    rpc::service_handler<comm::runtime_system::remote_interface::worker_endpoint_for_scheduler> const _worker_endpoint_for_scheduler;

    rpc::service_socket_provider const _service_socket_provider;
    rpc::service_tcp_provider const _service_tcp_provider;
    rpc::endpoint const _local_endpoint;
    //! END Syntax goal

    //! \note process proxy
    struct WorkerServer
    {
      WorkerServer (Resource const&);

      rpc::endpoint endpoint_for_scheduler() const;

    private:
      Worker _worker;
    };

    std::unordered_map<resource::ID, WorkerServer> _workers;
    std::unordered_map<resource::ID, resource::ID> _proxies;
  };
}
