#pragma once

#include <gspc/Task.hpp>
#include <gspc/task/ID.hpp>
#include <gspc/task/Result.hpp>

#include <util-generic/callable_signature.hpp>
#include <util-generic/ostream/modifier.hpp>

#include <exception>
#include <functional>
#include <ostream>
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
      task::ID extract (Task::Requirements, task::Input);

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

      Task const& at (task::ID) const;

      using FailedToPostprocess = std::pair< task::result::Success
                                           , std::exception_ptr
                                           >;
      using Failure = task::result::Failure;
      using Postponed = task::result::Postponed;
      using CancelIgnored = task::result::CancelIgnored;
      using CancelOptional = task::result::CancelOptional;

      using TaskState = boost::variant< FailedToPostprocess
                                      , Failure
                                      , Postponed
                                      , CancelIgnored
                                      , CancelOptional
                                      >;

      template<typename Predicate>
        using is_retry_predicate =
          fhg::util::is_callable
           <Predicate, bool (Task const&, TaskState const&)>;

      struct RetryAll
      {
        bool operator() (Task const&, TaskState const&) const
        {
          return true;
        }
      };

      template< typename Predicate = RetryAll
              , typename = std::enable_if_t<is_retry_predicate<Predicate>{}>
              >
        void mark_for_retry (Predicate&& = {});

      bool has_retry_task() const;
      task::ID retry_task();

      friend std::ostream& operator<< (std::ostream&, ProcessingState const&);

      template<typename Archive>
        void serialize (Archive&, unsigned int);

    private:
      friend struct print_processing_state;

      task::ID _next_task_id {0};

      //! every task in tasks is either
      //! - in flight: _extracted
      //!   - but postponed by scheduler: _postponed
      //! - failed executing: _failed_to_execute
      //! - failed to post process: _failed_to_post_process
      //! - extracted, but no longer required after inject (heureka):
      //!   - if flagged ignored: _cancelled_ignore
      //!   - if flagged optional: _cancelled_optional
      //! - marked for retry: _marked_for_retry
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
      std::unordered_map<task::ID, FailedToPostprocess> _failed_to_post_process;
      std::unordered_map<task::ID, Failure> _failed_to_execute;
      std::unordered_map<task::ID, Postponed> _postponed;
      std::unordered_map<task::ID, CancelIgnored> _cancelled_ignored;
      std::unordered_map<task::ID, CancelOptional> _cancelled_optional;
      std::unordered_set<task::ID> _marked_for_retry;
    };

    struct print_processing_state : public fhg::util::ostream::modifier
    {
      using TaskPrinter = std::function<void (std::ostream&, Task const&)>;

      print_processing_state
        ( ProcessingState const&
        , TaskPrinter = [] (std::ostream& os, Task const& task)
                        {
                          os << task;
                        }
        );
      virtual std::ostream& operator() (std::ostream&) const override;

    private:
      ProcessingState const& _processing_state;
      TaskPrinter _task_printer;
    };
  }
}

#include <gspc/workflow_engine/ProcessingState.ipp>
