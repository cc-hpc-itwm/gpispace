#include <fhglog/fhglog.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <cstring>
#include <csignal>

#include <boost/program_options.hpp>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string/predicate.hpp>

#include <we/loader/loader.hpp>

static bool verbose (false);

static void cleanUp()
{
}

static void sig_handler(int sig)
{
  std::clog << "Lethal signal (" << sig << ") received" << " - I will do my best to save the world!" << std::endl;
  cleanUp();
  exit(666);
}

int main(int ac, char **av)
{
  // TODO: should go to the fhglog module
  fhg::log::Configurator::configure();

  typedef std::vector<std::string> mod_list;
  namespace po = boost::program_options;

  po::options_description opts("Available Options");
  // fill in defaults
  opts.add_options()
    ("help,h", "show this help text")
    ("append-search-path,p",  po::value<std::vector<std::string> >(), "append to module search path")
    ("prepend-search-path,a", po::value<std::vector<std::string> >(), "prepend to module search path")
    ("load,l" , po::value<std::vector<std::string> >(), "shared modules that shall be loaded")
    ("selftest,T", "run selftest of loaded modules")
    ("verbose,v", "verbose output")
    ;

  po::positional_options_description positional; // positional parameters used for command line parsing
  positional.add("load", -1);

  po::variables_map vm;
  try
  {
    po::store(po::command_line_parser(ac, av).options(opts).positional(positional).run(), vm);
  }
  catch (const std::exception &ex)
  {
    std::cerr << "E: could not parse command line: " << ex.what() << std::endl;
    std::cerr << opts << std::endl;
    return 2;
  }
  verbose = (vm.count("verbose") > 0);

  if (vm.count("help"))
  {
    std::cerr << "usage: loader [options] module...." << std::endl;
    std::cerr << opts << std::endl;
    return 1;
  }

  signal(SIGSEGV, &sig_handler);
  signal(SIGABRT, &sig_handler);

  we::loader::loader loader;

  /*
   * TODO:
   *   The actual steps should be like this
   *       - create the module loader
   *       - load modules specified on command line
   *       - wait for signals
   *
   *   Modules to load:
   *       - modules get a pointer to the loader to be able to load additional things
   *       - fhglog
   *       - fvm
   *       - activity executor
   *         - keeps the pointer to the module loader
   *         - create activity executor object
   *         -
   */

  if (vm.count ("prepend-search-path"))
  {
    const std::vector<std::string>& search_path= vm["prepend-search-path"].as<std::vector<std::string> >();

    for (std::vector<std::string>::const_iterator p(search_path.begin()); p != search_path.end(); ++p)
      loader.prepend_search_path( *p );
  }

  if (vm.count ("append-search-path"))
  {
    const std::vector<std::string>& search_path= vm["append-search-path"].as<std::vector<std::string> >();

    for (std::vector<std::string>::const_iterator p(search_path.begin()); p != search_path.end(); ++p)
      loader.append_search_path( *p );
  }

  if (vm.count("load"))
  {
    try
    {
      const mod_list& cmdline_mods = vm["load"].as<std::vector<std::string> >();
      for (mod_list::const_iterator mod(cmdline_mods.begin()); mod != cmdline_mods.end(); ++mod)
        loader.load(*mod);
    }
    catch (const we::loader::ModuleLoadFailed &mlf)
    {
      std::cerr << "could not load module: " << mlf.what() << std::endl;
      return 3;
    }
    catch (const std::exception &ex)
    {
      std::cerr << "could not load module: " << ex.what() << std::endl;
      return 3;
    }
  }

  LOG(INFO, "module loader initialized and waiting for signals...");
  LOG(DEBUG, loader);

  if (vm.count("selftest"))
  {
    return loader.selftest();
  }

  sigset_t waitset;
  int sig(0);
  int result(0);

  sigfillset(&waitset);
  sigprocmask(SIG_BLOCK, &waitset, NULL);

  bool signal_ignored = true;
  while (signal_ignored)
  {
    result = sigwait(&waitset, &sig);
    if (result == 0)
    {
      DLOG(DEBUG, "got signal: " << sig);
      switch (sig)
      {
      case SIGTERM:
      case SIGHUP:
      case SIGINT:
        signal_ignored = false;
        break;
      default:
        DLOG(INFO, "ignoring signal: " << sig);
        break;
      }
    }
    else
    {
      LOG(ERROR, "error while waiting for signal: " << result);
      break;
    }
  }

  LOG(INFO, "terminating...");

  return 0;
}
