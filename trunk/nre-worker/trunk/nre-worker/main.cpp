#include <fhglog/fhglog.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <cstring>

// sdpa dependencies
#include <sdpa/wf/Activity.hpp>
#include <sdpa/modules/ModuleLoader.hpp>

// fvm dependencies
#include <fvm-pc/pc.hpp>

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
    do_leave_ = true;
  }

  ~fvm_pc_connection_mgr()
  {
    if (do_leave_)
    {
      LOG(INFO, "disconnecting from FVM...");
      fvmLeave();
    }
  }

  bool do_leave_;
};
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

    DLOG(DEBUG, "parsing line: " << line);

    std::string::size_type split_pos(line.find_first_of(" "));
    const std::string param_name(line.substr(0, split_pos));
    const std::string param_value(line.substr(split_pos+1));
    if (param_name.empty() || param_value.empty())
    {
      LOG(WARN, "ignoring invalid line: " << line);      
    }
    else
    {
      DLOG(DEBUG, "name=" << param_name << " value=" << param_value);
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
  if (ac < 2)
  {
    std::cerr << "usage: " << av[0] << " path-to-fvm-config [modules...]" << std::endl;
    return 1;
  }
  fhg::log::Configurator::configure();

  // connect to FVM
  fvm_pc_config_t pc_cfg;

  // read those from the config file!
  read_fvm_config(av[1], pc_cfg);

  fvm_pc_connection_mgr fvm_pc;
  
  try
  {
    fvm_pc.init(pc_cfg);  
  } catch (const std::exception &ex)
  {
    LOG(FATAL, "fvm-connection failed: " << ex.what());
  }

  using namespace sdpa::modules;
  // create the module loader
  ModuleLoader::ptr_t loader(ModuleLoader::create());

  for (int i = 2; i < ac; ++i)
  {
    loader->load(av[i]);
  }

  sdpa::nre::worker::ActivityExecutor executor(loader, "127.0.0.1:8000");

  // Activity act;
  // while (true)
  // {
  //  in >> act;
  //  execute(act);
  //  out << act;
  // }

  executor.loop();
}
