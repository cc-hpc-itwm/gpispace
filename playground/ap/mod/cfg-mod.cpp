#include <sdpa/modules/Macros.hpp>
#include <iostream>
#include <string>
#include <cstdlib> // malloc, free
#include <cstring>
#include <fhglog/fhglog.hpp>
#include <fvm-pc/pc.hpp>
#include <sdpa/modules/util.hpp>

#include "cfg.hpp"

using namespace sdpa::modules;

void read_config(data_t &param) throw (std::exception)
{
  const std::string &config_file = param.at("config_file").token().data();

  int num_nodes = fvmGetNodeCount();
  std::cout << "running on " << num_nodes << " nodes" << std::endl;

  cfg_t *configs = new cfg_t[num_nodes];

  // call the C function
  c_read_config(config_file, configs);

  // write configs to global memory
  fvmAllocHandle_t global_cfg = fvmGlobalAlloc(sizeof(cfg_t));
  fvm::util::distribute_data(configs, global_cfg);
  delete [] configs;

  // store output token
  param["cfg"].token().data(global_cfg);
}

SDPA_MOD_INIT_START(cfg-test)
{
  SDPA_REGISTER_FUN_START(read_config);
	SDPA_ADD_INP("config_file", char * );
	SDPA_ADD_OUT("cfg", fvmAllocHandle_t );
  SDPA_REGISTER_FUN_END(read_config);
}
SDPA_MOD_INIT_END(cfg-test)
