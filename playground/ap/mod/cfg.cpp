#include <fstream>
#include <fhglog/fhglog.hpp>
#include <fvm-pc/pc.hpp>
#include "cfg.hpp"

int c_read_config(const std::string &config_file, cfg_t *cfgs)
{
  std::ifstream ifs(config_file.c_str());
  if (! ifs)
  {
    LOG(ERROR, "could not open config file: " << config_file);
    // ignore the error for now
  }

  fvmAllocHandle_t velocity_field = fvmGlobalAlloc(2 << 16);
  fvmAllocHandle_t output_volume = fvmGlobalAlloc(2 << 16);
  for (size_t node = 0; node < fvmGetNodeCount(); ++node)
  {
    LOG(DEBUG, "initializing config for node " << node);
    cfgs[node].velocity_field = velocity_field;
    cfgs[node].output_volume = output_volume;
  }
  return 0;
}
