#pragma once

#include <gspc/ErrorOr.hpp>
#include <gspc/Forest.hpp>
#include <gspc/Resource.hpp>
#include <gspc/comm/runtime_system/resource_manager/Client.hpp>
#include <gspc/remote_interface/ConnectionAndPID.hpp>
#include <gspc/remote_interface/Hostname.hpp>
#include <gspc/remote_interface/ID.hpp>
#include <gspc/remote_interface/Strategy.hpp>
#include <gspc/resource/ID.hpp>
#include <gspc/rpc/TODO.hpp>

#include <util-generic/scoped_boost_asio_io_service_with_threads.hpp>

#include <algorithm>
#include <memory>
#include <tuple>
#include <unordered_map>
#include <unordered_set>

namespace gspc
{
  //! \todo Scoped because ConnectionAndPID is scoped. Is unscoped
  //! actually needed?
  class ScopedRuntimeSystem
  {
  public:
    ScopedRuntimeSystem (comm::runtime_system::resource_manager::Client);

    std::unordered_map
      < remote_interface::Hostname
      , ErrorOr<UniqueForest<std::tuple<Resource, ErrorOr<resource::ID>>>>
      >
      add ( std::unordered_set<remote_interface::Hostname>
          , remote_interface::Strategy
          , UniqueForest<Resource> const&
          ) noexcept;

    Forest<resource::ID>
      add_or_throw  ( std::unordered_set<remote_interface::Hostname> hostnames
                    , remote_interface::Strategy strategy
                    , UniqueForest<Resource> const& resources
                    );

    //! \note Assumes that two connected resource IDs have the same
    //! RemoteInterface ID.
    Forest<resource::ID, ErrorOr<>> remove (Forest<resource::ID> const&);

    rpc::endpoint worker_endpoint_for_scheduler (resource::ID) const;

    //! \todo observation for IML and advanced scheduler
    //! std::string hostname (NodeID) const;
    //! std::string hostname (ResourceID) const;
    //! std::map<std::string /*hostname*/, std::set<ResourceID>>
    //!   resources_by_host() const;

  private:
    comm::runtime_system::resource_manager::Client _resource_manager;

    //! \todo thread count based on parameter or?
    fhg::util::scoped_boost_asio_io_service_with_threads
      _remote_interface_io_service {1};

    remote_interface::ID _next_remote_interface_id;

    //! \todo cleanup, e.g. when last resource using them is
    //! removed.
    std::unordered_map< remote_interface::Hostname
                      , std::unique_ptr<remote_interface::ConnectionAndPID>
                      > _remote_interface_by_hostname;
    std::unordered_map< remote_interface::ID
                      , remote_interface::Hostname
                      > _hostname_by_remote_interface_id;

    remote_interface::ConnectionAndPID*
      remote_interface_by_id (remote_interface::ID) const;

    std::unordered_map
      < remote_interface::Hostname
      , ErrorOr<remote_interface::ConnectionAndPID*>
      >
      remote_interfaces ( std::unordered_set<remote_interface::Hostname>
                        , remote_interface::Strategy
                        ) noexcept;

    //! \todo OPTIMIZE access to hostname via map<hostID, hostName>
  };
}
