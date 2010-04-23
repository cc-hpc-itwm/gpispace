#include <fhglog/fhglog.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <cstring>
#include <csignal>
#include <boost/program_options.hpp>
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string/predicate.hpp>

// fvm dependencies
#include <fvm-pc/pc.hpp>

#if ! defined(NRE_PCD_DEFAULT_CFG)
#   define NRE_PCD_DEFAULT_CFG "/etc/fvm/fvm.rc"
#endif

#include "ActivityExecutor.hpp"

namespace
{
struct fvm_pc_connection_mgr
{
  fvm_pc_connection_mgr()
    : do_leave_(false)
  {

  }

  explicit
  fvm_pc_connection_mgr(const fvm_pc_config_t &cfg)
    : do_leave_(false)
  {
    init(cfg);
  }
  
  void init(const fvm_pc_config_t &cfg)
  {
    LOG(INFO, "connecting to FVM via: " << cfg.msqfile);
    int err;
    if ((err = fvmConnect(cfg)) < 0)
    {
      LOG(FATAL, "Could not open connection to FVM: " << err);
      throw std::runtime_error("Could not open connection to the FVM!");
    }
    LOG(INFO, "successfully connected to FVM!");
    do_leave_ = true;
  }

  void leave()
  {
    if (do_leave_)
    {
      LOG(INFO, "disconnecting from FVM...");
      fvmLeave();
      do_leave_ = false;
    }
  }

  ~fvm_pc_connection_mgr() throw()
  {
    try
    {
      leave();
    }
    catch (...) { }
  }

  bool do_leave_;
};
}

bool verbose(false);
  
void cleanUp()
{
  fvmLeave();
}

void sig_handler(int sig)
{
  std::cout << "Lethal signal (" << sig << ") received" << " - I will do my best to save the world!" << std::endl;
  cleanUp();
  exit(666);
}

const std::size_t MAX_PATH_LEN = 1024;
int read_fvm_config(const std::string &path, fvm_pc_config_t &cfg) throw(std::exception)
{
  LOG(DEBUG, "reading fvm-config from file: " << path);
  std::ifstream ifs(path.c_str());
  if (! ifs)
  {
    throw std::runtime_error("could not open fvm-config file " + path + " for reading!");
  }

  cfg.shmemsize = 0;
  cfg.fvmsize = 0;

  while (ifs)
  {
    std::string line;
    std::getline(ifs, line);
    if (line.empty()) continue;
    if (line[0] == '#') continue;

    DLOG(DEBUG, "parsing line: \"" << line << "\"");

    std::string::size_type split_pos(line.find_first_of(" "));
    const std::string param_name(line.substr(0, split_pos));
    const std::string param_value(line.substr(split_pos+1));
    if (param_name.empty() || param_value.empty())
    {
      LOG(WARN, "ignoring invalid line: " << line);      
    }
    else
    {
      if (param_name == "SHMSZ")
      {
        std::istringstream istr(param_value);
        istr >> cfg.shmemsize;
        if (! istr)
        {
          LOG(FATAL, "could not parse " << param_name << " into std::size_t");
          throw std::runtime_error("parser error in line " + line + ": could not parse std::size_t value!");
        }
        DLOG(DEBUG, "set shmemsize to \"" << cfg.shmemsize << "\"");
      }
      else if (param_name == "FVMSZ")
      {
        std::istringstream istr(param_value);
        istr >> cfg.fvmsize;
        if (! istr)
        {
          LOG(FATAL, "could not parse " << param_name << " into std::size_t");
          throw std::runtime_error("parser error in line " + line + ": could not parse std::size_t value!");
        }
        DLOG(DEBUG, "set fvmsize to \"" << cfg.fvmsize << "\"");
      }
      else if (param_name == "MSQFILE")
      {
        DLOG(DEBUG, "setting msqfile to \"" << param_value << "\"");
        strncpy(cfg.msqfile, param_value.c_str(), sizeof(cfg.msqfile));
      }
      else if (param_name == "SHMFILE")
      {
        DLOG(DEBUG, "setting shmemfile to \"" << param_value << "\"");
        strncpy(cfg.shmemfile, param_value.c_str(), sizeof(cfg.shmemfile));
      }
      else
      {
        LOG(WARN, "ignoring invalid parameter: " << param_name);
      }
    }
  }
  if (!cfg.shmemsize || !cfg.fvmsize)
  {
    throw std::runtime_error("config file did not set reasonable values for SHMSZ and/or FVMSZ!");
  }
  return 0;
}

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
  fvm_pc_config_t pc_cfg;

  // read those from the config file!
  try
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
  }

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

  using namespace sdpa::modules;

  std::clog << "I: starting on location: " << vm["location"].as<std::string>() << "..." << std::endl;
  sdpa::shared_ptr<sdpa::nre::worker::ActivityExecutor> executor(new sdpa::nre::worker::ActivityExecutor(vm["location"].as<std::string>()));

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
