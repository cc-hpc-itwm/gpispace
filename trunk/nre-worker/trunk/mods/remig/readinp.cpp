#include <sdpa/modules/Module.hpp>

#include <fhglog/fhglog.hpp>

#include <fvm-pc/pc.hpp>

using namespace sdpa::modules;

void readinp (Module::data_t &params)
{
  const fvmAllocHandle_t memhandle_for_configuration
    (params["memhandle_for_configuration"].token().data_as<fvmAllocHandle_t>());

  MLOG (DEBUG, "memhandle_for_configuration = " << memhandle_for_configuration);

  // at the moment: do nothing

  params["seq"].token().data("SEQ");
}

SDPA_MOD_INIT_START(readinp)
{
  SDPA_REGISTER_FUN(readinp);
}
SDPA_MOD_INIT_END(readinp)
