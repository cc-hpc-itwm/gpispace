#include <gspc/example/rtm/Parameter.hpp>
#include <gspc/example/rtm/WorkflowEngine.hpp>

#include <gspc/GreedyScheduler.hpp>
#include <gspc/Resource.hpp>
#include <gspc/ScopedRuntimeSystem.hpp>
#include <gspc/remote_interface/strategy/Thread.hpp>
#include <gspc/resource_manager/Trivial.hpp>
#include <gspc/serialization.hpp>

#include <util-generic/finally.hpp>
#include <util-generic/print_exception.hpp>
#include <util-generic/read_file.hpp>
#include <util-generic/timer/application.hpp>

#include <boost/format.hpp>

#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>
#include <fstream>

int main (int argc, char** argv)
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

  std::unique_ptr<gspc::rtm::WorkflowEngine> workflow_engine;

  if (argc > 2)
  {
    workflow_engine = std::make_unique<gspc::rtm::WorkflowEngine>
      ( gspc::bytes_load<gspc::workflow_engine::State>
          (fhg::util::read_file<std::vector<char>> (argv[2]))
      );
  }
  else
  {
    gspc::rtm::Parameter parameter;
    parameter.number_of_shots = 20;
    parameter.probability_of_failure.load = 0.0;
    parameter.probability_of_failure.process = 0.0;
    parameter.probability_of_failure.reduce = 0.0;
    parameter.probability_of_failure.store = 0.0;

    workflow_engine = std::make_unique<gspc::rtm::WorkflowEngine>
      (MODULE, parameter);
  }

  echo.section ("create schedule and execute job");

  gspc::GreedyScheduler scheduler
    ( *workflow_engine
    , resource_manager
    , runtime_system
    );

  scheduler.wait();

  echo.section ("sanity");

  auto const state (workflow_engine->state());

  if (argc > 1)
  {
    std::ofstream stream (argv[1]);
    auto serialized_state (gspc::bytes_save (state));
    stream.write (serialized_state.data(), serialized_state.size());

    if (!stream)
    {
      throw std::runtime_error
        (str (boost::format ("Could not write to %1%.") % argv[1]));
    }
  }

  echo << state << std::endl;

  if (auto final_result = workflow_engine->final_result())
  {
    echo << "Final result: "
         << fhg::util::print_container ("{", ", ", "}", *final_result)
         << std::endl;
  }
  else
  {
    echo << "Final result not yet produced.\n";
  }

  echo.section ("cleanup");

  return EXIT_SUCCESS;
}
catch (...)
{
  std::cerr << "EXCEPTION: " << fhg::util::current_exception_printer() << '\n';

  return EXIT_FAILURE;
}
