// Copyright (C) 2010-2012,2014-2016,2019-2026 Fraunhofer ITWM
// SPDX-License-Identifier: GPL-3.0-or-later

/*
 * The interface program  providing access to the global,  persistent memory. It
 * provides access to process containers via a UNIX sockets.
 *
 */

#include <gspc/util/boost/program_options/validators/nonempty_string.hpp>
#include <gspc/util/boost/program_options/validators/nonexisting_path.hpp>
#include <gspc/util/boost/program_options/validators/nonexisting_path_in_existing_directory.hpp>
#include <gspc/util/boost/program_options/validators/positive_integral.hpp>
#include <gspc/util/print_exception.hpp>
#include <gspc/util/scoped_boost_asio_io_service_with_threads.hpp>
#include <gspc/util/syscall.hpp>
#include <gspc/util/syscall/process_signal_block.hpp>
#include <gspc/util/syscall/signal_set.hpp>

#include <gspc/iml/vmem/gaspi/gpi/gaspi.hpp>
#include <gspc/iml/vmem/gaspi/pc/container/manager.hpp>

#include <gspc/rif/started_process_promise.hpp>

#include <gspc/iml/gaspi/NetdevID.hpp>
#include <gspc/iml/vmem/gaspi_context.hpp>

#include <boost/asio/io_service.hpp>
#include <boost/program_options.hpp>

#include <filesystem>
#include <boost/thread/scoped_thread.hpp>

#include <chrono>
#include <cstddef>
#include <exception>
#include <memory>
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
  gspc::rif::started_process_promise promise (argc, argv);

  // \todo Move to util-generic and use in other server
  // processes. Copy of this exists in gspc-logging-*.
  struct
  {
    gspc::util::syscall::signal_set const signals = {SIGINT, SIGTERM};
    gspc::util::syscall::process_signal_block const signal_block = {signals};

    void wait() const
    {
      gspc::util::syscall::sigwaitinfo (&signals._, nullptr);
    }
  } const stop_requested;

  try
  {

    ::boost::program_options::options_description options_description;
    options_description.add_options()
      ( option::gpi_timeout
      , ::boost::program_options::value
          <gspc::util::boost::program_options::positive_integral<std::size_t>>()
          ->required()
      , "seconds to wait for gaspi_proc_init"
      )
      ( option::port
      , ::boost::program_options::value
          <gspc::util::boost::program_options::positive_integral<unsigned short>>()
          ->required()
      , "port used for gaspi"
      )
      ( option::socket
      , ::boost::program_options::value
          <gspc::util::boost::program_options::nonexisting_path_in_existing_directory>()
          ->required()
      , "path to open communication socket"
      )
      ( option::netdev
      , ::boost::program_options::value<gspc::iml::gaspi::NetdevID>()->required()
      , "the network device to use"
      )
      ;

    ::boost::program_options::variables_map vm;
    ::boost::program_options::store
      ( ::boost::program_options::command_line_parser (argc, argv)
        .options (options_description)
        .run()
      , vm
      );

    ::boost::program_options::notify (vm);

    std::filesystem::path const socket_path
      { vm.at (option::socket)
          .as<gspc::util::boost::program_options::nonexisting_path_in_existing_directory>()
      };

    std::chrono::seconds const gpi_timeout
      ( vm.at (option::gpi_timeout)
        .as<gspc::util::boost::program_options::positive_integral<std::size_t>>()
      );

    unsigned short const port
      ( vm.at (option::port)
        .as<gspc::util::boost::program_options::positive_integral<unsigned short>>()
      );

    //! \todo more than one thread, parameter
    gspc::util::scoped_boost_asio_io_service_with_threads
      topology_server_io_service (8);
    auto topology_rpc_server
      ( std::make_unique
          <gspc::rpc::service_tcp_provider_with_deferred_dispatcher>
            (topology_server_io_service)
      );

    gspc::iml::vmem::gaspi_timeout initialization_timeout (gpi_timeout);
    gspc::iml::vmem::gaspi_context gaspi_context
      ( initialization_timeout
      , port
      , topology_rpc_server->local_endpoint().port()
      , vm.at (option::netdev).as<gspc::iml::gaspi::NetdevID>()
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
