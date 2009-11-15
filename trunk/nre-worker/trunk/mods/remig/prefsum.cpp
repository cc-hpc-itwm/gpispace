#include <sdpa/modules/Macros.hpp>

#include <fhglog/fhglog.hpp>

#include <fvm-pc/pc.hpp>

using namespace sdpa::modules;

void prefsum (data_t &params)
{
  const fvmAllocHandle_t memhandle_for_configuration
    (params["memhandle_for_configuration"].token().data_as<fvmAllocHandle_t>());

  MLOG (DEBUG, "memhandle_for_configuration = " << memhandle_for_configuration);

  // at the moment: do nothing

  params["seq"].token().data("SEQ");
}

SDPA_MOD_INIT_START(prefsum)
{
  SDPA_REGISTER_FUN_START(prefsum);
    SDPA_ADD_INP( "memhandle_for_configuration", fvmAllocHandle_t );

    SDPA_ADD_OUT("seq", char *);
  SDPA_REGISTER_FUN_END(prefsum);
}
SDPA_MOD_INIT_END(prefsum)
