#include <sdpa/modules/Macros.hpp>
#include <sdpa/modules/assert.hpp>
#include <sdpa/modules/util.hpp>

#include <fhglog/fhglog.hpp>

#include <fvm-pc/pc.hpp>

#include <remig/reGlbStructs.h>

using namespace sdpa::modules;

void finalize (data_t &params) throw (std::exception)
{
  const fvmAllocHandle_t memhandle_for_configuration
    (params.at("memhandle_for_configuration").token().data_as<fvmAllocHandle_t>());
  ASSERT_HANDLE(memhandle_for_configuration);

  const fvmAllocHandle_t memhandle_for_outputvolume
    (params.at("memhandle_for_temp_outputvolume_update").token().data_as<fvmAllocHandle_t>());
  ASSERT_HANDLE(memhandle_for_outputvolume);

  DMLOG (DEBUG, "memhandle_for_configuration = " << memhandle_for_configuration);
  DMLOG (DEBUG, "memhandle_for_outputvolume  = " << memhandle_for_outputvolume);

  cfg_t node_config;
  fvm::util::get_data(&node_config, memhandle_for_configuration);

  int ret (0);
  DMLOG(DEBUG, "deallocating remig-scratch");
  ret += fvmGlobalFree (node_config.hndScratch);
  DMLOG(DEBUG, "deallocating remig-global-memory");
  ret += fvmGlobalFree (node_config.hndGlbVMspace);
  DMLOG(DEBUG, "deallocating config handle");
  ret += fvmGlobalFree (memhandle_for_configuration);
  DMLOG(DEBUG, "deallocating temp output volume");
  ret += fvmGlobalFree (memhandle_for_outputvolume);

  ASSERT_SUCCESS(ret, "final cleanup");

  params["seq"].token().data("SEQ");
}

SDPA_MOD_INIT_START(finalize)
{
  SDPA_REGISTER_FUN_START(finalize);
    SDPA_ADD_INP( "memhandle_for_configuration", fvmAllocHandle_t );
    SDPA_ADD_INP( "memhandle_for_temp_outputvolume", fvmAllocHandle_t );
    SDPA_ADD_OUT( "seq", char * );
  SDPA_REGISTER_FUN_END(finalize);
}
SDPA_MOD_INIT_END(finalize)
