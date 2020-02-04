#include <gspc/Forest.hpp>
#include <gspc/GreedyScheduler.hpp>
#include <gspc/example/workflow_engine/ClassConflictWorkflowEngine.hpp>
#include <gspc/Resource.hpp>
#include <gspc/ScopedRuntimeSystem.hpp>
#include <gspc/remote_interface/strategy/Thread.hpp>
#include <gspc/resource_manager/Trivial.hpp>

#include <util-generic/finally.hpp>
#include <util-generic/make_optional.hpp>
#include <util-generic/print_exception.hpp>

#include <iostream>
#include <memory>
#include <stdexcept>

int main (int argc, char** argv)
try
{
  if (argc < 2 || 3 < argc)
  {
    throw std::runtime_error
      ("usage: Proto.exe prefix_chain_length [max_lookahead]");
  }

  gspc::resource_manager::Trivial resource_manager;

  gspc::ScopedRuntimeSystem runtime_system (resource_manager);

  gspc::UniqueForest<gspc::Resource> host_topology;
  host_topology.insert ({"B"}, {});

  gspc::remote_interface::strategy::Thread thread_strategy
    (std::make_shared<gspc::remote_interface::strategy::Thread::State>());

  auto const resource_ids
    (runtime_system.add_or_throw ({"host"}, thread_strategy, host_topology));
  FHG_UTIL_FINALLY ([&] { runtime_system.remove (resource_ids); });

  auto const prefix_chain_length (std::stoul (argv[1]));
  auto const max_lookahead
    (FHG_UTIL_MAKE_OPTIONAL (argc == 3, std::stoul (argv[2])));

  gspc::ClassConflictWorkflowEngine workflow_engine
    (MODULE, prefix_chain_length);

  gspc::GreedyScheduler scheduler
    ( workflow_engine
    , resource_manager
    , runtime_system

    , 1 // max attempts
    , max_lookahead
    );

  scheduler.wait();

  if (prefix_chain_length < 100)
  {
    std::cout << workflow_engine.state().processing_state << std::endl;
  }

  return EXIT_SUCCESS;
}
catch (...)
{
  std::cerr << "EXCEPTION: " << fhg::util::current_exception_printer() << '\n';

  return EXIT_FAILURE;
}
