#include <gspc/workflow_engine/ProcessingState.hpp>

#include <util-generic/print_container.hpp>
#include <util-generic/print_exception.hpp>

#include <utility>

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
        , Task {std::move (input), std::move (requirements)}
        ).first->first;
    }

    bool ProcessingState::has_extracted_tasks() const
    {
      return !_extracted.empty();
    }

    std::unordered_set<task::ID> ProcessingState::extracted() const
    {
      return _extracted;
    }

    Task const& ProcessingState::at (task::ID task_id) const
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
      return *_extracted.emplace (pop_any (_marked_for_retry)).first;
    }

    std::ostream& operator<< (std::ostream& os, ProcessingState const& s)
    {
      return os << print_processing_state (s);
    }

    print_processing_state::print_processing_state
      ( ProcessingState const& processing_state
      , TaskPrinter task_printer
      )
        : _processing_state (processing_state)
        , _task_printer (std::move (task_printer))
    {}

    std::ostream& print_processing_state::operator() (std::ostream& os) const
    {
      auto print_task_map
        ( [&] (std::string name, auto const& map)
          {
            os << name << ": " << map.size() << "\n"
               << fhg::util::print_container
                    ( "  ", "\n  ", "\n", map
                    , [&] (auto& s, auto const& x) -> decltype (s)
                      {
                        s << x.first << " -> ";

                        _task_printer (s, x.second);

                        return s;
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
