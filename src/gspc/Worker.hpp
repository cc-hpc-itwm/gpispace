#pragma once

#include <gspc/Task.hpp>

namespace gspc
{
  struct Worker
  {
    void submit (task::ID, Task);
    void cancel (task::ID);
    task::State status (task::ID);
  };
}
