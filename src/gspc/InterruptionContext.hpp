#pragma once

namespace gspc
{
  namespace resource_manager
  {
    class Coallocation;
    class Trivial;
    class WithPreferences;
  }

  struct InterruptionContext
  {
  private:
    friend class resource_manager::Coallocation;
    friend class resource_manager::Trivial;
    friend class resource_manager::WithPreferences;

    bool _interrupted {false};
  };
}
