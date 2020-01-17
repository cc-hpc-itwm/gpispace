#pragma once

#include <gspc/ErrorOr.hpp>
#include <gspc/Task.hpp>
#include <gspc/task/ID.hpp>
#include <gspc/task/Result.hpp>
#include <gspc/workflow_engine/State.hpp>

namespace gspc
{
  namespace interface
  {
    class WorkflowEngine
    {
    public:
      virtual ~WorkflowEngine() = default;
      //! extract.is_bool && extract.second == false: workflow has not
      //! finished yet, e.g. because it waits for external events
      //! \note workflow engine shall not say "true" when there are
      //! extacted tasks
      virtual boost::variant<Task, bool> extract() = 0;
      virtual void inject (task::ID, ErrorOr<task::Result>) = 0;

      virtual workflow_engine::State state() const = 0;
      //! \required: WorkflowEngine (workflow_engine::State)
    };
  }
}
