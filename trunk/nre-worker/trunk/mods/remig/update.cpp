#include <sdpa/modules/Module.hpp>

using namespace sdpa::modules;

void update (Module::data_t &params)
{
  const unsigned long slice_and_depth = params["slice_and_depth"].token().data_as<unsigned long>();
  const unsigned long number_of_frequencies = params["number_of_frequencies"].token().data_as<unsigned long>();
  const fvmHandle_t memhandle_for_outputvolume = params["memhandle_for_outputvolume"].token().data_as<fvmHandle_t>();

  const unsigned long slice = slice_and_depth / number_of_frequencies;
  const unsigned long depth = slice_and_depth % number_of_frequencies;
}

SDPA_MOD_INIT_START(update)
{
  SDPA_REGISTER_FUN(update);
}
SDPA_MOD_INIT_END(update)
