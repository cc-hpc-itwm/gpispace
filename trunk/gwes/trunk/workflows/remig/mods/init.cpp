#include <sdpa/modules/Module.hpp>

using namespace sdpa::modules;

void init (Module::data_t &params)
{
  //  const std::string config_file = params["config_file"].token().data_as<std::string>();
  // read config file, parse, setup everything

  params["number_of_frequencies"].token().data(10UL);
  params["number_of_depthlevels"].token().data(5UL);
  params["number_of_parallel_propagators"].token().data(3UL);
  params["memhandle_for_outputvolume"].token().data(0xdeadbeefUL);
  params["memhandle_for_configuration"].token().data(0x47110815UL);
}

SDPA_MOD_INIT_START(init)
{
  SDPA_REGISTER_FUN(init);
}
SDPA_MOD_INIT_END(init)
