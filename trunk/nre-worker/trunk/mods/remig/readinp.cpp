#include <sdpa/modules/Module.hpp>

using namespace sdpa::modules;

void readinp (Module::data_t &params)
{
  // at the moment: do nothing
}

SDPA_MOD_INIT_START(readinp)
{
  SDPA_REGISTER_FUN(readinp);
}
SDPA_MOD_INIT_END(readinp)
