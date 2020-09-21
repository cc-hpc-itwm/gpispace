/*
 * The interface program  providing access to the global,  persistent memory. It
 * provides access to process containers via a UNIX sockets.
 *
 */

#include <fhg/revision.hpp>
#include <fhg/util/boost/program_options/validators/nonempty_string.hpp>
#include <fhg/util/boost/program_options/validators/nonexisting_path.hpp>
#include <fhg/util/boost/program_options/validators/nonexisting_path_in_existing_directory.hpp>
#include <fhg/util/boost/program_options/validators/positive_integral.hpp>
#include <util-generic/cxx14/make_unique.hpp>
#include <util-generic/getenv.hpp>
#include <util-generic/print_exception.hpp>
#include <util-generic/scoped_boost_asio_io_service_with_threads.hpp>
#include <util-generic/syscall.hpp>

#include <iml/vmem/gaspi/gpi/gaspi.hpp>
#include <iml/vmem/gaspi/pc/container/manager.hpp>

#include <iml/rif/started_process_promise.hpp>

#include <iml/vmem/gaspi_context.hpp>
#include <iml/vmem/netdev_id.hpp>

#include <util-generic/syscall.hpp>
#include <util-generic/syscall/process_signal_block.hpp>
#include <util-generic/syscall/signal_set.hpp>

#include <boost/asio/io_service.hpp>
#include <boost/filesystem.hpp>
#include <boost/format.hpp>
#include <boost/make_shared.hpp>
#include <boost/program_options.hpp>
#include <boost/thread/scoped_thread.hpp>

#include <chrono>
#include <cstddef>
#include <exception>
#include <stdexcept>
#include <utility>

namespace
{
  namespace option
  {
    constexpr const char* const gpi_timeout ("gpi-timeout");
    constexpr const char* const port ("port");
    constexpr const char* const socket ("socket");
    constexpr const char* const netdev ("netdev");
  }
}

int main (int argc, char** argv)
{
  fhg::iml::rif::started_process_promise promise (argc, argv);

  // \todo Move to util-generic and use in other server
  // processes. Copy of this exists in gspc-logging-*.
  struct
  {
    fhg::util::syscall::signal_set const signals = {SIGINT, SIGTERM};
    fhg::util::syscall::process_signal_block const signal_block = {signals};

    void wait() const
    {
      fhg::util::syscall::sigwaitinfo (&signals._, nullptr);
    }
  } const stop_requested;

  try
  {

    boost::program_options::options_description options_description;
    options_description.add_options()
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
      ( option::netdev
      , boost::program_options::value<fhg::iml::vmem::netdev_id>()->required()
      , "the network device to use"
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

    std::chrono::seconds const gpi_timeout
      ( vm.at (option::gpi_timeout)
        .as<fhg::util::boost::program_options::positive_integral<std::size_t>>()
      );

    unsigned short const port
      ( vm.at (option::port)
        .as<fhg::util::boost::program_options::positive_integral<unsigned short>>()
      );

    //! \todo more than one thread, parameter
    fhg::util::scoped_boost_asio_io_service_with_threads
      topology_server_io_service (8);
    auto topology_rpc_server
      ( fhg::util::cxx14::make_unique
          <fhg::rpc::service_tcp_provider_with_deferred_dispatcher>
            (topology_server_io_service)
      );

    fhg::iml::vmem::gaspi_timeout initialization_timeout (gpi_timeout);
    fhg::iml::vmem::gaspi_context gaspi_context
      ( initialization_timeout
      , port
      , topology_rpc_server->local_endpoint().port()
      , vm.at (option::netdev).as<fhg::iml::vmem::netdev_id>()
      );

    const gpi::pc::container::manager_t container_manager
      ( socket_path.string()
      , gaspi_context
      , std::move (topology_rpc_server)
      );

    promise.set_result();

    stop_requested.wait();

    return EXIT_SUCCESS;
  }
  catch (...)
  {
    promise.set_exception (std::current_exception());

    return EXIT_FAILURE;
  }
}
