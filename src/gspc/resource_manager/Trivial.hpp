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
      };

      Acquired acquire
        (InterruptionContext const&, resource::Class resource_class);
      void release (Acquired const&);
    };
  }
}
