#pragma once

#include <gspc/ErrorOr.hpp>
#include <gspc/Forest.hpp>
#include <gspc/Resource.hpp>
#include <gspc/comm/runtime_system/remote_interface/Client.hpp>
#include <gspc/remote_interface/Hostname.hpp>
#include <gspc/remote_interface/ID.hpp>
#include <gspc/remote_interface/Strategy.hpp>
#include <gspc/resource/ID.hpp>

#include <boost/asio/io_service.hpp>

#include <tuple>

namespace gspc
{
  namespace remote_interface
  {
    class ConnectionAndPID
    {
    public:
      ConnectionAndPID ( boost::asio::io_service&
                       , Hostname
                       , Strategy
                       , ID
                       );
      ~ConnectionAndPID();

      ConnectionAndPID() = delete;
      ConnectionAndPID (ConnectionAndPID const&) = delete;
      ConnectionAndPID (ConnectionAndPID&&) = delete;
      ConnectionAndPID& operator= (ConnectionAndPID const&) = delete;
      ConnectionAndPID& operator= (ConnectionAndPID&&) = delete;

      UniqueForest<std::tuple<Resource, ErrorOr<resource::ID>>>
        add (UniqueForest<Resource> const&);
      Forest<resource::ID, ErrorOr<>> remove (Forest<resource::ID> const&);

      rpc::endpoint worker_endpoint_for_scheduler (resource::ID);

      Strategy const& strategy() const;

    private:
      Hostname _hostname;
      Strategy _strategy;
      comm::runtime_system::remote_interface::Client _client;
    };
  }
}
