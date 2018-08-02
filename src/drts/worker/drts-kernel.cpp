// bernd.loerwald@itwm.fraunhofer.de

#include <drts/worker/drts.hpp>

#include <fhg/util/boost/program_options/require_all_if_one.hpp>
#include <fhg/util/boost/program_options/validators/existing_path.hpp>
#include <fhg/util/boost/program_options/validators/positive_integral.hpp>
#include <util-generic/cxx14/make_unique.hpp>
#include <util-generic/print_exception.hpp>
#include <fhg/util/signal_handler_manager.hpp>
#include <fhg/util/thread/event.hpp>

#include <fhglog/Configuration.hpp>

#include <boost/asio/io_service.hpp>
#include <boost/program_options.hpp>
#include <boost/tokenizer.hpp>

#include <rif/started_process_promise.hpp>

#include <hwloc.h>

#include <functional>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
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
    constexpr char const* const backlog_length {"backlog-length"};
    constexpr char const* const library_search_path {"library-search-path"};
    constexpr char const* const socket {"socket"};
    constexpr char const* const master {"master"};
    constexpr char const* const gui_host {"gui-host"};
    constexpr char const* const gui_port {"gui-port"};
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
        ( boost::str ( boost::format ("socket out of range: %1%/%2%")
                     % target_socket
                     % (available_sockets-1)
                     )
        );
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
        ( boost::str
        ( boost::format ("could not bind to socket #%1% with cpuset %2%: %3%")
        % target_socket
        % cpuset_string.data()
        % strerror (errno)
        )
        );
    }

    hwloc_topology_destroy (topology);
  }
}

int main(int ac, char **av)
{
  fhg::rif::started_process_promise promise (ac, av);

  try
  {
    boost::asio::io_service remote_log_io_service;
    fhg::log::Logger logger;
    fhg::log::configure (remote_log_io_service, logger);

    namespace po = boost::program_options;

    po::options_description desc("options");

    std::string kernel_name;

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
      ( option_name::capability
      , po::value<std::vector<std::string>>()
        ->default_value (std::vector<std::string>(), "{}")
      , "capabilities of worker"
      )
      ( option_name::backlog_length
      , po::value<std::size_t>()->required()
      , "length of job backlog"
      )
      ( option_name::library_search_path
      , po::value<std::vector<boost::filesystem::path>>()
        ->default_value (std::vector<boost::filesystem::path>(), "{}")
      , "paths to search for module call libraries"
      )
      ( option_name::socket
      , po::value<std::size_t>()
      , "socket to pin worker on"
      )
      ( option_name::master
      , po::value<std::vector<std::string>>()->required()
      , "masters to connect to (unique_name%host%port)"
      )
      ( option_name::gui_host
      , po::value<std::string>()
      , "host to send gui notifications to"
      )
      ( option_name::gui_port
      , po::value
          <fhg::util::boost::program_options::positive_integral<unsigned short>>()
      , "port to send gui notifications to"
      )
      ;

    po::variables_map vm;

    po::store (po::command_line_parser (ac, av).options(desc).run(), vm);
    po::notify (vm);

    fhg::util::boost::program_options::require_all_if_one
      (vm, {option_name::gui_host, option_name::gui_port});

    fhg::util::thread::event<> stop_requested;
    const std::function<void()> request_stop
      (std::bind (&fhg::util::thread::event<>::notify, &stop_requested));

    fhg::util::signal_handler_manager signal_handlers;
    fhg::util::scoped_log_backtrace_and_exit_for_critical_errors const
      crit_error_handler (signal_handlers, logger);

    fhg::util::scoped_signal_handler const SIGTERM_handler
      (signal_handlers, SIGTERM, std::bind (request_stop));
    fhg::util::scoped_signal_handler const SIGINT_handler
      (signal_handlers, SIGINT, std::bind (request_stop));

    std::unique_ptr<gpi::pc::client::api_t> const virtual_memory_api
      ( vm.count (option_name::virtual_memory_socket)
      ? fhg::util::cxx14::make_unique<gpi::pc::client::api_t>
          ( logger
          , (static_cast<boost::filesystem::path>
              ( vm.at (option_name::virtual_memory_socket)
              .as<fhg::util::boost::program_options::existing_path>()
              )
            ).string()
          )
      : nullptr
      );
    std::unique_ptr<gspc::scoped_allocation> const shared_memory
      ( ( virtual_memory_api
        && vm.count (option_name::shared_memory_size)
        && vm.at (option_name::shared_memory_size).as<unsigned long>() > 0
        )
      ? fhg::util::cxx14::make_unique<gspc::scoped_allocation>
        ( virtual_memory_api
        , kernel_name + "-shared_memory"
        , vm.at (option_name::shared_memory_size).as<unsigned long>()
        )
      : nullptr
      );

    std::vector<DRTSImpl::master_info> master_info;
    std::set<std::string> seen_master_names;
    for ( std::string const& master
        : vm.at (option_name::master).as<std::vector<std::string>>()
        )
    {
      boost::tokenizer<boost::char_separator<char>> const tok
        (master, boost::char_separator<char> ("%"));

      std::vector<std::string> const parts (tok.begin(), tok.end());

      if (parts.size() != 3)
      {
        throw std::invalid_argument
          ("invalid master information: has to be of format 'name%host%port'");
      }

      if (!seen_master_names.emplace (master).second)
      {
        throw std::invalid_argument ("master already specified: " + master);
      }

      master_info.emplace_back
        (fhg::com::host_t (parts[1]), fhg::com::port_t (parts[2]));
    }

    boost::asio::io_service gui_io_service;

    if (vm.count (option_name::socket))
    {
      set_numa_socket (vm.at (option_name::socket).as<std::size_t>());
    }

    DRTSImpl const plugin
      ( request_stop
      , fhg::util::cxx14::make_unique<boost::asio::io_service>()
      , (vm.count (option_name::gui_host) || vm.count (option_name::gui_port))
        ? fhg::util::cxx14::make_unique<sdpa::daemon::NotificationService>
          ( vm.at (option_name::gui_host).as<std::string>()
          , vm.at (option_name::gui_port)
            .as<fhg::util::boost::program_options::positive_integral<unsigned short>>()
          , gui_io_service
          )
        : nullptr
      , kernel_name
      , virtual_memory_api.get()
      , shared_memory.get()
      , master_info
      , vm.at (option_name::capability)
      .as<std::vector<std::string>>()
      , vm.at (option_name::library_search_path)
      .as<std::vector<boost::filesystem::path>>()
      , vm.at (option_name::backlog_length)
      .as<std::size_t>()
      , logger
      );

    promise.set_result ( { plugin.logger_registration_endpoint().host
                         , std::to_string
                             (plugin.logger_registration_endpoint().port)
                         }
                       );

    stop_requested.wait();

    return 0;
  }
  catch (...)
  {
    promise.set_exception (std::current_exception());

    return EXIT_FAILURE;
  }
}
