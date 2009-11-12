#include <sdpa/modules/Module.hpp>

#include <fhglog/fhglog.hpp>

#include <fvm-pc/pc.hpp>

using namespace sdpa::modules;

void init (Module::data_t &params)
{
  const std::string config_file (params["config_file"].token().data());

  // read config file, parse, setup everything

  MLOG(DEBUG, "config_file = " << config_file);

  const unsigned long number_of_frequencies (10);
  const unsigned long number_of_depthlevels (5);
  const unsigned long number_of_parallel_propagators (3);
  const fvmAllocHandle_t memhandle_for_outputvolume (fvmGlobalAlloc(1<<20));
  const fvmAllocHandle_t memhandle_for_configuration (fvmGlobalAlloc(1<<10));

  // for now, just free the mem immediately

  fvmGlobalFree (memhandle_for_outputvolume);
  fvmGlobalFree (memhandle_for_configuration);

  params["number_of_frequencies"].token().data(number_of_frequencies);
  params["number_of_depthlevels"].token().data(number_of_depthlevels);
  params["number_of_parallel_propagators"].token().data(number_of_parallel_propagators);
  params["memhandle_for_outputvolume"].token().data(memhandle_for_outputvolume);
  params["memhandle_for_configuration"].token().data(memhandle_for_configuration);
  params["seq"].token().data("SEQ");
}

SDPA_MOD_INIT_START(init)
{
  SDPA_REGISTER_FUN(init);

  Module::data_t zero;

  init (zero);
}
SDPA_MOD_INIT_END(init)
