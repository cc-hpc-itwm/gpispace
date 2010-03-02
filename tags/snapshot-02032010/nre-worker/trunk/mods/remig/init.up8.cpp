#include <sdpa/modules/Macros.hpp>
#include <sdpa/modules/assert.hpp>

#include <fhglog/fhglog.hpp>

#include <fvm-pc/pc.hpp>

using namespace sdpa::modules;

void init (data_t &params) throw (std::exception)
{
  const std::string config_file (params.at("config_file").token().data());

  // read config file, parse, setup everything

  MLOG(DEBUG, "config_file = " << config_file);

  const unsigned long number_of_frequencies (20);
  const unsigned long number_of_depthlevels (40);
  const unsigned long number_of_parallel_propagators (8);
  const fvmAllocHandle_t memhandle_for_outputvolume (fvmGlobalAlloc(1<<20));
  ASSERT_ALLOC(memhandle_for_outputvolume, "global alloc");

  const fvmAllocHandle_t memhandle_for_configuration (fvmGlobalAlloc(1<<10));
  ASSERT_ALLOC(memhandle_for_configuration, "global alloc");

  // for now, just free the mem immediately
  MLOG (DEBUG, "number_of_frequencies = " << number_of_frequencies);
  MLOG (DEBUG, "number_of_depthlevels = " << number_of_depthlevels);
  MLOG (DEBUG, "number_of_parallel_propagators = " << number_of_parallel_propagators);
  MLOG (DEBUG, "memhandle_for_outputvolume = " << memhandle_for_outputvolume);
  MLOG (DEBUG, "memhandle_for_configuration = " << memhandle_for_configuration);

  params["number_of_frequencies"].token().data(number_of_frequencies);
  params["number_of_depthlevels"].token().data(number_of_depthlevels);
  params["number_of_parallel_propagators"].token().data(number_of_parallel_propagators);
  params["memhandle_for_temp_outputvolume_update_A"].token().data(memhandle_for_outputvolume);
  params["memhandle_for_temp_outputvolume_update_B"].token().data(memhandle_for_outputvolume);
  params["memhandle_for_temp_outputvolume_update_C"].token().data(memhandle_for_outputvolume);
  params["memhandle_for_temp_outputvolume_update_D"].token().data(memhandle_for_outputvolume);
  params["memhandle_for_temp_outputvolume_update_E"].token().data(memhandle_for_outputvolume);
  params["memhandle_for_temp_outputvolume_update_F"].token().data(memhandle_for_outputvolume);
  params["memhandle_for_temp_outputvolume_update_G"].token().data(memhandle_for_outputvolume);
  params["memhandle_for_temp_outputvolume_update_H"].token().data(memhandle_for_outputvolume);
  params["memhandle_for_temp_outputvolume_calc"].token().data(memhandle_for_outputvolume);
  params["memhandle_for_configuration"].token().data(memhandle_for_configuration);

  params["seq"].token().data("SEQ");
}

SDPA_MOD_INIT_START(init)
{
  SDPA_REGISTER_FUN_START(init);
    SDPA_ADD_INP( "config_file", char * );

    SDPA_ADD_OUT( "number_of_frequencies", unsigned long );
    SDPA_ADD_OUT( "number_of_depthlevels", unsigned long );
    SDPA_ADD_OUT( "number_of_parallel_propagators", unsigned long );
    SDPA_ADD_OUT( "memhandle_for_temp_outputvolume_update_A", fvmAllocHandle_t );
    SDPA_ADD_OUT( "memhandle_for_temp_outputvolume_update_B", fvmAllocHandle_t );
    SDPA_ADD_OUT( "memhandle_for_temp_outputvolume_update_C", fvmAllocHandle_t );
    SDPA_ADD_OUT( "memhandle_for_temp_outputvolume_update_D", fvmAllocHandle_t );
    SDPA_ADD_OUT( "memhandle_for_temp_outputvolume_update_E", fvmAllocHandle_t );
    SDPA_ADD_OUT( "memhandle_for_temp_outputvolume_update_F", fvmAllocHandle_t );
    SDPA_ADD_OUT( "memhandle_for_temp_outputvolume_update_G", fvmAllocHandle_t );
    SDPA_ADD_OUT( "memhandle_for_temp_outputvolume_update_H", fvmAllocHandle_t );
    SDPA_ADD_OUT( "memhandle_for_temp_outputvolume_calc", fvmAllocHandle_t );
    SDPA_ADD_OUT( "memhandle_for_configuration", fvmAllocHandle_t );

    SDPA_ADD_OUT("seq", char *);
  SDPA_REGISTER_FUN_END(init);
}
SDPA_MOD_INIT_END(init)
