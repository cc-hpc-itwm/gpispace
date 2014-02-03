// bernd.loerwald@itwm.fraunhofer.de

#include <fhg/plugin/core/kernel.hpp>
#include <fhg/plugin/core/license.hpp>
#include <fhg/util/daemonize.hpp>
#include <fhg/util/pidfile_writer.hpp>
#include <fhg/util/signal_handler_manager.hpp>
#include <fhg/util/split.hpp>

#include <fhglog/LogMacros.hpp>

#include <boost/foreach.hpp>
#include <boost/program_options.hpp>

#include <string>
#include <vector>

int main(int ac, char **av)
{
  FHGLOG_SETUP();
  fhg::log::Logger::ptr_t logger (fhg::log::Logger::get());

  namespace po = boost::program_options;

  po::options_description desc("options");

  std::vector<std::string> config_vars;
  std::string pidfile;
  std::string kernel_name;
  fhg::core::kernel_t::search_path_t search_path;

  desc.add_options()
    ("help,h", "this message")
    ("name,n", po::value<std::string>(&kernel_name), "give the kernel a name")
    ("set,s", po::value<std::vector<std::string> >(&config_vars), "set a parameter to a value key=value")
    ("pidfile", po::value<std::string>(&pidfile)->default_value(pidfile), "write pid to pidfile")
    ("daemonize", "daemonize after all checks were successful")
    ("gpi_enabled", "load gpi api")
    ( "keep-going,k", "just log errors, but do not refuse to start")
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

  std::vector<std::string> mods_to_load;
  mods_to_load.push_back ("drts");

  if (vm.count ("gpi_enabled"))
  {
    mods_to_load.push_back ("gpi");
    mods_to_load.push_back ("gpi_compat");
  }

  const bool daemonize (vm.count ("daemonize"));
  const bool keep_going (vm.count ("keep-going"));

  fhg::plugin::magically_check_license (logger);

  if (not pidfile.empty())
  {
    fhg::util::pidfile_writer const pidfile_writer (pidfile);

    if (daemonize)
    {
      fhg::util::fork_and_daemonize_child_and_abandon_parent();
    }

    pidfile_writer.write();
  }
  else
  {
    if (daemonize)
    {
      fhg::util::fork_and_daemonize_child_and_abandon_parent();
    }
  }

  fhg::core::kernel_t kernel (kernel_name, search_path);

  BOOST_FOREACH (std::string const & p, config_vars)
  {
    typedef std::pair<std::string,std::string> key_val_t;
    key_val_t kv (fhg::util::split_string(p, "="));
    if (kv.first.empty())
    {
      LLOG (WARN, logger, "invalid config variable: must not be empty");
    }
    else
    {
      DLLOG (TRACE, logger, "setting " << kv.first << " to " << kv.second);
      kernel.put(kv.first, kv.second);
    }
  }

  BOOST_FOREACH (std::string const & p, mods_to_load)
  {
    try
    {
      int ec = kernel.load_plugin (p);
      if (ec != 0)
        throw std::runtime_error (strerror (ec));
    }
    catch (std::exception const &ex)
    {
      LLOG (ERROR, logger, "could not load `" << p << "' : " << ex.what());
      if (! keep_going)
      {
        throw;
      }
    }
  }

  fhg::util::signal_handler_manager signal_handlers;

  signal_handlers.add_log_backtrace_and_exit_for_critical_errors (logger);

  signal_handlers.add (SIGTERM, boost::bind (&fhg::core::kernel_t::stop, &kernel));
  signal_handlers.add (SIGINT, boost::bind (&fhg::core::kernel_t::stop, &kernel));

  return kernel.run_and_unload (false);
}
