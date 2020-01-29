#include <gspc/resource_manager/WithPreferences.hpp>

#include <util-generic/warning.hpp>

#include <algorithm>
#include <iterator>
#include <stdexcept>

namespace gspc
{
  namespace resource_manager
  {
    void WithPreferences::interrupt (InterruptionContext& interruption_context)
    {
      std::lock_guard<std::mutex> const resources_lock (_resources_guard);

      interruption_context._interrupted = true;

      _resources_became_available_or_interrupted.notify_all();
    }

    void WithPreferences::add (Resources new_resources)
    {
      std::lock_guard<std::mutex> const resources_lock (_resources_guard);

      //! \note already asserts precondition of uniqueness
      _resources.merge (new_resources);

      new_resources.for_each_node
        ( [&] (Resources::Node const& resource)
          {
            _resource_usage_by_id.emplace (resource.first, 0);
            _available_resources_by_class[resource.second]
              . emplace (resource.first)
              ;
          }
        );

      _resources_became_available_or_interrupted.notify_all();
    }

    void WithPreferences::remove (Forest<resource::ID> to_remove)
    {
      std::lock_guard<std::mutex> const resources_lock (_resources_guard);

      to_remove.for_each_node
        ( [&] (forest::Node<resource::ID> const& resource)
          {
            if (!_resource_usage_by_id.count (resource.first))
            {
              throw std::invalid_argument ("Unknown.");
            }
          }
        );

      //! \todo for (hard/soft)-remove: mark resources for removal, do
      //! not acquire again, notify schedulers for (hard/soft)-cancel
      //! by resource id, yield until:

      to_remove.for_each_node
        ( [&] (forest::Node<resource::ID> const& resource)
          {
            if (_resource_usage_by_id.at (resource.first))
            {
              throw std::invalid_argument ("In use.");
            }
          }
        );

      // here: all resources in `to_remove` are known and unused

      to_remove.upward_apply
        ( [&] (forest::Node<resource::ID> const& resource)
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

    namespace
    {
      template<typename Collection>
        auto select_any (Collection& collection)
          -> decltype (*collection.begin())
      {
        return *collection.begin();
      }
    }

    WithPreferences::Acquired WithPreferences::acquire
      ( InterruptionContext const& interruption_context
      , std::vector<resource::Class> resource_classes
      )
    {
      std::unique_lock<std::mutex> resources_lock (_resources_guard);

      auto const has_available_resource
        ( [&] (resource::Class resource_class)
          {
            return _available_resources_by_class.count (resource_class)
              && !_available_resources_by_class.at (resource_class).empty()
              ;
          }
        );

      _resources_became_available_or_interrupted.wait
        ( resources_lock
        , [&]
          {
            return std::any_of ( resource_classes.begin()
                               , resource_classes.end()
                               , has_available_resource
                               )
              || interruption_context._interrupted
              ;
          }
        );

      if (interruption_context._interrupted)
      {
        throw Interrupted{};
      }

      auto const resource_class
        ( std::find_if ( resource_classes.begin()
                       , resource_classes.end()
                       , has_available_resource
                       )
        );

      auto const requested
        (select_any (_available_resources_by_class.at (*resource_class)));

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

      _resources.down_up
        ( requested
        , [&] (Resources::Node const& x)
          {
            if (0 != ++_resource_usage_by_id.at (x.first))
            {
              _available_resources_by_class.at (x.second).erase (x.first);
            }
          }
        );

      return Acquired
        { requested
        , fhg::util::suppress_warning::sign_conversion<std::size_t>
            ( std::distance (resource_classes.begin(), resource_class)
            , "begin is not after std::find_if (begin, ...)"
            )
        };
    }

    void WithPreferences::release (Acquired const& to_release)
    {
      return release (to_release.requested);
    }
    void WithPreferences::release (resource::ID const& to_release)
    {
      std::lock_guard<std::mutex> const resources_lock (_resources_guard);

      _resources.down_up
        ( to_release
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

      _resources_became_available_or_interrupted.notify_all();
    }
  }
}
