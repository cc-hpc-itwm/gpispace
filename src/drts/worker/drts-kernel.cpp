// bernd.loerwald@itwm.fraunhofer.de

#include <drts/worker/drts.hpp>

#include <fhg/util/boost/program_options/validators/existing_path.hpp>
#include <fhg/util/make_unique.hpp>
#include <fhg/util/print_exception.hpp>
#include <fhg/util/signal_handler_manager.hpp>
#include <fhg/util/split.hpp>
#include <fhg/util/thread/event.hpp>
#include <fhglog/Configuration.hpp>
#include <fhglog/LogMacros.hpp>

#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/program_options.hpp>
#include <boost/tokenizer.hpp>

#include <fstream>
#include <functional>
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
  }
}

int main(int ac, char **av)
try
{
  boost::asio::io_service remote_log_io_service;
  fhg::log::configure (remote_log_io_service);
  fhg::log::Logger::ptr_t logger (fhg::log::Logger::get());

  namespace po = boost::program_options;

  po::options_description desc("options");

  std::vector<std::string> config_vars;
  std::string kernel_name;

  desc.add_options()
    ("help,h", "this message")
    ("name,n", po::value<std::string>(&kernel_name), "give the kernel a name")
    ("set,s", po::value<std::vector<std::string>>(&config_vars), "set a parameter to a value key=value")
    ( "startup-messages-pipe"
    , po::value<int>()->required()
    , "pipe filedescriptor to use for communication during startup (ports used, ...)"
    )
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
    ;

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
    LLOG (ERROR, logger, "use " << av[0] << " --help to get a list of options");
    return EXIT_FAILURE;
  }
  po::notify (vm);

  if (vm.count("help"))
  {
    std::cout << av[0] << " [options]" << std::endl;
    std::cout << std::endl;
    std::cout << desc << std::endl;
    return EXIT_SUCCESS;
  }

  std::map<std::string, std::string> config_variables;
  for (const std::string& p : config_vars)
  {
    const std::pair<std::string, std::string> kv (fhg::util::split_string (p, '='));
    if (kv.first.empty())
    {
      LLOG (ERROR, logger, "invalid config variable: must not be empty");
      throw std::runtime_error ("invalid config variable: must not be empty");
    }

    config_variables.insert (kv);
  }
  config_variables["kernel_name"] = kernel_name;

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
  if (virtual_memory_api)
  {
    virtual_memory_api->start();
  }
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

  const std::list<std::string> masters
    ( fhg::util::split<std::string, std::string>
        (config_variables.at ("plugin.drts.master"), ',')
    );

  if (masters.empty())
  {
    throw std::runtime_error ("no masters specified");
  }

  const std::list<std::string> capabilities
    ( fhg::util::split<std::string, std::string>
        (config_variables.at ("plugin.drts.capabilities"), ',')
    );

  std::vector<DRTSImpl::master_info> master_info;
  std::set<std::string> seen_master_names;
  for (std::string const& master : masters)
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

  boost::optional<std::size_t> const socket
    ( config_variables.count ("plugin.drts.socket")
    ? boost::lexical_cast<std::size_t> (config_variables.at ("plugin.drts.socket"))
    : boost::optional<std::size_t>()
    );

  boost::asio::io_service peer_io_service;
  if (config_variables.count ("plugin.drts.gui_url"))
  {
    boost::asio::io_service gui_io_service;
    DRTSImpl const plugin
      ( request_stop
      , peer_io_service
      , std::pair<std::string, boost::asio::io_service&>
        (config_variables.at ("plugin.drts.gui_url"), gui_io_service)
      , config_variables
      , kernel_name
      , virtual_memory_api.get()
      , shared_memory.get()
      , master_info
      , capabilities
      , socket
      );

    {
      boost::iostreams::stream<boost::iostreams::file_descriptor_sink>
        startup_messages_pipe ( vm.at ("startup-messages-pipe").as<int>()
                              , boost::iostreams::close_handle
                              );
      startup_messages_pipe << "OKAY\n";
    }

    stop_requested.wait();
  }
  else
  {
    DRTSImpl const plugin ( request_stop
                          , peer_io_service
                          , boost::none
                          , config_variables
                          , kernel_name
                          , virtual_memory_api.get()
                          , shared_memory.get()
                          , master_info
                          , capabilities
                          , socket
                          );
    {
      boost::iostreams::stream<boost::iostreams::file_descriptor_sink>
        startup_messages_pipe ( vm.at ("startup-messages-pipe").as<int>()
                              , boost::iostreams::close_handle
                              );
      startup_messages_pipe << "OKAY\n";
    }

    stop_requested.wait();
  }

  return 0;
}
catch (...)
{
  fhg::util::print_current_exception (std::cerr, "EX: ");
  return 1;
}
