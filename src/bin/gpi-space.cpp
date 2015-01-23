/*
 * The interface program  providing access to the global,  persistent memory. It
 * provides access to process containers via a UNIX sockets.
 *
 */

#include <fhg/revision.hpp>
#include <fhg/util/boost/program_options/validators/nonempty_string.hpp>
#include <fhg/util/boost/program_options/validators/nonexisting_path.hpp>
#include <fhg/util/boost/program_options/validators/positive_integral.hpp>
#include <fhg/util/make_unique.hpp>
#include <fhg/util/print_exception.hpp>
#include <fhg/util/signal_handler_manager.hpp>
#include <fhg/util/thread/event.hpp>

#include <fhgcom/peer.hpp>

#include <fhglog/LogMacros.hpp>

#include <gpi-space/gpi/api.hpp>
#include <gpi-space/gpi/fake_api.hpp>
#include <gpi-space/gpi/gaspi.hpp>
#include <gpi-space/pc/container/manager.hpp>

#include <boost/asio/io_service.hpp>
#include <boost/filesystem.hpp>
#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/make_shared.hpp>
#include <boost/program_options.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/thread.hpp>

#include <chrono>
#include <memory>

namespace
{
  namespace option
  {
    constexpr const char* const gpi_api ("gpi-api");
    constexpr const char* const gpi_mem ("gpi-mem");
    constexpr const char* const gpi_timeout ("gpi-timeout");
    constexpr const char* const log_file ("log-file");
    constexpr const char* const log_host ("log-host");
    constexpr const char* const log_port ("log-port");
    constexpr const char* const log_level ("log-level");
    constexpr const char* const port ("port");
    constexpr const char* const socket ("socket");
    constexpr const char* const startup_messages_pipe ("startup-messages-pipe");
  }
}

