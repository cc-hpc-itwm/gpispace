#pragma once

#include <gspc/Workflow.hpp>
#include <gspc/ResourceManager.hpp>

namespace gspc
{
  struct Scheduler
  {
    Scheduler ( resource_manager::Connection
              , workflow::Connection
              );

    void wait();
    void stop();

    void finished (task::ID, task::State);
  };
}
