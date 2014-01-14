// bernd.loerwald@itwm.fraunhofer.de

#include <fhg/plugin/core/kernel.hpp> // search_path_t
#include <fhg/plugin/setup_and_run_fhgkernel.hpp>

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

  std::vector<std::string> config_vars;
  std::string state_path;
  std::string pidfile;
  std::string kernel_name;
  fhg::core::kernel_t::search_path_t search_path;

  desc.add_options()
    ("help,h", "this message")
    ("verbose,v", "be verbose")
    ("name,n", po::value<std::string>(&kernel_name), "give the kernel a name")
    ("set,s", po::value<std::vector<std::string> >(&config_vars), "set a parameter to a value key=value")
    ("state,S", po::value<std::string>(&state_path), "state directory to use")
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
  mods_to_load.push_back ("numa");
  mods_to_load.push_back ("logc");
  mods_to_load.push_back ("kvs");
  mods_to_load.push_back ("progress");
  mods_to_load.push_back ("netd");
  mods_to_load.push_back ("wfe");
  mods_to_load.push_back ("drts");

  if (vm.count ("gpi_enabled"))
  {
    mods_to_load.push_back ("gpi");
    mods_to_load.push_back ("gpi_compat");
  }

  return setup_and_run_fhgkernel ( vm.count ("daemonize")
                                 , vm.count ("keep-going")
                                 , mods_to_load
                                 , config_vars
                                 , state_path
                                 , pidfile
                                 , kernel_name
                                 , search_path
                                 , logger
                                 );
}
