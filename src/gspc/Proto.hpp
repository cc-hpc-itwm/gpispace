#pragma once

#include <gspc/comm/runtime_system/remote_interface/Client.hpp>
#include <gspc/comm/runtime_system/remote_interface/protocol.hpp>
#include <gspc/comm/runtime_system/resource_manager/Client.hpp>
#include <gspc/comm/runtime_system/resource_manager/Server.hpp>
#include <gspc/comm/worker/scheduler/Server.hpp>

#include <gspc/ErrorOr.hpp>
#include <gspc/Forest.hpp>

#include <gspc/interface/ResourceManager.hpp>
#include <gspc/interface/Scheduler.hpp>
#include <gspc/interface/WorkflowEngine.hpp>

#include <gspc/Job.hpp>
#include <gspc/job/FinishReason.hpp>
#include <gspc/job/ID.hpp>

#include <gspc/RemoteInterface.hpp>
#include <gspc/remote_interface/Hostname.hpp>
#include <gspc/remote_interface/ID.hpp>

#include <gspc/Resource.hpp>
#include <gspc/resource/Class.hpp>
#include <gspc/resource/ID.hpp>

#include <gspc/resource_manager/Coallocation.hpp>
#include <gspc/resource_manager/Trivial.hpp>
#include <gspc/resource_manager/WithPreferences.hpp>

#include <gspc/rpc/TODO.hpp>

#include <gspc/Task.hpp>
#include <gspc/task/ID.hpp>
#include <gspc/task/Result.hpp>

#include <gspc/value_type.hpp>

#include <gspc/GreedyScheduler.hpp>
#include <gspc/ScopedRuntimeSystem.hpp>
#include <gspc/remote_interface/ConnectionAndPID.hpp>
#include <gspc/remote_interface/Strategy.hpp>
#include <gspc/remote_interface/strategy/Thread.hpp>

#include <gspc/Worker.hpp>

#include <gspc/MapWorkflowEngine.hpp>
#include <gspc/workflow_engine/ProcessingState.hpp>
#include <gspc/workflow_engine/State.hpp>

#include <gspc/util-generic_hash_forward_declare.hpp>
#include <util-generic/connectable_to_address_string.hpp>
#include <util-generic/finally.hpp>
#include <util-generic/hash/combined_hash.hpp>
#include <util-generic/serialization/boost/filesystem/path.hpp>
#include <util-generic/hostname.hpp>
#include <util-generic/print_exception.hpp>
#include <util-generic/scoped_boost_asio_io_service_with_threads.hpp>
#include <util-generic/this_bound_mem_fn.hpp>
#include <util-generic/threadsafe_queue.hpp>

#include <boost/variant.hpp>
#include <boost/range/adaptor/map.hpp>
#include <boost/filesystem/path.hpp>

#include <cstdint>
#include <cstdlib>
#include <exception>
#include <iostream>
#include <list>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <tuple>
#include <unordered_map>
#include <unordered_set>

namespace gspc
{
  class PetriNetWorkflow{};
  class PetriNetWorkflowEngine : public interface::WorkflowEngine
  {
  public:
    PetriNetWorkflowEngine (PetriNetWorkflow);

    virtual boost::variant<Task, bool> extract() override;
    virtual void inject (task::ID, ErrorOr<task::Result>) override;
  };
  class TreeTraversalWorkflow;
  class TreeTraversalWorkflowEngine;
  class MapReduceWorkflow;
  class MapReduceWorkflowEngine;

  class ReschedulingGreedyScheduler;
  class LookaheadScheduler;
  class WorkStealingScheduler;
  class CoallocationScheduler;
  class TransferCostAwareScheduler;

  namespace resource_manager
  {
    // class CoallocationWithPreference : public interface::ResourceManager
    // {
    //   // `[(rc, count)]` or `[rc], count` or both?
    //   Acquired acquire (std::list<std::pair<resource::Class, std::size_t>>);
    // };
  }
}
