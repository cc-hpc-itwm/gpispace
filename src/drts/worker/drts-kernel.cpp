// bernd.loerwald@itwm.fraunhofer.de

#include <drts/worker/drts.hpp>

#include <plugin/core/kernel.hpp>
#include <fhg/util/boost/program_options/validators/existing_path.hpp>
#include <fhg/util/signal_handler_manager.hpp>
#include <fhg/util/split.hpp>

#include <fhglog/LogMacros.hpp>

#include <boost/program_options.hpp>

#include <fstream>
#include <functional>
#include <string>
#include <vector>

int main(int ac, char **av)
{
  boost::asio::io_service remote_log_io_service;
  FHGLOG_SETUP (remote_log_io_service);
  fhg::log::Logger::ptr_t logger (fhg::log::Logger::get());

  namespace po = boost::program_options;

  po::options_description desc("options");

  std::vector<std::string> config_vars;
  std::string kernel_name;
  fhg::core::kernel_t::search_path_t search_path;

  desc.add_options()
    ("help,h", "this message")
    ("name,n", po::value<std::string>(&kernel_name), "give the kernel a name")
    ("set,s", po::value<std::vector<std::string>>(&config_vars), "set a parameter to a value key=value")
    ("gpi_enabled", "load gpi api")
    ( "add-search-path,L", po::value<fhg::core::kernel_t::search_path_t>(&search_path)
    , "add a path to the search path for plugins"
    )
    ( "startup-messages-fifo"
    , po::value<fhg::util::boost::program_options::existing_path>()->required()
    , "fifo to use for communication during startup (ports used, ...)"
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

  std::vector<std::string> mods_to_load;
  if (vm.count ("gpi_enabled"))
  {
    mods_to_load.push_back ("gpi");
    mods_to_load.push_back ("gpi_compat");
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

  fhg::core::wait_until_stopped waiter;
  const std::function<void()> request_stop (waiter.make_request_stop());

  fhg::core::kernel_t kernel (search_path, request_stop, config_variables);

  for (std::string const & p : mods_to_load)
  {
    kernel.load_plugin_by_name (p);
  }

  fhg::util::signal_handler_manager signal_handlers;

  signal_handlers.add_log_backtrace_and_exit_for_critical_errors (logger);

  signal_handlers.add (SIGTERM, std::bind (request_stop));
  signal_handlers.add (SIGINT, std::bind (request_stop));

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
      );

    {
      std::ofstream startup_messages_fifo
        ( vm["startup-messages-fifo"]
        . as<fhg::util::boost::program_options::existing_path>().string()
        );
      startup_messages_fifo << "OKAY\n";
    }

    waiter.wait();
  }
  else
  {
    DRTSImpl const plugin ( request_stop
                          , peer_io_service
                          , boost::none
                          , config_variables
                          );

    {
      std::ofstream startup_messages_fifo
        ( vm["startup-messages-fifo"]
        . as<fhg::util::boost::program_options::existing_path>().string()
        );
      startup_messages_fifo << "OKAY\n";
    }

    waiter.wait();
  }

  return 0;
}
