#include <gspc/example/rtm/Parameter.hpp>

#include <gspc/Resource.hpp>
#include <gspc/resource_manager/Trivial.hpp>

#include <util-generic/print_exception.hpp>
#include <util-generic/timer/application.hpp>

#include <iostream>
#include <stdexcept>

namespace
{
  template<typename Echo, typename RM>
    struct acquisition
  {
    acquisition (Echo& echo_, RM& rm_) : echo (echo_), rm (rm_) {}

    void acquire
      (gspc::resource::Class resource_class, std::size_t N)
    {
      echo.section ("acquire " + std::to_string (N) + " x " + resource_class);

      while (N --> 0)
      {
        acquired.emplace (rm.acquire (resource_class).requested);
      }
    }
    ~acquisition()
    {
      echo.section ("release all acquired resources");

      for (auto resource_id : acquired)
      {
        rm.release (typename RM::Acquired {resource_id});
      }
    }

    Echo& echo;
    RM& rm;
    std::unordered_set<gspc::resource::ID> acquired;
  };
}

int main ()
try
{
  fhg::util::default_application_timer echo ("massive_acquire");

  std::size_t const num_cores_per_socket (20);
  std::size_t const num_sockets          (4);
  std::size_t const num_sockets_with_gpu (2);
  std::size_t const num_loads            (8);
  std::size_t const num_stores           (8);

  std::size_t const num_nodes            (1000);

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

  echo.section ("create resource trees");

  using RS = gspc::Forest<gspc::resource::ID, gspc::resource::Class>;

  RS resources;

  gspc::remote_interface::ID next_remote_interface_id {0};

  for (std::size_t n (0); n < num_nodes; ++n, ++next_remote_interface_id)
  {
    gspc::resource::ID next_resource_id {{next_remote_interface_id}};

    resources.UNSAFE_merge
      ( host_topology.unordered_transform
        ( [&] (gspc::unique_forest::Node<gspc::Resource> const& r) -> RS::Node
          {
            return {++next_resource_id, r.second.resource_class};
          }
        )
      );
  }

  echo.section ("create resource manager and add resources");

  using RM = gspc::resource_manager::Trivial;

  RM resource_manager;
  resource_manager.add (resources);

  {
    acquisition<decltype (echo), RM> acquisition (echo, resource_manager);

    acquisition.acquire
      ("core", num_cores_per_socket * num_sockets * num_nodes);
  }

  {
    acquisition<decltype (echo), RM> acquisition (echo, resource_manager);

    acquisition.acquire ("gpu", num_sockets_with_gpu * num_nodes / 2);
    acquisition.acquire ("load", num_loads * num_nodes);
    acquisition.acquire ("store", num_stores * num_nodes);
  }

  echo.section ("cleanup");

  return EXIT_SUCCESS;
}
catch (...)
{
  std::cerr << "EXCEPTION: " << fhg::util::current_exception_printer() << '\n';

  return EXIT_FAILURE;
}
