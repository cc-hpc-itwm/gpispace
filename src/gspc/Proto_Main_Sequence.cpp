#include <gspc/Forest.hpp>
#include <gspc/GreedyScheduler.hpp>
#include <gspc/SequenceWorkflowEngine.hpp>
#include <gspc/Resource.hpp>
#include <gspc/ScopedRuntimeSystem.hpp>
#include <gspc/remote_interface/strategy/Thread.hpp>
#include <gspc/resource_manager/Trivial.hpp>

#include <util-generic/finally.hpp>
#include <util-generic/print_exception.hpp>

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

int main (int argc, char** argv)
try
{
  if (argc != 2)
  {
    throw std::invalid_argument ("usage: Proto.exe num_tasks");
  }

  gspc::resource_manager::Trivial resource_manager;

  gspc::ScopedRuntimeSystem runtime_system (resource_manager);

  gspc::UniqueForest<gspc::Resource> host_topology;
  // n -> s0 -> c0 <- gpu --|
  //         -> c1 <- gpu ---> gpu_exclusive
  //   -> s1 -> c2 <- gpu --|
  //         -> c3 <- gpu -|
  auto const c0 (host_topology.insert ({"core"}, {}));
  auto const c1 (host_topology.insert ({"core"}, {}));
  auto const c2 (host_topology.insert ({"core"}, {}));
  auto const c3 (host_topology.insert ({"core"}, {}));
  auto const s0 (host_topology.insert ({"socket"}, {c0, c1}));
  auto const s1 (host_topology.insert ({"socket"}, {c2, c3}));
  host_topology.insert ({"node"}, {s0, s1});
  auto const gpu_exclusive (host_topology.insert ({"gpu_exclusive"}, {}));
  host_topology.insert ({"gpu"}, {c0, gpu_exclusive});
  host_topology.insert ({"gpu"}, {c1, gpu_exclusive});
  host_topology.insert ({"gpu"}, {c2, gpu_exclusive});
  host_topology.insert ({"gpu"}, {c3, gpu_exclusive});

  gspc::remote_interface::strategy::Thread thread_strategy
    (std::make_shared<gspc::remote_interface::strategy::Thread::State>());

  auto const resource_ids
    ( runtime_system.add_or_throw
    ( {"hostname1", "hostname2", "hostname3", "hostname4"}
      , thread_strategy
      , host_topology
      )
    );
  FHG_UTIL_FINALLY ([&] { runtime_system.remove (resource_ids); });

  gspc::SequenceWorkflowEngine workflow_engine (MODULE, std::stoul (argv[1]));

  gspc::GreedyScheduler scheduler
    ( workflow_engine
    , resource_manager
    , runtime_system
    );

  scheduler.wait();

  std::cout << workflow_engine.state() << std::endl;

  return EXIT_SUCCESS;
}
catch (...)
{
  std::cerr << "EXCEPTION: " << fhg::util::current_exception_printer() << '\n';

  return EXIT_FAILURE;
}