int main (int argc, char** argv)
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
        <fhg::util::boost::program_options::nonempty_string>()->required()
    , "name of log host"
    )
    ( option::log_port
    , boost::program_options::value
        <fhg::util::boost::program_options::positive_integral<unsigned short>>()
        ->required()
    , "port on log-host to log to"
    )
    ( option::log_level
    , boost::program_options::value
        <fhg::util::boost::program_options::nonempty_string>()->required()
    , "log level"
    )
    ( option::gpi_api
    , boost::program_options::value
        <fhg::util::boost::program_options::nonempty_string>()->required()
    , "'fake' or 'gaspi'"
    )
    ( option::gpi_mem
    , boost::program_options::value<std::size_t>()->required()
    , "memory to allocate (in bytes)"
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
        <fhg::util::boost::program_options::nonexisting_path>()->required()
    , "path to open communication socket"
    )
    ( option::startup_messages_pipe
    , boost::program_options::value<int>()->required()
    , "pipe filedescriptor to use for communication during startup (ports used, ...)"
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

  int const startup_messages_pipe_fd
    (vm.at (option::startup_messages_pipe).as<int>());

  boost::filesystem::path const socket_path
    ( vm.at (option::socket)
      .as<fhg::util::boost::program_options::nonexisting_path>()
    );

  boost::optional<boost::filesystem::path> const log_file
    ( vm.count (option::log_file)
    ? boost::optional<boost::filesystem::path>
        (vm.at (option::log_file).as<boost::filesystem::path>())
    : boost::none
    );

  std::string const log_host
    ( vm.at (option::log_host)
      .as<fhg::util::boost::program_options::nonempty_string>()
    );
  unsigned short const log_port
    ( vm.at (option::log_port)
      .as<fhg::util::boost::program_options::positive_integral<unsigned short>>()
    );

  std::string const log_level
    ( vm.at (option::log_level)
      .as<fhg::util::boost::program_options::nonempty_string>()
    );

  std::size_t const gpi_mem (vm.at (option::gpi_mem).as<std::size_t>());

  std::string const gpi_api_string
    ( vm.at (option::gpi_api)
      .as<fhg::util::boost::program_options::nonempty_string>()
    );
  enum requested_api_t { API_fake, API_gaspi };
  requested_api_t const requested_api
    ( gpi_api_string == "gaspi" ? API_gaspi
    : gpi_api_string == "fake" ? API_fake
    : throw std::invalid_argument ("gpi-api must be 'gaspi' or 'fake'")
    );

  std::chrono::seconds const gpi_timeout
    ( vm.at (option::gpi_timeout)
      .as<fhg::util::boost::program_options::positive_integral<std::size_t>>()
    );

  unsigned short const port
    ( vm.at (option::port)
      .as<fhg::util::boost::program_options::positive_integral<unsigned short>>()
    );

  std::string const server_url (log_host + ":" + std::to_string (log_port));
  setenv ("FHGLOG_to_server", server_url.c_str(), true);
  setenv ("FHGLOG_level", log_level.c_str(), true);
  setenv ("FHGLOG_to_console", "stderr", true);

  if (log_file)
  {
    setenv ("FHGLOG_to_file", log_file->string().c_str(), true);
  }

  boost::asio::io_service remote_log_io_service;
  FHGLOG_SETUP (remote_log_io_service);

  boost::asio::io_service topology_peer_io_service;
  boost::shared_ptr<fhg::com::peer_t> topology_peer
    ( boost::make_shared<fhg::com::peer_t>
        (topology_peer_io_service, fhg::com::host_t ("*"), fhg::com::port_t ("0"))
    );


  boost::shared_ptr<boost::thread> topology_peer_thread
    (boost::make_shared<boost::thread> (&fhg::com::peer_t::run, topology_peer));

  try
  {
    topology_peer->start ();
  }
  catch (std::exception const& ex)
  {
    LOG(ERROR, "could not start peer: " << ex.what());
    topology_peer_thread->interrupt();
    topology_peer_thread->join();
    topology_peer.reset();
    topology_peer_thread.reset();
    throw;
  }

  std::unique_ptr<gpi::api::gpi_api_t> const gpi_api
    ( [&gpi_mem, &gpi_timeout, &port, &requested_api, &topology_peer]()
        -> std::unique_ptr<gpi::api::gpi_api_t>
      {
        if (requested_api == API_gaspi)
        {
          return fhg::util::make_unique <gpi::api::gaspi_t>
            (gpi_mem, port, gpi_timeout, topology_peer->local_endpoint().port());
        }
        else
        {
          return fhg::util::make_unique <gpi::api::fake_gpi_api_t>
            (gpi_mem, gpi_timeout, topology_peer->local_endpoint().port());
        }
      }()
    );

  LOG (INFO, "GPI started: " << gpi_api->rank());

  if (0 == gpi_api->rank())
  {
    LOG (INFO, "GPISpace version: " << fhg::project_version());
    LOG (INFO, "GPISpace revision: " << fhg::project_revision());
    LOG (INFO, "GPIApi version: " << gpi_api->version());
  }

    LOG ( TRACE
        ,  "rank=" << gpi_api->rank()
        << " dma=" << gpi_api->dma_ptr()
        << " #nodes=" << gpi_api->number_of_nodes()
        << " mem_size=" << gpi_api->memory_size()
        );

    // other url examples are:
    //      gpi://?buffers=8&buffer_size=4194304 GPI memory
    //      sfs://<path>?create=true&size=1073741824
    const gpi::pc::container::manager_t container_manager
      ( socket_path.string()
      , {"gpi://?buffer_size=4194304&buffers=8"}
      , *gpi_api
      , topology_peer
      );

    LOG (INFO, "started GPI interface on rank " << gpi_api->rank() << " at " << socket_path);

    fhg::util::thread::event<> stop_requested;
    const std::function<void()> request_stop
      (std::bind (&fhg::util::thread::event<>::notify, &stop_requested));

    fhg::util::signal_handler_manager signal_handler;
    fhg::util::scoped_signal_handler const SIGTERM_handler
      (signal_handler, SIGTERM, std::bind (request_stop));
    fhg::util::scoped_signal_handler const SIGINT_handler
      (signal_handler, SIGINT, std::bind (request_stop));

    {
      boost::iostreams::stream<boost::iostreams::file_descriptor_sink>
        startup_messages_pipe ( startup_messages_pipe_fd
                              , boost::iostreams::close_handle
                              );
      startup_messages_pipe << "OKAY\n";
    }

    stop_requested.wait();

    topology_peer->stop();
    if (topology_peer_thread->joinable())
    {
      topology_peer_thread->join();
    }
    topology_peer.reset();
    topology_peer_thread.reset();

    LOG (INFO, "gpi process (rank " << gpi_api->rank() << ") terminated");
    return EXIT_SUCCESS;
}
catch (...)
{
  std::ostringstream ss;
  fhg::util::print_current_exception (ss, "");
  LOG (ERROR, "GPI could not be started: " << ss.str());

  return EXIT_FAILURE;
}
