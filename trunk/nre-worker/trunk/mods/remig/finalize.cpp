#include <sdpa/modules/Macros.hpp>
#include <sdpa/modules/assert.hpp>

#include <fhglog/fhglog.hpp>

#include <fvm-pc/pc.hpp>

using namespace sdpa::modules;

void finalize (data_t &params) throw (std::exception)
{
  const fvmAllocHandle_t memhandle_for_configuration
    (params.at("memhandle_for_configuration").token().data_as<fvmAllocHandle_t>());
  ASSERT_HANDLE(memhandle_for_configuration);

  const fvmAllocHandle_t memhandle_for_outputvolume
    (params.at("memhandle_for_outputvolume").token().data_as<fvmAllocHandle_t>());
  ASSERT_HANDLE(memhandle_for_outputvolume);

  MLOG (DEBUG, "memhandle_for_configuration = " << memhandle_for_configuration);
  MLOG (DEBUG, "memhandle_for_outputvolume  = " << memhandle_for_outputvolume);

  int ret (0);
  ret += fvmGlobalFree (memhandle_for_configuration);
  ret += fvmGlobalFree (memhandle_for_outputvolume);
  ASSERT_SUCCESS(ret, "global free: configuration");

  params["seq"].token().data("SEQ");
}

SDPA_MOD_INIT_START(finalize)
{
  SDPA_REGISTER_FUN_START(finalize);
    SDPA_ADD_INP( "memhandle_for_configuration", fvmAllocHandle_t );
    SDPA_ADD_INP( "memhandle_for_outputvolume", fvmAllocHandle_t );
    SDPA_ADD_OUT( "seq", char * );
  SDPA_REGISTER_FUN_END(finalize);
}
SDPA_MOD_INIT_END(finalize)
