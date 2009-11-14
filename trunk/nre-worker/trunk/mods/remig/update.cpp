#include <sdpa/modules/Module.hpp>

#include <fhglog/fhglog.hpp>

#include <fvm-pc/pc.hpp>

using namespace sdpa::modules;

void update (Module::data_t &params)
{
  const unsigned long slice_and_depth
    (params.at("slice_and_depth").token().data_as<unsigned long>());

  const unsigned long number_of_frequencies 
    (params.at("number_of_frequencies").token().data_as<unsigned long>());

  const fvmAllocHandle_t memhandle_for_configuration 
    (params.at("memhandle_for_configuration").token().data_as<fvmAllocHandle_t>());

  const fvmAllocHandle_t memhandle_for_outputvolume 
    (params.at("memhandle_for_outputvolume").token().data_as<fvmAllocHandle_t>());

  MLOG (DEBUG, "memhandle_for_configuration = " << memhandle_for_configuration);
  MLOG (DEBUG, "memhandle_for_outputvolume = " << memhandle_for_outputvolume);
  MLOG (DEBUG, "slice_and_depth = " << slice_and_depth);
  MLOG (DEBUG, "number_of_frequencies = " << number_of_frequencies);

  const unsigned long slice = slice_and_depth / number_of_frequencies;
  const unsigned long depth = slice_and_depth % number_of_frequencies;

  MLOG (DEBUG, "slice = " << slice);
  MLOG (DEBUG, "depth = " << depth);

  sleep(2);

  params["slice_and_depth_OUT"].token().data(slice_and_depth);
  params["memhandle_for_outputvolume_OUT"].token().data(memhandle_for_outputvolume);
}

SDPA_MOD_INIT_START(update)
{
  SDPA_REGISTER_FUN(update);
}
SDPA_MOD_INIT_END(update)
