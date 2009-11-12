#include <sdpa/modules/Module.hpp>

#include <fhglog/fhglog.hpp>

#include <fvm-pc/pc.hpp>

using namespace sdpa::modules;

void writeoutp (Module::data_t &params)
{
  const fvmAllocHandle_t memhandle_for_configuration
    (params["memhandle_for_configuration"].token().data_as<fvmAllocHandle_t>());

  MLOG (DEBUG, "memhandle_for_configuration = " << memhandle_for_configuration);

  // at the moment: do nothing
}

SDPA_MOD_INIT_START(writeoutp)
{
  SDPA_REGISTER_FUN(writeoutp);
}
SDPA_MOD_INIT_END(writeoutp)
