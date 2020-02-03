#include <gspc/workflow_engine/ProcessingState.hpp>

#include <util-generic/print_container.hpp>
#include <util-generic/print_exception.hpp>

namespace gspc
{
  namespace workflow_engine
  {
    task::ID ProcessingState::extract
      (Task::Requirements requirements, task::Input input)
    {
      auto const task_id {*_extracted.emplace (++_next_task_id).first};

      return _tasks.emplace
        ( task_id
        , Task {task_id, std::move (input), std::move (requirements)}
        ).first->second.id;
    }

    bool ProcessingState::has_extracted_tasks() const
    {
      return !_extracted.empty();
    }

    std::unordered_set<task::ID> ProcessingState::extracted() const
    {
      return _extracted;
    }

    Task ProcessingState::at (task::ID task_id) const
    {
      return _tasks.at (task_id);
    }

    namespace
    {
      task::ID pop_any (std::unordered_set<task::ID>& task_ids)
      {
        auto selected (task_ids.begin());
        auto task_id (*selected);
        task_ids.erase (selected);
        return task_id;
      }
    }

    bool ProcessingState::has_retry_task() const
    {
      return !_marked_for_retry.empty();
    }
    task::ID ProcessingState::retry_task()
    {
      auto const task_id (pop_any (_marked_for_retry));

      return _tasks.at (*_extracted.emplace (task_id).first).id;
    }

    std::ostream& operator<< (std::ostream& os, ProcessingState const& s)
    {
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

      os << "next_task_id: " << s._next_task_id << "\n";
      print_map ("tasks", s._tasks);
      print_set ("extracted", s._extracted);
      os << "failed_to_post_process: " << s._failed_to_post_process.size() << "\n"
         << fhg::util::print_container
           ( "  ", "\n  ", "\n", s._failed_to_post_process
           , [&] (auto& s, auto const& x) -> decltype (s)
             {
               return s << x.first << " -> "
                        << x.second.first << " -> "
                        << fhg::util::exception_printer (x.second.second)
                 ;
             }
           );
      print_map ("failed_to_execute", s._failed_to_execute);
      print_map ("cancelled_ignored", s._cancelled_ignored);
      print_map ("cancelled_optional", s._cancelled_optional);
      print_map ("postponed", s._postponed);
      print_set ("marked_for_retry", s._marked_for_retry);
      return os;
    }
  }
}
