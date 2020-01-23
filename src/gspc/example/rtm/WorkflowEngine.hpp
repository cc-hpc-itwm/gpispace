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

    private:
      struct WorkflowState
      {
        struct TaskHashEqual
        {
          std::size_t operator() (Task const& task) const
          {
            return std::hash<task::ID>{} (task.id);
          }
          bool operator() (Task const& lhs, Task const& rhs) const
          {
            return std::tie (lhs.id) == std::tie (rhs.id);
          }
        };

        boost::filesystem::path module;
        Parameter parameter;

        std::unordered_set<Task, TaskHashEqual, TaskHashEqual> front;
        //! \note order not required
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
