// bernd.loerwald@itwm.fraunhofer.de

#include <fhg/plugin/core/kernel.hpp> // search_path_t
#include <fhg/plugin/core/license.hpp>
#include <fhg/util/daemonize.hpp>
#include <fhg/util/signal_handler_manager.hpp>

#include <fhglog/LogMacros.hpp>

#include <boost/program_options.hpp>

#include <string>
#include <vector>

int main(int ac, char **av)
{
  FHGLOG_SETUP();
  fhg::log::Logger::ptr_t logger (fhg::log::Logger::get());

  namespace po = boost::program_options;

  po::options_description desc("options");

  fhg::core::kernel_t::search_path_t search_path;

  std::string netd_url;

  desc.add_options()
    ("help,h", "this message")
    ("netd_url", po::value<std::string>(&netd_url), "url to listen on")
    ( "add-search-path,L", po::value<fhg::core::kernel_t::search_path_t>(&search_path)
    , "add a path to the search path for plugins"
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

  fhg::plugin::magically_check_license (logger);

  fhg::util::fork_and_daemonize_child_and_abandon_parent();

  fhg::core::kernel_t kernel ("", "gspc-rif", search_path);

  kernel.put ("plugin.rif.url", netd_url);

  const int ec (kernel.load_plugin ("rif"));
  if (ec != 0)
  {
    throw std::runtime_error (strerror (ec));
  }

  fhg::util::signal_handler_manager signal_handlers;

  signal_handlers.add_log_backtrace_and_exit_for_critical_errors (logger);

  signal_handlers.add (SIGTERM, boost::bind (&fhg::core::kernel_t::stop, &kernel));
  signal_handlers.add (SIGINT, boost::bind (&fhg::core::kernel_t::stop, &kernel));

  return kernel.run_and_unload (false);
}
