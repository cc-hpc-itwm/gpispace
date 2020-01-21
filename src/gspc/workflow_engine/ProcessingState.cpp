#include <gspc/workflow_engine/ProcessingState.hpp>

#include <util-generic/print_container.hpp>
#include <util-generic/print_exception.hpp>

namespace gspc
{
  namespace workflow_engine
  {
    Task ProcessingState::extract
        ( resource::Class resource_class
        , Task::Inputs inputs
        , boost::filesystem::path so
        , std::string symbol
        )
    {
      auto const task_id {*_extracted.emplace (++_next_task_id).first};

      return _tasks.emplace
        ( task_id
        , Task {task_id, resource_class, inputs, so, symbol}
        ).first->second;
    }

    bool ProcessingState::has_extracted_tasks() const
    {
      return !_extracted.empty();
    }

    std::unordered_set<task::ID> ProcessingState::extracted() const
    {
      return _extracted;
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

      os << "next_task_id: " << s._next_task_id << "\n";
      print_map ("tasks", s._tasks);
      os << "extracted: " << s._extracted.size() << "\n"
         << fhg::util::print_container ( "  ", "\n  ", "\n", s._extracted)
         << "failed_to_post_process: " << s._failed_to_post_process.size() << "\n"
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
      return os;
    }
  }
}
