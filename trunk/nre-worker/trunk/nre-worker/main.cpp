#include <fhglog/fhglog.hpp>

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
int read_fvm_config(const std::string &path, fvm_pc_config_t &cfg)
{
  LOG(DEBUG, "reading fvm-config from file: " << path);
  strncpy(cfg.msqfile, "/tmp/fvm_pc_msq", MAX_PATH_LEN);
  strncpy(cfg.shmemfile, "/tmp/fvm_pc_key", MAX_PATH_LEN);
  return 0;
}

int main(int ac, char **av)
{
  fhg::log::Configurator::configure();
  // connect to FVM
  fvm_pc_config_t pc_cfg;

  // read those from the config file!
  pc_cfg.shmemsize = 1073741824;
  pc_cfg.fvmsize   = 1073741824;
  read_fvm_config("/tmp/nre-worker/fvmconfig", pc_cfg);

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

  for (int i = 1; i < ac; ++i)
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
