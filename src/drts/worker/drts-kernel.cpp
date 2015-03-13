// bernd.loerwald@itwm.fraunhofer.de

#include <drts/worker/drts.hpp>

#include <fhg/util/boost/program_options/require_all_if_one.hpp>
#include <fhg/util/boost/program_options/validators/existing_path.hpp>
#include <fhg/util/boost/program_options/validators/positive_integral.hpp>
#include <fhg/util/make_unique.hpp>
#include <fhg/util/print_exception.hpp>
#include <fhg/util/signal_handler_manager.hpp>
#include <fhg/util/thread/event.hpp>

#include <fhglog/Configuration.hpp>
#include <fhglog/LogMacros.hpp>

#include <boost/asio/io_service.hpp>
#include <boost/program_options.hpp>
#include <boost/tokenizer.hpp>

#include <rif/startup_messages_pipe.hpp>

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
}

int main(int ac, char **av)
try
{
  boost::asio::io_service remote_log_io_service;
  fhg::log::Logger& logger (fhg::log::GLOBAL_logger());
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

  desc.add (fhg::rif::startup_messages_pipe::program_options());

  po::variables_map vm;
  try
  {
    po::store( po::command_line_parser(ac, av)
             . options(desc).run()
             , vm
             );
  }
  catch (std::exception const &ex)
  {
    LLOG (ERROR, logger, "invalid command line: " << ex.what());
    return EXIT_FAILURE;
  }
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
      ? fhg::util::make_unique<gpi::pc::client::api_t>
        ((static_cast<boost::filesystem::path>
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
      ? fhg::util::make_unique<gspc::scoped_allocation>
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
      (parts[0], fhg::com::host_t (parts[1]), fhg::com::port_t (parts[2]));
  }

  boost::asio::io_service gui_io_service;

  DRTSImpl const plugin
    ( request_stop
    , fhg::util::make_unique<boost::asio::io_service>()
    , (vm.count (option_name::gui_host) || vm.count (option_name::gui_port))
      ? fhg::util::make_unique<sdpa::daemon::NotificationService>
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
    , vm.count (option_name::socket)
    ? vm.at (option_name::socket).as<std::size_t>()
    : boost::optional<std::size_t>()
    , vm.at (option_name::library_search_path)
    .as<std::vector<boost::filesystem::path>>()
    , vm.at (option_name::backlog_length)
    .as<std::size_t>()
    , logger
    );

  {
    fhg::rif::startup_messages_pipe startup_messages_pipe (vm);
  }

  stop_requested.wait();

  return 0;
}
catch (...)
{
  fhg::util::print_current_exception (std::cerr, "EX: ");
  return 1;
}
