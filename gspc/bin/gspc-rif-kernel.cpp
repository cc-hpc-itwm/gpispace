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

  std::vector<std::string> mods_to_load;
  mods_to_load.push_back ("rif");
  mods_to_load.push_back ("netd");

  std::vector<std::string> config_vars;
  config_vars.push_back ("plugin.netd.url=" + netd_url);

  return setup_and_run_fhgkernel ( true // daemonize
                                 , false //keep-going
                                 , mods_to_load
                                 , config_vars
                                 , "" // state_path
                                 , "" // pid_file
                                 , "gspc-rif"
                                 , search_path
                                 , logger
                                 );
}
