#include <sdpa/daemon/resource_manager/ResourceManager.hpp>

#include <util-generic/warning.hpp>

#include <algorithm>
#include <fstream>
#include <iterator>
#include <stdexcept>

namespace gspc
{
  void ResourceManager::add
    ( resource::ID const& resource_id
    , resource::Class const& resource_class
    , Resources::Children const& children
    )
  {
    std::lock_guard<std::mutex> const resources_lock (_resources_guard);

    _resources.insert (resource_id, resource_class, children);

    _resource_usage_by_id.emplace (resource_id, 0);
    _available_resources_by_class[resource_class].emplace (resource_id);
  }

  void ResourceManager::remove (resource::ID const& resource_id)
  {
    std::lock_guard<std::mutex> const resources_lock (_resources_guard);

    if (!_resource_usage_by_id.count (resource_id))
    {
      std::ostringstream osstr;
      osstr << resource_id;
      throw std::invalid_argument ("Unknown resource id: " + osstr.str());
    }

    //Note: if a worker with this resource id dies, the scheduler is aware about
    //it and will re-schedule all tasks on other workers with different resource ids
    if (_resource_usage_by_id.at (resource_id))
    {
      throw std::invalid_argument ("Cannot remove resources that are in use.");
    }

    // remove all parent resources
    _resources.upward_apply
      ( [&] (forest::Node<resource::ID, resource::Class> const& resource)
        {
          auto const removed_node (_resources.remove_leaf (resource.first));
          _resource_usage_by_id.erase (removed_node.first);

          auto available_resources_by_class
            (_available_resources_by_class.find (removed_node.second));
          available_resources_by_class->second.erase (removed_node.first);
          if (available_resources_by_class->second.empty())
          {
            _available_resources_by_class.erase
              (available_resources_by_class);
          }
        }
      );
  }

  // nonblocking, returns true if the resource is free and immediately acquired
  bool ResourceManager::acquire (resource::ID const& resource_id)
  {
    std::unique_lock<std::mutex> resources_lock (_resources_guard);

    if (!_resource_usage_by_id.count (resource_id))
    {
      std::ostringstream osstr;
      osstr << resource_id;
      throw std::invalid_argument ("Unknown resource id: " + osstr.str());
    }

    //! \todo for (hard/soft)-remove: mark resources for removal, do
    //! not acquire again, notify schedulers for (hard/soft)-cancel
    //! by resource id, yield until:

    //! block every resource `child*` that is down-reachable because
    //! `requested` is a (transitive) parent of `child*`

    //! block every (transitive) parent `(other_)parent*` of all
    //! down-reachable resources `child*` because one of their child
    //! has been blocked

    //! parent -> requested -> child1 -> child11
    //!        -> sibling~            -> child12
    //!                     -> child2 <- other_parent2
    //!                     -> child3 <- other_parent31 <- other_parent311
    //!                               <- other_parent32
    //!               other_child321~ <-

    //! Note: sibling~ and other_child321~ are _not_ blocked

    //! In this forest when requested is blocked, then all resources
    //! marked by * or # are blocked, too. The numbers after the *
    //! are the changes in the values of the reference counters, the
    //! number after the # are the exact values of the reference
    //! counters (they must have been zero before the acquisition)
    //!
    //!       A*4 --+       G*4 ..>       J*1
    //!       |     |       |             |
    //!       v     |       v             v
    //!       B     +-> requested#4 --+   H*1 --> I
    //!       |             |         |   |
    //!       v             v         |   v
    //!       F             C#2       +-> E#1
    //!                     |
    //!                     v
    //!                     D#1

    //! Note:
    //! - G ..> means that G might have more children. If not, then * is #.
    //! - B and F are not blocked as it would require going down
    //! from A again.
    //! - While blocking E blocks it's parent H, again, I is not
    //! blocked due to up-down required.
    //! - To release anything but `requested` will lead to chaos!

    if (_resource_usage_by_id.at (resource_id) == 0)
    {
      _resources.down_up
        ( resource_id
        , [&] (Resources::Node const& x)
          {
            if (1 == ++_resource_usage_by_id.at (x.first))
            {
              _available_resources_by_class.at (x.second).erase (x.first);
            }
          }
        );

      return true;
    }

    return false;
  }

  void ResourceManager::release (resource::ID const& resource_id)
  {
    std::lock_guard<std::mutex> const resources_lock (_resources_guard);

    _resources.down_up
      ( resource_id
      , [&] (Resources::Node const& x)
        {
          if (_resource_usage_by_id.at (x.first) == 0)
          {
            throw std::invalid_argument ("Not in use.");
          }

          if (0 == --_resource_usage_by_id.at (x.first))
          {
            _available_resources_by_class.at (x.second).emplace (x.first);
          }
        }
      );
  }
}
