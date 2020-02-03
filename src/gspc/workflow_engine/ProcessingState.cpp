#include <gspc/workflow_engine/ProcessingState.hpp>

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
      return *_extracted.emplace (pop_any (_marked_for_retry)).first;
    }

    namespace
    {
      struct print_task : public fhg::util::ostream::modifier
      {
        print_task (Task const& task) : _task (task) {}
        virtual std::ostream& operator() (std::ostream& os) const override
        {
          return os << _task;
        }
      private:
        Task const& _task;
      };
    }

    std::ostream& operator<< (std::ostream& os, ProcessingState const& s)
    {
      return os << print_processing_state<print_task> (s);
    }
  }
}
