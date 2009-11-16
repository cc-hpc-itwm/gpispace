#include <sdpa/modules/Macros.hpp>
#include <sdpa/modules/assert.hpp>

#include <fhglog/fhglog.hpp>

#include <fvm-pc/pc.hpp>


using namespace sdpa::modules;

void calc (data_t &params) throw (std::exception)
{
  const unsigned long slice_and_depth
    (params.at("slice_and_depth").token().data_as<unsigned long>());

  const unsigned long number_of_frequencies 
    (params.at("number_of_frequencies").token().data_as<unsigned long>());

  const fvmAllocHandle_t memhandle_for_configuration 
    (params.at("memhandle_for_configuration").token().data_as<fvmAllocHandle_t>());
  // maybe enclose this in #ifdefs?
  ASSERT_HANDLE(memhandle_for_configuration);

  MLOG (DEBUG, "memhandle_for_configuration = " << memhandle_for_configuration);
  MLOG (DEBUG, "slice_and_depth = " << slice_and_depth);
  MLOG (DEBUG, "number_of_frequencies = " << number_of_frequencies);

  const unsigned long slice = slice_and_depth / number_of_frequencies;
  const unsigned long depth = slice_and_depth % number_of_frequencies;

  MLOG (DEBUG, "slice = " << slice);
  MLOG (DEBUG, "depth = " << depth);

  params["slice_and_depth_OUT"].token().data(slice_and_depth);
}

SDPA_MOD_INIT_START(calc)
{
  SDPA_REGISTER_FUN_START(calc);
    SDPA_ADD_INP( "slice_and_depth", unsigned long );
    SDPA_ADD_INP( "number_of_frequencies", unsigned long );
    SDPA_ADD_INP( "memhandle_for_configuration", fvmAllocHandle_t );
    SDPA_ADD_OUT( "slice_and_depth_OUT", unsigned long );
  SDPA_REGISTER_FUN_END(calc);
}
SDPA_MOD_INIT_END(calc)
