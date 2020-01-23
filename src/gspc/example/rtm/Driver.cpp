#include <gspc/example/rtm/Parameter.hpp>
#include <gspc/example/rtm/WorkflowEngine.hpp>

#include <gspc/GreedyScheduler.hpp>
#include <gspc/Resource.hpp>
#include <gspc/ScopedRuntimeSystem.hpp>
#include <gspc/remote_interface/strategy/Thread.hpp>
#include <gspc/resource_manager/Trivial.hpp>

#include <util-generic/finally.hpp>
#include <util-generic/print_exception.hpp>
#include <util-generic/timer/application.hpp>

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

int main ()
try
{
  fhg::util::default_application_timer echo ("rtm");

  std::size_t const num_cores_per_socket (10);
  std::size_t const num_sockets          (2);
  std::size_t const num_sockets_with_gpu (1);
  std::size_t const num_loads            (4);
  std::size_t const num_stores           (4);

  std::size_t const num_nodes            (2);

  echo << "num_cores_per_socket = " << num_cores_per_socket << '\n';
  echo << "num_sockets          = " << num_sockets << '\n';
  echo << "num_sockets_with_gpu = " << num_sockets_with_gpu << '\n';
  echo << "num_loads            = " << num_loads << '\n';
  echo << "num_stores           = " << num_stores << '\n';

  echo << "num_nodes            = " << num_nodes << '\n';

  echo.section ("create host topology");

  auto host_topology
    (gspc::rtm::host_topology ( num_cores_per_socket
                              , num_sockets
                              , num_sockets_with_gpu
                              , num_loads
                              , num_stores
                              )
    );

  echo.section ("create resource manager");

  gspc::resource_manager::Trivial resource_manager;

  echo.section ("create runtime system");

  gspc::ScopedRuntimeSystem runtime_system (resource_manager);

  echo.section ("create runtime system");

  gspc::remote_interface::strategy::Thread thread_strategy
    (std::make_shared<gspc::remote_interface::strategy::Thread::State>());

  echo.section ("add resources");

  std::unordered_set<gspc::remote_interface::Hostname> hostnames;

  for (std::size_t node {0}; node < num_nodes; ++node)
  {
    hostnames.emplace (std::to_string (node));
  }

  auto const resource_ids
    (runtime_system.add_or_throw (hostnames, thread_strategy, host_topology));
  FHG_UTIL_FINALLY ([&] { runtime_system.remove (resource_ids); });

  echo.section ("create workflow engine");

  gspc::rtm::Parameter parameter;

  gspc::rtm::WorkflowEngine workflow_engine (MODULE, parameter);

  echo.section ("create schedule and execute job");

  gspc::GreedyScheduler scheduler
    ( workflow_engine
    , resource_manager
    , runtime_system
    );

  scheduler.wait();

  echo.section ("sanity");

  echo << workflow_engine.state() << std::endl;

  echo.section ("cleanup");

  return EXIT_SUCCESS;
}
catch (...)
{
  std::cerr << "EXCEPTION: " << fhg::util::current_exception_printer() << '\n';

  return EXIT_FAILURE;
}
