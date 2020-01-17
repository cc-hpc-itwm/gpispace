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
      Acquired acquire (resource::Class resource_class);
    };
  }
}
