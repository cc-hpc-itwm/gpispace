#include <fhg/util/boost/program_options/validators/existing_path.hpp>
#include <fhg/util/signal_handler_manager.hpp>
#include <fhg/util/thread/event.hpp>

#include <rif/started_process_promise.hpp>

#include <vmem/ipc_client.hpp>

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>

namespace
{
  namespace option
  {
    constexpr const char* const cache_size ("cache-size");
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
      ( option::cache_size
      , boost::program_options::value<std::size_t>()->required()
      , "size of cache to create"
      )
      ( option::socket
      , boost::program_options::value
          <fhg::util::boost::program_options::existing_path>()->required()
      , "path to communication socket"
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
        .as<fhg::util::boost::program_options::existing_path>()
      );

    intertwine::vmem::size_t const cache_size
      (vm.at (option::cache_size).as<std::size_t>());

    intertwine::vmem::ipc_client client (socket_path);

    auto const cache_id (client.shareable_cache_create (cache_size));

    fhg::util::thread::event<> stop_requested;
    const std::function<void()> request_stop
      (std::bind (&fhg::util::thread::event<>::notify, &stop_requested));

    fhg::util::signal_handler_manager signal_handler;
    fhg::util::scoped_signal_handler const SIGTERM_handler
      (signal_handler, SIGTERM, std::bind (request_stop));
    fhg::util::scoped_signal_handler const SIGINT_handler
      (signal_handler, SIGINT, std::bind (request_stop));

    promise.set_result ({cache_id.to_string()});

    stop_requested.wait();

    client.cache_delete (cache_id);

    return EXIT_SUCCESS;
  }
  catch (...)
  {
    promise.set_exception (std::current_exception());

    return EXIT_FAILURE;
  }
}
