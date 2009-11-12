#include <sdpa/modules/Module.hpp>

#include <fhglog/fhglog.hpp>

#include <fvm-pc/pc.hpp>

using namespace sdpa::modules;

void finalize (Module::data_t &params)
{
  const fvmAllocHandle_t memhandle_for_configuration
    (params["memhandle_for_configuration"].token().data_as<fvmAllocHandle_t>());

  MLOG (DEBUG, "memhandle_for_configuration = " << memhandle_for_configuration);

  int ret (fvmGlobalFree (memhandle_for_configuration));

  MLOG (DEBUG, "fvmGlobalfree(memhandle_for_configuration) = " << ret);

  // at the moment: do nothing
}

SDPA_MOD_INIT_START(finalize)
{
  SDPA_REGISTER_FUN(finalize);
}
SDPA_MOD_INIT_END(finalize)
