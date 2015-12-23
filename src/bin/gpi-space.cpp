/*
 * The interface program  providing access to the global,  persistent memory. It
 * provides access to process containers via a UNIX sockets.
 *
 */

#include <fhg/revision.hpp>
#include <fhg/util/boost/program_options/require_all_if_one.hpp>
#include <fhg/util/boost/program_options/validators/nonempty_string.hpp>
#include <fhg/util/boost/program_options/validators/nonexisting_path.hpp>
#include <fhg/util/boost/program_options/validators/nonexisting_path_in_existing_directory.hpp>
#include <fhg/util/boost/program_options/validators/positive_integral.hpp>
#include <util-generic/cxx14/make_unique.hpp>
#include <util-generic/print_exception.hpp>
#include <fhg/util/signal_handler_manager.hpp>
#include <fhg/util/thread/event.hpp>

#include <fhglog/Configuration.hpp>
#include <fhglog/LogMacros.hpp>

#include <gpi-space/gpi/gaspi.hpp>
#include <gpi-space/pc/container/manager.hpp>

#include <rif/started_process_promise.hpp>

#include <vmem/gaspi_context.hpp>

#include <boost/asio/io_service.hpp>
#include <boost/filesystem.hpp>
#include <boost/make_shared.hpp>
#include <boost/program_options.hpp>
#include <boost/thread/scoped_thread.hpp>

#include <chrono>
#include <memory>

namespace
{
  namespace option
  {
    constexpr const char* const gpi_timeout ("gpi-timeout");
    constexpr const char* const log_file ("log-file");
    constexpr const char* const log_host ("log-host");
    constexpr const char* const log_port ("log-port");
    constexpr const char* const log_level ("log-level");
    constexpr const char* const port ("port");
    constexpr const char* const socket ("socket");
  }
}

int main (int argc, char** argv)
{
  fhg::rif::started_process_promise promise (argc, argv);

  try
  {

    boost::program_options::options_description options_description;
    options_description.add_options()
      ( option::log_file
      , boost::program_options::value<boost::filesystem::path>()
      , "file to log to"
      )
      ( option::log_host
      , boost::program_options::value
          <fhg::util::boost::program_options::nonempty_string>()
      , "name of log host"
      )
      ( option::log_port
      , boost::program_options::value
          <fhg::util::boost::program_options::positive_integral<unsigned short>>()
      , "port on log-host to log to"
      )
      ( option::log_level
      , boost::program_options::value
          <fhg::util::boost::program_options::nonempty_string>()->required()
      , "log level"
      )
      ( option::gpi_timeout
      , boost::program_options::value
          <fhg::util::boost::program_options::positive_integral<std::size_t>>()
          ->required()
      , "seconds to wait for gaspi_proc_init"
      )
      ( option::port
      , boost::program_options::value
          <fhg::util::boost::program_options::positive_integral<unsigned short>>()
          ->required()
      , "port used for gaspi"
      )
      ( option::socket
      , boost::program_options::value
          <fhg::util::boost::program_options::nonexisting_path_in_existing_directory>()
          ->required()
      , "path to open communication socket"
      )
      ;

    boost::program_options::variables_map vm;
    boost::program_options::store
      ( boost::program_options::command_line_parser (argc, argv)
        .options (options_description)
        .run()
      , vm
      );

    boost::program_options::notify (vm);

    boost::filesystem::path const socket_path
      ( vm.at (option::socket)
        .as<fhg::util::boost::program_options::nonexisting_path_in_existing_directory>()
      );

    boost::optional<boost::filesystem::path> const log_file
      ( vm.count (option::log_file)
      ? boost::optional<boost::filesystem::path>
          (vm.at (option::log_file).as<boost::filesystem::path>())
      : boost::none
      );

    fhg::util::boost::program_options::require_all_if_one
      (vm, {option::log_host, option::log_port});

    boost::optional<std::string> const log_host
      ( vm.count (option::log_host)
      ? boost::optional<std::string>
          ( vm.at (option::log_host)
            .as<fhg::util::boost::program_options::nonempty_string>()
          )
      : boost::none
      );
    boost::optional<unsigned short> const log_port
      ( vm.count (option::log_port)
      ? boost::optional<unsigned short>
          ( vm.at (option::log_port)
            .as<fhg::util::boost::program_options::positive_integral<unsigned short>>()
          )
      : boost::none
      );

    std::string const log_level
      ( vm.at (option::log_level)
        .as<fhg::util::boost::program_options::nonempty_string>()
      );

    std::chrono::seconds const gpi_timeout
      ( vm.at (option::gpi_timeout)
        .as<fhg::util::boost::program_options::positive_integral<std::size_t>>()
      );

    unsigned short const port
      ( vm.at (option::port)
        .as<fhg::util::boost::program_options::positive_integral<unsigned short>>()
      );

    if (log_host || log_port)
    {
      std::string const server_url (*log_host + ":" + std::to_string (*log_port));
      setenv ("FHGLOG_to_server", server_url.c_str(), true);
    }
    setenv ("FHGLOG_level", log_level.c_str(), true);

    if (log_file)
    {
      setenv ("FHGLOG_to_file", log_file->string().c_str(), true);
    }

    boost::asio::io_service remote_log_io_service;
    fhg::log::Logger logger;
    fhg::log::configure (remote_log_io_service, logger);

    fhg::util::signal_handler_manager signal_handler;
    fhg::util::scoped_log_backtrace_and_exit_for_critical_errors const
      crit_error_handler (signal_handler, logger);

    auto topology_rpc_server
      (fhg::util::cxx14::make_unique<fhg::rpc::server_with_multiple_clients_and_deferred_dispatcher>());

    fhg::vmem::gaspi_timeout initialization_timeout (gpi_timeout);
    fhg::vmem::gaspi_context gaspi_context
      ( initialization_timeout
      , port
      , topology_rpc_server->local_endpoint().port()
      );

    const gpi::pc::container::manager_t container_manager
      ( logger
      , socket_path.string()
      , gaspi_context
      , std::move (topology_rpc_server)
      );

    fhg::util::thread::event<> stop_requested;
    const std::function<void()> request_stop
      (std::bind (&fhg::util::thread::event<>::notify, &stop_requested));

    fhg::util::scoped_signal_handler const SIGTERM_handler
      (signal_handler, SIGTERM, std::bind (request_stop));
    fhg::util::scoped_signal_handler const SIGINT_handler
      (signal_handler, SIGINT, std::bind (request_stop));

    promise.set_result ({});

    stop_requested.wait();

    return EXIT_SUCCESS;
  }
  catch (...)
  {
    promise.set_exception (std::current_exception());

    return EXIT_FAILURE;
  }
}
