// Copyright (C) 2025 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

#include <drts/worker/drts.hpp>

#include <fhg/util/signal_handler_manager.hpp>
#include <util-generic/boost/program_options/validators/existing_path.hpp>
#include <util-generic/print_exception.hpp>

#include <boost/asio/io_service.hpp>
#include <boost/program_options.hpp>
#include <boost/tokenizer.hpp>

#include <rif/started_process_promise.hpp>

#include <hwloc.h>

#include <fmt/core.h>
#include <memory>
#include <set>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

//! \todo move to a central option name collection
namespace
{
  namespace option_name
  {
    constexpr char const* const virtual_memory_socket
      {"virtual-memory-socket"};
    constexpr char const* const shared_memory_size
      {"shared-memory-size"};
    constexpr char const* const capability {"capability"};
    constexpr char const* const library_search_path {"library-search-path"};
    constexpr char const* const socket {"socket"};
    constexpr char const* const parent {"parent"};
    constexpr char const* const certificates {"certificates"};
  }

  void set_numa_socket (std::size_t target_socket)
  {
    hwloc_topology_t topology;

    if (hwloc_topology_init (&topology) != 0)
    {
      throw std::runtime_error ("hwloc_topology_init failed");
    }
    if (hwloc_topology_load (topology) != 0)
    {
      throw std::runtime_error ("hwloc_topology_load failed");
    }

    const int depth (hwloc_get_type_depth (topology, HWLOC_OBJ_SOCKET));
    if (depth == HWLOC_TYPE_DEPTH_UNKNOWN)
    {
      throw std::runtime_error ("could not get number of sockets");
    }

    const size_t available_sockets (hwloc_get_nbobjs_by_depth (topology, depth));

    if (target_socket >= available_sockets)
    {
      throw std::runtime_error
        { fmt::format ( "socket out of range: {}/{}"
                      , target_socket
                      , available_sockets - 1
                      )
        };
    }

    const hwloc_obj_t obj
      (hwloc_get_obj_by_depth (topology, depth, target_socket));

    if (obj == nullptr)
    {
      throw std::runtime_error ("hwloc_get_obj_by_depth failed");
    }

    if (hwloc_set_cpubind (topology, obj->cpuset, HWLOC_CPUBIND_PROCESS) != 0)
    {
      std::vector<char> cpuset_string (256);
      while (static_cast<std::size_t>
            (hwloc_bitmap_snprintf ( cpuset_string.data()
                                   , cpuset_string.size()
                                   , obj->cpuset
                                   )
            ) >= cpuset_string.size()
            )
      {
        cpuset_string.resize (2 * cpuset_string.size());
      }

      throw std::runtime_error
        { fmt::format
            ( "could not bind to socket #{} with cpuset {}: {}"
            , target_socket
            , cpuset_string.data()
            , strerror (errno)
            )
        };
    }

    hwloc_topology_destroy (topology);
  }
}

int main (int ac, char **av)
{
  fhg::rif::started_process_promise promise (ac, av);

  try
  {
    namespace po = ::boost::program_options;

    po::options_description desc("options");

    std::string kernel_name;
    unsigned short comm_port;

    desc.add_options()
      ("name,n", po::value<std::string>(&kernel_name), "give the kernel a name")
      ( option_name::virtual_memory_socket
      , po::value<fhg::util::boost::program_options::existing_path>()
      , "socket file to communicate with the virtual memory manager"
        ", if given the virtual memory manager is required to be running"
        ", if not given, the kernel can not manage memory"
      )
      ( option_name::shared_memory_size
      , po::value<unsigned long>()
      , "size of shared memory associated with the kernel"
      )
      ( "port,p"
      , po::value<unsigned short>(&comm_port)->default_value(0)
      , "workers's communication port"
      )
      ( option_name::capability
      , po::value<std::vector<std::string>>()
        ->default_value (std::vector<std::string>(), "{}")
      , "capabilities of worker"
      )
      ( option_name::library_search_path
      , po::value<std::vector<::boost::filesystem::path>>()
        ->default_value (std::vector<::boost::filesystem::path>(), "{}")
      , "paths to search for module call libraries"
      )
      ( option_name::socket
      , po::value<std::size_t>()
      , "socket to pin worker on"
      )
      ( option_name::parent
      , po::value<std::string>()->required()
      , "parent to connect to (unique_name%host%port)"
      )
      ( option_name::certificates
      , po::value<::boost::filesystem::path>()
      , "folder containing SSL certificates"
      )
      ;

    po::variables_map vm;

    po::store (po::command_line_parser (ac, av).options (desc).run(), vm);
    po::notify (vm);

    fhg::logging::stream_emitter log_emitter;

    fhg::util::signal_handler_manager signal_handlers;
    fhg::util::scoped_log_backtrace_and_exit_for_critical_errors const
      crit_error_handler (signal_handlers, log_emitter);

    fhg::util::Execution execution (signal_handlers);

    std::unique_ptr<iml::Client> const virtual_memory_api
      (
      #if GSPC_WITH_IML
        vm.count (option_name::virtual_memory_socket)
      ? std::make_unique<iml::Client>
          ( static_cast<::boost::filesystem::path>
              ( vm.at (option_name::virtual_memory_socket)
              .as<fhg::util::boost::program_options::existing_path>()
              )
          )
      :
      #endif
        nullptr
      );
    std::unique_ptr<iml::SharedMemoryAllocation> const shared_memory
      (
      #if GSPC_WITH_IML
        ( virtual_memory_api
        && vm.count (option_name::shared_memory_size)
        && vm.at (option_name::shared_memory_size).as<unsigned long>() > 0
        )
      ? std::make_unique<iml::SharedMemoryAllocation>
        ( *virtual_memory_api
        , vm.at (option_name::shared_memory_size).as<unsigned long>()
        )
      :
      #endif
        nullptr
      );

    auto const parent_info
      ( [&]() -> std::tuple<fhg::com::host_t, fhg::com::port_t>
        {
          ::boost::tokenizer<::boost::char_separator<char>> const tok
            ( vm.at (option_name::parent).as<std::string>()
            , ::boost::char_separator<char> ("%")
            );

          std::vector<std::string> const parts (tok.begin(), tok.end());

          if (parts.size() != 3)
          {
            throw std::invalid_argument
              ("invalid parent information: has to be of format 'name%host%port'");
          }

          unsigned short port;
          std::istringstream iss (parts.at (2));
          iss >> port;

          return std::make_tuple
            (fhg::com::host_t (parts[1]), fhg::com::port_t {port});
        }()
      );

    if (vm.count (option_name::socket))
    {
      set_numa_socket (vm.at (option_name::socket).as<std::size_t>());
    }

    gspc::Certificates certificates;

    if (vm.count (option_name::certificates))
    {
      certificates = vm.at (option_name::certificates).as<::boost::filesystem::path>();
    }

    DRTSImpl const plugin
      ( [&] { execution.stop(); }
      , std::make_unique<::boost::asio::io_service>()
      , kernel_name
      , comm_port
      , virtual_memory_api.get()
      , shared_memory.get()
      , parent_info
      , vm.at (option_name::capability)
      .as<std::vector<std::string>>()
      , vm.at (option_name::library_search_path)
      .as<std::vector<::boost::filesystem::path>>()
      , log_emitter
      , certificates
      );

    promise.set_result (log_emitter.local_endpoint().to_string());

    execution.wait();

    return 0;
  }
  catch (...)
  {
    promise.set_exception (std::current_exception());

    return EXIT_FAILURE;
  }
}
