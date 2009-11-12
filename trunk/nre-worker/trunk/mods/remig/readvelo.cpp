#include <sdpa/modules/Module.hpp>

#include <fhglog/fhglog.hpp>

#include <fvm-pc/pc.hpp>

using namespace sdpa::modules;

void readvelo (Module::data_t &params)
{
  const fvmAllocHandle_t memhandle_for_configuration
    (params["memhandle_for_configuration"].token().data_as<fvmAllocHandle_t>());

  MLOG (DEBUG, "memhandle_for_configuration = " << memhandle_for_configuration);

  // at the moment: do nothing
}

SDPA_MOD_INIT_START(readvelo)
{
  SDPA_REGISTER_FUN(readvelo);
}
SDPA_MOD_INIT_END(readvelo)
