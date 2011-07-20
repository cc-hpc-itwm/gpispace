#include <signal.h>

#include <vector>
#include <string>
#include <iostream>

#include <boost/foreach.hpp>
#include <boost/program_options.hpp>

#include <fhglog/minimal.hpp>
#include <fhg/plugin/core/kernel.hpp>

static fhg::core::kernel_t kernel;


static void sig_term(int)
{
  kernel.stop ();
}

int main(int ac, char **av)
{
  FHGLOG_SETUP(ac,av);

  signal (SIGINT, sig_term);
  signal (SIGTERM, sig_term);

  namespace po = boost::program_options;

  po::options_description desc("options");

  std::vector<std::string> mods_to_load;
  bool keep_going (false);

  desc.add_options()
    ("help,h", "this message")
    ("verbose,v", "be verbose")
    ( "keep-going,k", "just log errors, but do not refuse to start")
    ( "load,l"
    , po::value<std::vector<std::string> >(&mods_to_load)
    , "modules to load"
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
    std::cerr << "invalid command line: " << ex.what() << std::endl;
    std::cerr << "use " << av[0] << " --help to get a list of options" << std::endl;
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

  keep_going = vm.count("keep-going") != 0;

  BOOST_FOREACH (std::string const & p, mods_to_load)
  {
    try
    {
      kernel.load_plugin (p);
    }
    catch (std::exception const &ex)
    {
      MLOG(ERROR, "could not load `" << p << "' : " << ex.what());
      if (! keep_going)
      {
        return EXIT_FAILURE;
      }
    }
  }

  int rc = kernel.run();
  kernel.unload_all();
  return rc;
}
