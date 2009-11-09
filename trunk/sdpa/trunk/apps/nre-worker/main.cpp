#include <fhglog/fhglog.hpp>
#include <sdpa/modules/ModuleLoader.hpp>
#include <fvm-pc/pc.hpp>

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
    if (do_leave_) fvmLeave();
  }

  bool do_leave_;
};
}

int main(int ac, char **av)
{
  fhg::log::Configurator::configure();
  // connect to FVM
  fvm_pc_config_t pc_cfg;

  // read those from the config file!
  pc_cfg.shmemsize = 1073741824;
  pc_cfg.fvmsize   = 1073741824;
  pc_cfg.msqfile   = "/tmp/nre-worker/keyfile";
  pc_cfg.shmemfile = "/tmp/nre-worker/shmemkey";

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
}
