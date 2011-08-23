#include <signal.h>

#include <vector>
#include <string>
#include <iostream>

#include <boost/foreach.hpp>
#include <boost/program_options.hpp>

#include <fhglog/minimal.hpp>
#include <fhg/util/split.hpp>
#include <fhg/plugin/plugin.hpp>
#include <fhg/plugin/core/kernel.hpp>

static fhg::core::kernel_t *kernel = 0;

static void at_exit ()
{
  if (kernel) kernel->stop();
}

static void sig_term(int)
{
  if (kernel) kernel->stop ();
}

int main(int ac, char **av)
{
  FHGLOG_SETUP(ac,av);

  kernel = new fhg::core::kernel_t;

  signal (SIGINT, sig_term);
  signal (SIGTERM, sig_term);

  namespace po = boost::program_options;

  po::options_description desc("options");

  std::vector<std::string> mods_to_load;
  std::vector<std::string> config_vars;
  bool keep_going (false);

  desc.add_options()
    ("help,h", "this message")
    ("verbose,v", "be verbose")
    ("set,s", po::value<std::vector<std::string> >(&config_vars), "set a parameter to a value key=value")
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

  BOOST_FOREACH (std::string const & p, config_vars)
  {
    typedef std::pair<std::string,std::string> key_val_t;
    key_val_t kv (fhg::util::split_string(p, "="));
    if (kv.first.empty())
    {
      MLOG(WARN, "invalid config variable: must not be empty");
    }
    else
    {
      MLOG(TRACE, "setting " << kv.first << " to " << kv.second);
      kernel->put(kv.first, kv.second);
    }
  }

  BOOST_FOREACH (std::string const & p, mods_to_load)
  {
    try
    {
      kernel->load_plugin (p);
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

  atexit(&at_exit);
  int rc = kernel->run();
  MLOG(INFO, "shutting down... (" << rc << ")");
  kernel->unload_all();

  delete kernel;
  kernel = 0;

  return rc;
}
