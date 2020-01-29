#pragma once

#include <gspc/Forest.hpp>
#include <gspc/interface/ResourceManager.hpp>
#include <gspc/resource/Class.hpp>
#include <gspc/resource/ID.hpp>

#include <condition_variable>
#include <cstddef>
#include <list>
#include <mutex>
#include <unordered_map>
#include <unordered_set>

namespace gspc
{
  namespace resource_manager
  {
    class WithPreferences : public interface::ResourceManager
    {
    public:
      virtual void add (Resources) override;
      virtual void remove (Forest<resource::ID>) override;

      struct Acquired
      {
        resource::ID requested;
        std::size_t selected;
      };

      //! blocks if no resource of that class available/exists
      //! (not-block-on-not-exists would race with add).
      Acquired acquire
        (InterruptionContext const&, std::vector<resource::Class>);
      void release (Acquired const&);
      virtual void interrupt (InterruptionContext&) override;

    private:
      std::mutex _resources_guard;
      std::condition_variable _resources_became_available_or_interrupted;

      Resources _resources;
      std::unordered_map<resource::ID, std::size_t> _resource_usage_by_id;
      std::unordered_map<resource::Class, std::unordered_set<resource::ID>>
        _available_resources_by_class;

      void release (resource::ID const&);
    };
  }
}
