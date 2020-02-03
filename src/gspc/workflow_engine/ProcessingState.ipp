#include <stdexcept>

#include <util-generic/print_container.hpp>
#include <util-generic/print_exception.hpp>

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

    template<typename TaskPrinter>
      print_processing_state<TaskPrinter>::print_processing_state
        (ProcessingState const& processing_state)
        : _processing_state (processing_state)
    {}
    template<typename TaskPrinter>
      std::ostream& print_processing_state<TaskPrinter>::operator()
        (std::ostream& os) const
    {
      auto print_task_map
        ( [&] (std::string name, auto const& map)
          {
            os << name << ": " << map.size() << "\n"
               << fhg::util::print_container
                    ( "  ", "\n  ", "\n", map
                    , [&] (auto& s, auto const& x) -> decltype (s)
                      {
                        return s << x.first << " -> " << TaskPrinter (x.second);
                      }
                    )
              ;
          }
        );
      auto print_map
        ( [&] (std::string name, auto const& map)
          {
            os << name << ": " << map.size() << "\n"
               << fhg::util::print_container
                    ( "  ", "\n  ", "\n", map
                    , [&] (auto& s, auto const& x) -> decltype (s)
                      {
                        return s << x.first << " -> " << x.second;
                      }
                    )
              ;
          }
        );
      auto print_set
        ( [&] (std::string name, auto const& set)
          {
            os << name << ": " << set.size() << "\n"
               << fhg::util::print_container ("  ", "\n  ", "\n", set)
              ;
          }
        );

      os << "next_task_id: " << _processing_state._next_task_id << "\n";
      print_task_map ("tasks", _processing_state._tasks);
      print_set ("extracted", _processing_state._extracted);
      os << "failed_to_post_process: " << _processing_state._failed_to_post_process.size() << "\n"
         << fhg::util::print_container
           ( "  ", "\n  ", "\n", _processing_state._failed_to_post_process
           , [&] (auto& s, auto const& x) -> decltype (s)
             {
               return s << x.first << " -> "
                        << x.second.first << " -> "
                        << fhg::util::exception_printer (x.second.second)
                 ;
             }
           );
      print_map ("failed_to_execute", _processing_state._failed_to_execute);
      print_map ("cancelled_ignored", _processing_state._cancelled_ignored);
      print_map ("cancelled_optional", _processing_state._cancelled_optional);
      print_map ("postponed", _processing_state._postponed);
      print_set ("marked_for_retry", _processing_state._marked_for_retry);
      return os;
    }
  }
}
