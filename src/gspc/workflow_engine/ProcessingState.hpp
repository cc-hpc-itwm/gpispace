#pragma once

#include <gspc/ErrorOr.hpp>
#include <gspc/Task.hpp>
#include <gspc/resource/Class.hpp>
#include <gspc/task/ID.hpp>
#include <gspc/task/Result.hpp>

#include <util-generic/callable_signature.hpp>

#include <boost/filesystem/path.hpp>

#include <exception>
#include <ostream>
#include <string>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace gspc
{
  namespace workflow_engine
  {
    struct ProcessingState
    {
    public:
      Task extract
        ( resource::Class resource_class
        , Task::Input input
        , boost::filesystem::path so
        , std::string symbol
        );

      template<typename Function>
        using is_post_process =
          fhg::util::is_callable
            < Function
            , void (Task const&, task::result::Success const&)
            >;

      template< typename Function
              , typename = std::enable_if_t<is_post_process<Function>{}>
              >
      void inject
        ( task::ID task_id
        , task::Result result
        , Function&& post_process_success
        );

      bool has_extracted_tasks() const;

      std::unordered_set<task::ID> extracted() const;

      friend std::ostream& operator<< (std::ostream&, ProcessingState const&);

      //! \todo missing api to restart/drop postponed/failed tasks

    private:
      task::ID _next_task_id {0};

      //! every task in tasks is either
      //! - in flight: _extracted
      //!   - but postponed by scheduler: _postponed
      //! - failed executing: _failed_to_execute
      //! - failed to post process: _failed_to_post_process
      //! - extracted, but no longer required after inject (heureka):
      //!   - if flagged ignored: _cancelled_ignore
      //!   - if flagged optional: _cancelled_optional
      //! note: finished tasks are forgotten (they changed state when
      //! post processed).
      //! \note would be enough to remember data required to
      //! reconstruct task
      //! \todo Let workflow engine implementation decide whether to
      //! keep successes/cancels? -> may be answered with "no",
      //! because workflow engine sees result and task before giving
      //! it to this function, so can store there anyway.
      std::unordered_map<task::ID, Task> _tasks;

      //! \note keys of _extracted, _failed_to_post_process,
      //! _failed_to_execute are
      //! - disjoint
      //! - subset of keys (_tasks)
      std::unordered_set<task::ID> _extracted;
      std::unordered_map< task::ID
                        , std::pair<task::result::Success, std::exception_ptr>
                        > _failed_to_post_process;
      std::unordered_map<task::ID, task::result::Failure> _failed_to_execute;
      std::unordered_map<task::ID, task::result::Postponed> _postponed;
      std::unordered_map<task::ID, task::result::CancelIgnored> _cancelled_ignored;
      std::unordered_map<task::ID, task::result::CancelOptional> _cancelled_optional;
    };
  }
}

#include <gspc/workflow_engine/ProcessingState.ipp>
