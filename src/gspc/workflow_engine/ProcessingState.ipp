#include <stdexcept>

namespace gspc
{
  namespace workflow_engine
  {
    template<typename Function, typename>
      void ProcessingState::inject
        ( task::ID task_id
        , ErrorOr<task::Result> error_or_result
        , Function&& post_process_result
        )
    {
      if (!_extracted.erase (task_id))
      {
        //! \todo collect information: multiple result
        throw std::invalid_argument ("ProcessingState::inject: Unknown task.");
      }

      auto task (_tasks.find (task_id));

      if (task == _tasks.end())
      {
        throw std::logic_error
          ("ProcessingState::inject: INCONSISTENCY: Tasks data missing.");
      }

      auto result_successfully_post_processed
        ( [&] (task::Result const& result) noexcept
          {
            try
            {
              post_process_result (task->second, result);

              return true;
            }
            catch (...)
            {
              _failed_to_post_process.emplace
                (task_id, std::make_pair (result, std::current_exception()));

              return false;
            }
          }
          );

      if (!error_or_result)
      {
        _failed_to_execute.emplace (task_id, error_or_result.error());
      }
      else if (result_successfully_post_processed (error_or_result.value()))
      {
        _tasks.erase (task);
      }
    }
  }
}
