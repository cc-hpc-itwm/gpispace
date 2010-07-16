#include <Module.hpp>
#include <iostream>
#include <string>
#include <cstdlib> // malloc, free
#include <cstring>
#include <pc.h>


using namespace sdpa::modules;

typedef struct
{
  int i;
  char data[1020];
} cfg_t;

void c_read_config(cfg_t *cfgs)
{
  cfgs[0].i = 42;
}

static void test_distribute(Module::data_t &params) throw (std::exception)
{
  std::cout << "running RunTest example" << std::endl;
  int num_nodes = fvmGetNodeCount();
  std::cout << "running on " << num_nodes << " nodes" << std::endl;

  cfg_t *configs = new cfg_t[num_nodes];
  configs[0].i = 0;
  c_read_config(configs);
  std::cout << "reading config finished: " << configs[0].i << std::endl;

  // write configs to global memory
  fvmAllocHandle_t global_cfg = fvmGlobalAlloc(sizeof(cfg_t));
  fvmAllocHandle_t scratch = fvmLocalAlloc(sizeof(cfg_t));
  for (std::size_t i=0; i < num_nodes; ++i)
  {
     // copy to shared mem
     std::cout << "copying " << sizeof(cfg_t) << " bytes config " << &configs[i] << " of node " << i << " to " << fvmGetShmemPtr() << std::endl;
     memcpy(fvmGetShmemPtr(), &configs[i], sizeof(cfg_t));

     // distribute the config to the node
     std::cout << "distributing config to global memory" << std::endl;
     fvmCommHandle_t comm_hdl = fvmPutGlobalData(global_cfg, i*sizeof(cfg_t), sizeof(cfg_t), 0, scratch);

     std::cout << "waiting for finish..." << std::endl;
     fvmCommHandleState_t state = waitComm(comm_hdl);
     std::cout << "comm state = " << state << std::endl;
  }
  fvmLocalFree(scratch);
  delete [] configs;
}

extern "C" {
  void sdpa_mod_init(Module *mod) {
    SDPA_REGISTER_FUN(mod, test_distribute);
 
    //actually call the functions
    sdpa::modules::Module::data_t params;
    test_distribute(params);

  }
}
