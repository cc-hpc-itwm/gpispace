#include <gspc/workflow_engine/ProcessingState.hpp>

#include <util-generic/print_container.hpp>
#include <util-generic/print_exception.hpp>

namespace gspc
{
  namespace workflow_engine
  {
    Task ProcessingState::extract
        ( resource::Class resource_class
        , boost::optional<heureka::Group> heureka_group
        , Task::Inputs inputs
        , boost::filesystem::path so
        , std::string symbol
        )
    {
      auto const task_id {*_extracted.emplace (++_next_task_id).first};

      return _tasks.emplace
        ( task_id
        , Task {task_id, resource_class, heureka_group, inputs, so, symbol}
        ).first->second;
    }

    bool ProcessingState::has_extracted_tasks() const
    {
      return !_extracted.empty();
    }

    std::ostream& operator<< (std::ostream& os, ProcessingState const& s)
    {
      return os
        << "next_task_id: " << s._next_task_id << "\n"
        << "tasks: " << s._tasks.size() << "\n"
        << fhg::util::print_container
           ( "  ", "\n  ", "\n", s._tasks
           , [&] (auto& s, auto const& x) -> decltype (s)
             {
               return s << x.first << " -> " << x.second;
             }
           )
        << "extracted: " << s._extracted.size() << "\n"
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
           )
        << "failed_to_execute: " << s._failed_to_execute.size()
        << fhg::util::print_container
           ( "  ", "\n  ", "\n", s._failed_to_execute
           , [&] (auto& s, auto const& x) -> decltype (s)
             {
               return s << x.first << " -> "
                        << fhg::util::exception_printer (x.second)
                 ;
             }
           )
        ;
    }
  }
}