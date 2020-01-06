#include <gspc/ExecutionEnvironment.hpp>

#include <stdexcept>
#include <utility>

namespace gspc
{
  resource_manager::ID ExecutionEnvironment::add_resource_manager
    ( resource_manager::Connection resource_manager
    )
  {
    return _resource_managers.emplace
      ( _next_resource_manager_id++
      , std::move (resource_manager)
      ).first->first;
  };

  remote_interface::ID ExecutionEnvironment::bootstrap_remote_interface
    ( remote_interface::Hostname hostname
    , remote_interface::Strategy
    )
  {
    auto const remote_interface_id (_next_remote_interface_id++);

    _remote_interfaces.emplace ( remote_interface_id
                  , RemoteInterface{}
                  );

    _remote_interface_ids_by_hostname[hostname].emplace (remote_interface_id);

    return remote_interface_id;
  }

  TreeResult<resource::ID> ExecutionEnvironment::add
    ( remote_interface::ID remote_interface_id
    , resource_manager::ID resource_manager_id
    , Tree<Resource> resources
    )
  {
    auto resource_manager (at (resource_manager_id));
    auto remote_interface (at (remote_interface_id));

    auto const resource_manager_result
      (resource_manager.add (std::move (resources)));

    return remote_interface.add (resource_manager_id, resource_manager_result);
  }

  std::unordered_map< resource_manager::ID
                    , Forest<MaybeError<resource::ID>>
                    > const&
    ExecutionEnvironment::resources
    ( remote_interface::ID remote_interface_id
    ) const
  {
    return at (remote_interface_id).resources();
  }

  std::unordered_map< resource_manager::ID
                    , Forest<MaybeError<resource::ID>>
                    >
    ExecutionEnvironment::resources
    ( remote_interface::Hostname hostname
    ) const
  {
    std::unordered_map< resource_manager::ID
                      , Forest<MaybeError<resource::ID>>
                      > result;

    for (auto const& remote_interface_id : at (hostname))
    {
      for (auto const& resource : resources (remote_interface_id))
      {
        auto& rs (result[resource.first]);

        rs.insert
          (rs.end(), resource.second.begin(), resource.second.end());
      }
    }

    return result;
  }


  RemoteInterface& ExecutionEnvironment::at
    (remote_interface::ID remote_interface_id)
  {
    auto remote_interface (_remote_interfaces.find (remote_interface_id));

    if (remote_interface == _remote_interfaces.end())
    {
      throw std::invalid_argument
        ( "ExecutionEnvironment: Unknown RemoteInterface '"
        + to_string (remote_interface_id)
        + "'"
        );
    }

    return remote_interface->second;
  }
  RemoteInterface const& ExecutionEnvironment::at
    (remote_interface::ID remote_interface_id) const
  {
    return const_cast<ExecutionEnvironment*> (this)->at (remote_interface_id);
  }

  ResourceManager& ExecutionEnvironment::at
    (resource_manager::ID resource_manager_id)
  {
    auto resource_manager (_resource_managers.find (resource_manager_id));

    if (resource_manager == _resource_managers.end())
    {
      throw std::invalid_argument
        ( "ExecutionEnvironment: Unknown ResourceManager '"
        + to_string (resource_manager_id)
        + "'"
        );
    }

    return resource_manager->second.get();
  }
  ResourceManager const& ExecutionEnvironment::at
    (resource_manager::ID resource_manager_id) const
  {
    return const_cast<ExecutionEnvironment*> (this)->at (resource_manager_id);
  }

  std::unordered_set<remote_interface::ID>& ExecutionEnvironment::at
    (remote_interface::Hostname hostname)
  {
    auto remote_interface_ids
      (_remote_interface_ids_by_hostname.find (hostname));

    if (remote_interface_ids == _remote_interface_ids_by_hostname.end())
    {
      throw std::invalid_argument
        ( "ExecutionEnvironment: Unknown host '"
        + hostname
        + "'"
        );
    }

    return remote_interface_ids->second;
  }
  std::unordered_set<remote_interface::ID> const& ExecutionEnvironment::at
    (remote_interface::Hostname hostname) const
  {
    return const_cast<ExecutionEnvironment*> (this)->at (std::move (hostname));
  }
}
