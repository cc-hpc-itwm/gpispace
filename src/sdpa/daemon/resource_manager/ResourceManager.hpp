#pragma once

#include <drts/Forest.hpp>
#include <drts/resource/Class.hpp>
#include <drts/resource/ID.hpp>

#include <cstddef>
#include <list>
#include <mutex>
#include <unordered_map>
#include <unordered_set>

namespace gspc
{
  class ResourceManager
  {
  public:
    using Resources = Forest<resource::ID, resource::Class>;

    void add
      ( resource::ID const&
      , resource::Class const&
      , Resources::Children const&
      );
    void remove (resource::ID const&);

    bool acquire (resource::ID const&);
    void release (resource::ID const&);

  private:
    std::mutex _resources_guard;

    Resources _resources;
    std::unordered_map<resource::ID, std::size_t> _resource_usage_by_id;
    std::unordered_map<resource::Class, std::unordered_set<resource::ID>>
      _available_resources_by_class;
  };
}
