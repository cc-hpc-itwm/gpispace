#pragma once

#include <gspc/Forest.hpp>
#include <gspc/interface/ResourceManager.hpp>
#include <gspc/resource/Class.hpp>
#include <gspc/resource/ID.hpp>

#include <condition_variable>
#include <cstddef>
#include <mutex>
#include <unordered_map>
#include <unordered_set>

namespace gspc
{
  namespace resource_manager
  {
    class Coallocation : public interface::ResourceManager
    {
    public:
      //! \note Only supports forward-disjoint classes.
      virtual void add (Resources) override;
      virtual void remove (Forest<resource::ID>) override;

      struct Acquired
      {
        std::unordered_set<resource::ID> requesteds;
        //! \note not shown to the user but implicitly locked, in
        //! order to avoid partial release. note: disallows freeing
        //! only requested but keeping dependent, e.g. (A -> B,
        //! request B, get {B, A}, release only B, still have A.)
        // std::unordered_set<resource::ID> dependent;
      };

      Acquired acquire (resource::Class, std::size_t);
      void release (Acquired const&);
      virtual void interrupt() override;

    private:
      //! C is not forward disjoint: C -> D <- C, but C -> D <- B is,
      //! also transitive! No acquire of a C-class resource shall
      //! implicitly block a different C-class resource.
      static void assert_is_strictly_forward_disjoint_by_resource_class
        (Resources const&);

      std::mutex _resources_guard;
      std::condition_variable _resources_became_available_or_interrupted;
      bool _interrupted {false};

      Resources _resources;
      std::unordered_map<resource::ID, std::size_t> _resource_usage_by_id;
      std::unordered_map<resource::Class, std::unordered_set<resource::ID>>
        _available_resources_by_class;

      void release (resource::ID const&);
    };
  }
}
