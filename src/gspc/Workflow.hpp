#pragma once

#include <gspc/Task.hpp>

#include <boost/optional.hpp>

#include <functional>
#include <unordered_map>
#include <vector>

namespace gspc
{
  struct Workflow
  {
    // fault tolerance, complete state to recover from
    using State = std::vector<char>;
    State state() const;
    Workflow (State);

    boost::optional<Task> task();
    void inject (task::ID, task::State);

  private:
    task::ID _next_task {0};
    std::unordered_map<task::ID, Task> _tasks;
  };

  namespace workflow
  {
    using Connection = std::reference_wrapper<Workflow>;
  }
}