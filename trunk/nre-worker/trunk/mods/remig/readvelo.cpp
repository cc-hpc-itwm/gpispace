#include <sdpa/modules/Module.hpp>

using namespace sdpa::modules;

void readvelo (Module::data_t &params)
{
  // at the moment: do nothing
}

SDPA_MOD_INIT_START(readvelo)
{
  SDPA_REGISTER_FUN(readvelo);
}
SDPA_MOD_INIT_END(readvelo)
