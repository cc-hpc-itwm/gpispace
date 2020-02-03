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

      //! \todo the bool should probably become
      //! enum { InjectMayProduceATask         = false
      //!      , ExternalEventMayProduceATask  = false // equal to the first case when put_token implies inject
      //!      , NoMoreTasksWillBeProducedEver = false // inject missing
      //!      , Done                          = true  // inject would be an error
      //!      };
      virtual boost::variant<task::ID, bool> extract() = 0;

      struct InjectResult
      {
        //! \note ignored or optional is important for the
        //! workflow_engine but not for the scheduler. It is okay to
        //! cancel, it is okay to run and inject the results

        //! shall not contain the injected task
        std::unordered_set<task::ID> tasks_with_ignored_result;
        std::unordered_set<task::ID> tasks_with_optional_result;
      };
      virtual InjectResult inject (task::ID, task::Result) = 0;

      virtual Task const& at (task::ID) const = 0;

      virtual workflow_engine::State state() const = 0;
      //! \required: WorkflowEngine (workflow_engine::State)
    };
  }
}
