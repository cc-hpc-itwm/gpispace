#include <fhg/revision.hpp>
#include <fhg/util/boost/program_options/validators/nonempty_string.hpp>
#include <fhg/util/boost/program_options/validators/nonexisting_path.hpp>
#include <fhg/util/boost/program_options/validators/nonexisting_path_in_existing_directory.hpp>
#include <fhg/util/boost/program_options/validators/positive_integral.hpp>
#include <fhg/util/signal_handler_manager.hpp>
#include <fhg/util/thread/event.hpp>
#include <util-generic/hostname.hpp>

#include <rif/protocol.hpp>
#include <rif/started_process_promise.hpp>

#include <vmem/gaspi/equally_distributed_segment.hpp>
#include <vmem/ipc_server.hpp>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

#include <chrono>
#include <memory>

namespace
{
  namespace option
  {
    constexpr const char* const gpi_timeout ("gpi-timeout");
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

    std::chrono::seconds const gpi_timeout
      ( vm.at (option::gpi_timeout)
        .as<fhg::util::boost::program_options::positive_integral<std::size_t>>()
      );

    unsigned short const gpi_port
      ( vm.at (option::port)
        .as<fhg::util::boost::program_options::positive_integral<unsigned short>>()
      );

    struct hopefully_free_port
    {
      uint16_t get() const
      {
        return _provider->local_endpoint().port();
      }
      uint16_t release()
      {
        auto res (get());
        _provider.reset();
        return res;
      }

      boost::asio::io_service _io_service;
      fhg::rpc::service_dispatcher _dispatcher;
      std::unique_ptr<fhg::rpc::service_tcp_provider_with_deferred_start>
        _provider = fhg::util::cxx14::make_unique<fhg::rpc::service_tcp_provider_with_deferred_start> (_io_service, _dispatcher);
    } intertwine_comm_port;

    std::promise<std::vector<intertwine::vmem::node>> nodes_promise;
    std::promise<void> setup_finished;

    fhg::rpc::service_dispatcher setup_dispatcher;
    fhg::rpc::service_handler
      <fhg::rif::protocol::local::vmem_set_nodes_and_wait_for_startup>
        setup_handler { setup_dispatcher
                      , [&] (std::vector<intertwine::vmem::node> nodes_)
                        {
                          nodes_promise.set_value (std::move (nodes_));
                          setup_finished.get_future().get();
                        }
                      };
    fhg::util::scoped_boost_asio_io_service_with_threads setup_thread {1};
    fhg::rpc::service_socket_provider setup_provider
      {setup_thread, setup_dispatcher};

    promise.set_result ( { setup_provider.local_endpoint().path()
                         , std::to_string (intertwine_comm_port.get())
                         }
                       );

    auto nodes (nodes_promise.get_future().get());

    intertwine::vmem::server server
      ( intertwine::vmem::gaspi::context::params {gpi_timeout, gpi_port}
      , intertwine::vmem::beegfs::context::params{}
      , nodes
      , intertwine_comm_port.release()
      );
    intertwine::vmem::ipc_server ipc_server (server, socket_path);

    fhg::util::thread::event<> stop_requested;
    const std::function<void()> request_stop
      (std::bind (&fhg::util::thread::event<>::notify, &stop_requested));

    fhg::util::signal_handler_manager signal_handler;
    fhg::util::scoped_signal_handler const SIGTERM_handler
      (signal_handler, SIGTERM, std::bind (request_stop));
    fhg::util::scoped_signal_handler const SIGINT_handler
      (signal_handler, SIGINT, std::bind (request_stop));

    setup_finished.set_value();

    stop_requested.wait();

    return EXIT_SUCCESS;
  }
  catch (...)
  {
    promise.set_exception (std::current_exception());

    return EXIT_FAILURE;
  }
}
