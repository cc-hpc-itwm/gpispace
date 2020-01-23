#pragma once

#include <gspc/Task.hpp>
#include <gspc/example/rtm/Parameter.hpp>
#include <gspc/interface/WorkflowEngine.hpp>
#include <gspc/task/ID.hpp>
#include <gspc/task/Result.hpp>
#include <gspc/workflow_engine/ProcessingState.hpp>
#include <gspc/workflow_engine/State.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/optional.hpp>

#include <cstdint>
#include <unordered_set>
#include <vector>

namespace gspc
{
  namespace rtm
  {
    using PartialResult = std::unordered_set<Shot>;

    struct LoadInput
    {
      Parameter parameter;
      Shot shot;

      template<typename Archive>
        void serialize (Archive&, unsigned int);
    };
    struct LoadOutput
    {
      Shot shot;

      template<typename Archive>
        void serialize (Archive&, unsigned int);
    };

    struct ProcessInput
    {
      Parameter parameter;
      Shot shot;

      template<typename Archive>
        void serialize (Archive&, unsigned int);
    };
    struct ProcessOutput
    {
      PartialResult result;

      template<typename Archive>
        void serialize (Archive&, unsigned int);
    };

    struct ReduceInput
    {
      Parameter parameter;
      PartialResult lhs;
      PartialResult rhs;

      template<typename Archive>
        void serialize (Archive&, unsigned int);
    };
    struct ReduceOutput
    {
      PartialResult result;

      template<typename Archive>
        void serialize (Archive&, unsigned int);
    };

    struct StoreInput
    {
      Parameter parameter;
      PartialResult result;

      template<typename Archive>
        void serialize (Archive&, unsigned int);
    };
    struct StoreOutput
    {
      PartialResult result;

      template<typename Archive>
        void serialize (Archive&, unsigned int);
    };

    struct print_task : public fhg::util::ostream::modifier
    {
      print_task (Task const&);
      virtual std::ostream& operator() (std::ostream&) const override;
    private:
      Task const& _task;
    };

    class WorkflowEngine : public interface::WorkflowEngine
    {
    public:
      WorkflowEngine
        ( boost::filesystem::path module
        , Parameter
        );

      virtual boost::variant<Task, bool> extract() override;
      virtual InjectResult inject (task::ID, task::Result) override;

      virtual workflow_engine::State state() const override;
      WorkflowEngine (workflow_engine::State);

      boost::optional<PartialResult> final_result() const;

    private:
      struct WorkflowState
      {
        boost::filesystem::path module;
        Parameter parameter;

        using TaskInput =
          boost::variant<LoadInput, ProcessInput, ReduceInput, StoreInput>;

        //! \note order not required, neither for front nor for partial_results
        std::vector<TaskInput> front;
        std::vector<PartialResult> partial_results;
        boost::optional<PartialResult> final_result;

        template<typename Archive>
          void serialize (Archive& ar, unsigned int /* version */);
      } _workflow_state;

      bool workflow_finished() const;

      workflow_engine::ProcessingState _processing_state;
    };
  }
}

#include <gspc/example/rtm/WorkflowEngine.ipp>
