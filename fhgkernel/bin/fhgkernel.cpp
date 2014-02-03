// alexander.petry@itwm.fraunhofer.de

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

  std::vector<std::string> mods_to_load;
  std::vector<std::string> config_vars;
  std::string pidfile;
  std::string kernel_name ("fhgkernel");
  fhg::core::kernel_t::search_path_t search_path;

  desc.add_options()
    ("help,h", "this message")
    ("name,n", po::value<std::string>(&kernel_name), "give the kernel a name")
    ("set,s", po::value<std::vector<std::string> >(&config_vars), "set a parameter to a value key=value")
    ("pidfile", po::value<std::string>(&pidfile)->default_value(pidfile), "write pid to pidfile")
    ("daemonize", "daemonize after all checks were successful")
    ( "keep-going,k", "just log errors, but do not refuse to start")
    ( "load,l"
    , po::value<std::vector<std::string> >(&mods_to_load)
    , "modules to load"
    )
    ( "add-search-path,L", po::value<fhg::core::kernel_t::search_path_t>(&search_path)
    , "add a path to the search path for plugins"
    )
    ;

  po::positional_options_description p;
  p.add("load", -1);

  po::variables_map vm;
  try
  {
    po::store( po::command_line_parser(ac, av)
             . options(desc).positional(p).run()
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

  return setup_and_run_fhgkernel ( vm.count ("daemonize")
                                 , vm.count ("keep-going")
                                 , mods_to_load
                                 , config_vars
                                 , pidfile
                                 , kernel_name
                                 , search_path
                                 , logger
                                 );
}
