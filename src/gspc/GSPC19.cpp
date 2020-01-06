#include <gspc/ExecutionEnvironment.hpp>
#include <gspc/MaybeError.hpp>
#include <gspc/RemoteInterface.hpp>
#include <gspc/ResourceManager.hpp>
#include <gspc/Resource.hpp>
#include <gspc/Scheduler.hpp>
#include <gspc/Scheduler.hpp>
#include <gspc/Task.hpp>
#include <gspc/Tree.hpp>
#include <gspc/Worker.hpp>
#include <gspc/Workflow.hpp>

#include <util-generic/print_exception.hpp>

#include <cstdlib>
#include <iostream>

int main()
try
{
  gspc::ResourceManager resource_manager;
  gspc::ExecutionEnvironment execution_environment;

  auto const resource_manager_id
    (execution_environment.add_resource_manager (resource_manager));

  auto const remote_interface_id
    (execution_environment.bootstrap_remote_interface ("host", "ssh"));

  std::cout << "resource_manager_id " << to_string (resource_manager_id) << std::endl;
  std::cout << "remote_interface_id " << to_string (remote_interface_id) << std::endl;

  // std::cout <<
    execution_environment.add ( remote_interface_id
                              , resource_manager_id
                              , gspc::Tree<gspc::Resource> {gspc::Resource{}}
                              )
            // << std::endl
      ;

  gspc::Workflow workflow ({});
  gspc::Scheduler scheduler {resource_manager, workflow};

  return EXIT_SUCCESS;
}
catch (...)
{
  std::cerr << "EXCEPTION: " << fhg::util::current_exception_printer() << '\n';

  return EXIT_FAILURE;
}
