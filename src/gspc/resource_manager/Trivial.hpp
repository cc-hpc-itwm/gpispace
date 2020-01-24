#pragma once

#include <gspc/resource/Class.hpp>
#include <gspc/resource_manager/WithPreferences.hpp>

namespace gspc
{
  namespace resource_manager
  {
    class Trivial : public WithPreferences
    {
    public:
      struct Acquired
      {
        resource::ID requested;
        //! \note not shown to the user but implicitly locked, in
        //! order to avoid partial release. note: disallows freeing
        //! only requested but keeping dependent, e.g. (A -> B,
        //! request B, get {B, A}, release only B, still have A.)
        // std::unordered_set<resource::ID> dependent;
      };

      Acquired acquire (resource::Class resource_class);
      void release (Acquired const&);
    };
  }
}
