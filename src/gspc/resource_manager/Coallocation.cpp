#include <gspc/resource_manager/Coallocation.hpp>

#include <algorithm>
#include <iterator>
#include <stdexcept>

namespace gspc
{
  namespace resource_manager
  {
    void Coallocation::assert_is_strictly_forward_disjoint_by_resource_class
      (Resources const& resources)
    {
      resources.for_each_leaf
        ( [&] (Resources::Node const& start)
          {
            std::unordered_map<resource::Class, std::size_t>
              visible_resources_by_class;

            resources.up
              ( start.first
              , [&] (Resources::Node const& visible)
                {
                  if (visible_resources_by_class[visible.second]++)
                  {
                    throw std::logic_error
                      ("not strictly_forward_disjoint_by_resource_class");
                  }
                }
              );
          }
        );
    }

    void Coallocation::interrupt (InterruptionContext& interruption_context)
    {
      std::lock_guard<std::mutex> const resources_lock (_resources_guard);

      interruption_context._interrupted = true;

      _resources_became_available_or_interrupted.notify_all();
    }

    void Coallocation::add (Resources new_resources)
    {
      //! \note We can't add connections to existing resources, so
      //! checking addition is enough.
      assert_is_strictly_forward_disjoint_by_resource_class (new_resources);

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

    void Coallocation::remove (Forest<resource::ID> to_remove)
    {
      std::lock_guard<std::mutex> const resources_lock (_resources_guard);

      to_remove.for_each_node
        ( [&] (forest::Node<resource::ID> const& resource)
          {
            if (!_resource_usage_by_id.count (resource.first))
            {
              throw std::invalid_argument ("Unknown.");
            }
            if (_resource_usage_by_id.at (resource.first))
            {
              throw std::invalid_argument ("In use.");
            }
          }
        );

      //! required: upwards or downwards per unconnected sub-tree,
      //! just *not random*. Every block that happened before removal
      //! has to still block during (or have one side of the block
      //! vanished).
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
      template<typename ResourceIDs>
        std::unordered_set<resource::ID> select_any
          (ResourceIDs& collection, std::size_t count)
      {
        auto begin (collection.begin());
        return {begin, std::next (begin, count)};
      }
    }

    Coallocation::Acquired Coallocation::acquire
      ( InterruptionContext const& interruption_context
      , resource::Class resource_class, std::size_t count
      )
    {
      std::unique_lock<std::mutex> resources_lock (_resources_guard);

      auto const has_available_resources
        ( [&] (resource::Class resource_class)
          {
            return _available_resources_by_class.count (resource_class)
              && _available_resources_by_class.at (resource_class).size()
                 >= count
              ;
          }
        );

      _resources_became_available_or_interrupted.wait
        ( resources_lock
        , [&]
          {
            return has_available_resources (resource_class)
              || interruption_context._interrupted;
          }
        );

      if (interruption_context._interrupted)
      {
        throw Interrupted{};
      }

      auto const requesteds
        (select_any (_available_resources_by_class.at (resource_class), count));

      //! \note Forward-disjoint is required here: All traversals are
      //! downwards independent (they may reach the same resources
      //! upwards though), but one element of requesteds will never
      //! lock another one.
      std::for_each
        ( requesteds.begin()
        , requesteds.end()
        , [&] (auto const& requested)
          {
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
          }
        );

      return Acquired {requesteds /*, up-visited*/};
    }

    void Coallocation::release (Acquired const& to_release)
    {
      std::lock_guard<std::mutex> const resources_lock (_resources_guard);

      std::for_each ( to_release.requesteds.begin()
                    , to_release.requesteds.end()
                    , [&] (resource::ID const& id)
                      {
                        release (id);
                      }
                    );

      _resources_became_available_or_interrupted.notify_all();
    }
    void Coallocation::release (resource::ID const& to_release)
    {
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
    }
  }
}
