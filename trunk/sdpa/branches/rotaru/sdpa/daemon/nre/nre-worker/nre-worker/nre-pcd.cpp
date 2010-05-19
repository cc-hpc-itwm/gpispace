#include <sdpa/daemon/nre/nre-worker/nre-worker/nre-pcd.hpp>
#include <sdpa/daemon/nre/messages.hpp>

int main(int ac, char **av)
{
  namespace po = boost::program_options;

  po::options_description opts("Available Options");
  // fill in defaults
  opts.add_options()
    ("help,h", "show this help text")
    ("location,l", po::value<std::string>()->default_value("127.0.0.1:8000"), "where to listen")
    ("config,c" , po::value<std::string>()->default_value(NRE_PCD_DEFAULT_CFG), "input parameter to the activity")
    ("load" , po::value<std::vector<std::string> >(), "shared modules that shall be loaded")
    ("search-path,p", po::value<std::string>()->default_value(""), "search path for the modules")
    ("verbose,v", "verbose output")
    ("keep-going,k", "keep going, even if the FVM is not there")
  ;

  po::positional_options_description positional; // positional parameters used for command line parsing
  positional.add("load", -1);

  po::variables_map vm;
  try
  {
    po::store(po::command_line_parser(ac, av).options(opts)
                                             .positional(positional)
                                             .run()
            , vm);
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
    std::cerr << "usage: nre-pcd [options] module...." << std::endl;
    std::cerr << opts << std::endl;
    return 1;
  }

  fhg::log::Configurator::configure();

  // connect to FVM
  // initialize it with some default values
  fvm_pc_config_t pc_cfg ("/tmp/msq", "/tmp/shmem", 52428800, 52428800);

  // read those from the config file!
  /*try
  {
    read_fvm_config(vm["config"].as<std::string>(), pc_cfg);
  }
  catch (const std::exception &ex)
  {
    std::cerr << "E: could not read config file: " << ex.what() << std::endl;
    if (vm.count("keep-going"))
    {
      std::cerr << "**** ignoring this error (keep going=true)" << std::endl;
    }
    else
    {
      return 2;
    }
  }*/

  fvm_pc_connection_mgr fvm_pc;
  try
  {
    fvm_pc.init(pc_cfg);  
  } catch (const std::exception &ex)
  {
    std::cerr << "E: could not connect to FVM: " << ex.what() << std::endl;
    if (vm.count("keep-going"))
    {
      std::cerr << "**** ignoring this error (keep going=true)" << std::endl;
    }
    else
    {
      return 2;
    }
  }

  signal(SIGSEGV, &sig_handler);
  signal(SIGABRT, &sig_handler);

  using namespace we::loader;

  LOG(INFO, "starting on location: " << vm["location"].as<std::string>() << "...");
  sdpa::shared_ptr<sdpa::nre::worker::ActivityExecutor> executor(new sdpa::nre::worker::ActivityExecutor(vm["location"].as<std::string>(), fvmGetRank()));
  executor->loader().append_search_path( vm["search-path"].as<std::string>() );

  try
  {
    typedef std::vector<std::string> mod_list;
	mod_list mods;
	if (getenv("PCD_MODULES") != NULL)
	{
	  // first fill the list with those modules specified in PCD_MODULES
	  typedef boost::tokenizer<boost::char_separator<char> > tokenizer;
	  boost::char_separator<char> sep(":");
	  std::string PCD_MODULES(getenv("PCD_MODULES"));
	  tokenizer tokens(PCD_MODULES, sep);
	  std::copy(tokens.begin(), tokens.end(), std::back_inserter(mods));
	}

    if (vm.count("load"))
    {
      const mod_list &cmdline_mods(vm["load"].as<std::vector<std::string> >());
	  std::copy(cmdline_mods.begin(), cmdline_mods.end(), std::back_inserter(mods));
    }

	for (mod_list::const_iterator mod(mods.begin()); mod != mods.end(); ++mod)
	{
	  executor->loader().load(*mod);
	}
  }
  catch (const ModuleLoadFailed &mlf)
  {
    std::cerr << "could not load module: " << mlf.what() << std::endl;
    return 3;
  }
  catch (const std::exception &ex)
  {
    std::cerr << "could not load module: " << ex.what() << std::endl;
    return 3;
  }

  try
  {
    executor->start();
  }
  catch (const std::exception &ex)
  {
    std::cerr << "could not start executor: " << ex.what() << std::endl;
    return 4;
  }

  LOG(DEBUG, "waiting for signals...");
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
      LOG(DEBUG, "got signal: " << sig);
      switch (sig)
      {
        case SIGTERM:
        case SIGHUP:
        case SIGINT:
          signal_ignored = false;
          break;
        default:
          LOG(INFO, "ignoring signal: " << sig);
          break;
      }
    }
    else
    {
      LOG(ERROR, "error while waiting for signal: " << result);
    }
  }
  fvm_pc.leave();
  LOG(INFO, "terminating...");
  if (! executor->stop())
  {
    LOG(WARN, "executor did not stop correctly...");
  }

  return 0;
}
