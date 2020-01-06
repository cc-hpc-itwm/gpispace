#pragma once

#include <gspc/MaybeError.hpp>
#include <gspc/RemoteInterface.hpp>
#include <gspc/Resource.hpp>
#include <gspc/ResourceManager.hpp>
#include <gspc/Tree.hpp>

#include <unordered_map>
#include <unordered_set>

namespace gspc
{
  struct ExecutionEnvironment
  {
    resource_manager::ID add_resource_manager
      ( resource_manager::Connection
      );

    //! pre: no managed active resources
    void remove_resource_manager
      ( resource_manager::ID
      );

    //! post: no active resources
    remote_interface::ID bootstrap_remote_interface
      ( remote_interface::Hostname hostname
      , remote_interface::Strategy strategy
      );

    //! pre: no active resources
    void teardown_remote_interface
      ( remote_interface::ID
      );

    //! post: \a resources are added to \a remote_interface_id, are active
    //! and managed by \a resource_manager
    //! returned tree has the same structure as the input tree
    TreeResult<resource::ID> add
      ( remote_interface::ID remote_interface_id
      , resource_manager::ID resource_manager_id
      , Tree<Resource> resources
      );
    //! \todo add complete forests at once
    Forest<MaybeError<resource::ID>> add
      ( remote_interface::ID remote_interface_id
      , resource_manager::ID resource_manager_id
      , Forest<Resource> resources
      );

    //! post: resources at \a remote_interface_id and managed by \a
    //! resource_manager_id are not active
    //! returned tree has the same structure as the input tree
    TreeResult<resource::ID> remove
      ( remote_interface::ID remote_interface_id
      , resource_manager::ID resource_manager_id
      , Tree<resource::ID> resources
      );

    //! list resources at \a remote_interface_id
    std::unordered_map< resource_manager::ID
                      , Forest<MaybeError<resource::ID>>
                      > const&
      resources
      ( remote_interface::ID remote_interface_id
      ) const;

    //! list resources at \a hostname
    std::unordered_map< resource_manager::ID
                      , Forest<MaybeError<resource::ID>>
                      > resources
      ( remote_interface::Hostname hostname
      ) const;

  private:
    remote_interface::ID _next_remote_interface_id {0};
    resource_manager::ID _next_resource_manager_id {0};

    std::unordered_map<remote_interface::ID, RemoteInterface>
      _remote_interfaces;
    std::unordered_map< remote_interface::Hostname
                      , std::unordered_set<remote_interface::ID>
                      > _remote_interface_ids_by_hostname;
    std::unordered_map<resource_manager::ID, resource_manager::Connection>
      _resource_managers;

    RemoteInterface const& at (remote_interface::ID) const;
    ResourceManager const& at (resource_manager::ID) const;
    std::unordered_set<remote_interface::ID> const& at (remote_interface::Hostname) const;

    RemoteInterface& at (remote_interface::ID);
    ResourceManager& at (resource_manager::ID);
    std::unordered_set<remote_interface::ID>& at (remote_interface::Hostname);
  };
}
