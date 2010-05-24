#ifndef NRE_NRE_WORKER_NRE_PCD_HPP
#define NRE_NRE_WORKER_NRE_PCD_HPP 1

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

#include <sdpa/daemon/nre/nre-worker/nre-worker/ActivityExecutor.hpp>

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

#endif
