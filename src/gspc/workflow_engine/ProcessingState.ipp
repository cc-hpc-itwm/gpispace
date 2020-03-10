#include <stdexcept>

#include <boost/serialization/unordered_map.hpp>
#include <boost/serialization/unordered_set.hpp>
#include <boost/serialization/utility.hpp>

namespace gspc
{
  namespace workflow_engine
  {
    template<typename Function, typename>
      void ProcessingState::inject
        ( task::ID task_id
        , task::Result result
        , Function&& post_process_success
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

      auto forget_task
        ( [&]
          {
            //! \todo Customization point here?
            _tasks.erase (task);
          }
        );

      auto success_successfully_post_processed
        ( [&] (task::result::Success const& success) noexcept
          {
            try
            {
              post_process_success (task->second, success);

              return true;
            }
            catch (...)
            {
              _failed_to_post_process.emplace
                (task_id, std::make_pair (success, std::current_exception()));

              return false;
            }
          }
        );

      return fhg::util::visit<void>
        ( result
        , [&] (task::result::Success const& success)
          {
            if (success_successfully_post_processed (success))
            {
              forget_task();
            }
          }
        , [&] (task::result::Failure const& failure)
          {
            _failed_to_execute.emplace (task_id, failure);
          }
        , [&] (task::result::CancelIgnored const& cancel_ignored)
          {
            _cancelled_ignored.emplace (task_id, cancel_ignored);
            forget_task();
          }
        , [&] (task::result::CancelOptional const& cancel_optional)
          {
            _cancelled_optional.emplace (task_id, cancel_optional);
            forget_task();
          }
        , [&] (task::result::Postponed const& postponed)
          {
            _postponed.emplace (task_id, postponed);
          }
        );
    }

    template<typename Predicate, typename>
      void ProcessingState::mark_for_retry (Predicate&& predicate)
    {
      auto mark
        ( [&] (auto& states)
          {
            auto task_state (states.begin());

            while (task_state != states.end())
            {
              if (predicate (_tasks.at (task_state->first), task_state->second))
              {
                _marked_for_retry.emplace (task_state->first);

                task_state = states.erase (task_state);
              }
              else
              {
                ++task_state;
              }
            }
          }
        );

      mark (_failed_to_post_process);
      mark (_failed_to_execute);
      mark (_postponed);
      mark (_cancelled_ignored);
      mark (_cancelled_optional);
    }

    template<typename Archive>
      void ProcessingState::serialize (Archive& ar, unsigned int)
    {
      ar & _next_task_id;
      ar & _tasks;
      ar & _extracted;
      ar & _failed_to_post_process;
      ar & _failed_to_execute;
      ar & _postponed;
      ar & _cancelled_ignored;
      ar & _cancelled_optional;
      ar & _marked_for_retry;
    }
  }
}
