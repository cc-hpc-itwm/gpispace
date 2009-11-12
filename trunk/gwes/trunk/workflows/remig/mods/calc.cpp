#include <sdpa/modules/Module.hpp>

using namespace sdpa::modules;

void calc (Module::data_t &params)
{
  const unsigned long slice_and_depth = params["slice_and_depth"].token().data_as<unsigned long>();
  const unsigned long number_of_frequencies = params["number_of_frequencies"].token().data_as<unsigned long>();
  const fvmHandle_t memhandle_for_configuration = params["memhandle_for_configuration"].token().data_as<fvmHandle_t>();

  const unsigned long slice = slice_and_depth / number_of_frequencies;
  const unsigned long depth = slice_and_depth % number_of_frequencies;
}

SDPA_MOD_INIT_START(calc)
{
  SDPA_REGISTER_FUN(calc);
}
SDPA_MOD_INIT_END(calc)
